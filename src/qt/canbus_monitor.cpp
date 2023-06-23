#include <veutil/qt/canbus_monitor.hpp>

static unsigned int udev_device_get_ifindex(struct udev_device *d)
{
	const char *val;

	val = udev_device_get_property_value(d, "IFINDEX");
	if (!val)
		return 0;

	return strtoul(val, NULL, 10);
}

CanInterfaceMonitor::CanInterfaceMonitor(VeQItemSettings *settings, VeQItem *service, QObject *parent) :
	QObject(parent),
	mCanMonitor(QUdev::instance()),
	mSettings(settings),
	mService(service)
{
	// React to CAN-bus interface changes.
	auto monitor = mCanMonitor.monitor();
	udev_monitor_filter_add_match_subsystem_devtype(monitor, "net", nullptr);
	connect(&mCanMonitor, SIGNAL(udevEvent(QUdevAction, struct udev_device *)),
			SLOT(handleUdevCanBusEvent(QUdevAction, struct udev_device *)));
	mCanMonitor.enableReceiving();
}

void CanInterfaceMonitor::enumerate()
{
	if (!mCanInterfaces.empty()) {
		qCritical("only enumerate once");
		return;
	}

	// Get a list off available CAN-bus interfaces.
	struct udev_enumerate  *enumerate = udev_enumerate_new(QUdev::instance()->udev());
	udev_enumerate_add_match_subsystem(enumerate, "net");
	udev_enumerate_add_match_is_initialized(enumerate);
	udev_enumerate_add_match_sysattr(enumerate, "type", "280");
	QUdevList list = QUdev::instance()->scan(enumerate);

	for (const auto &dev: list) {
		unsigned int ifindex = udev_device_get_ifindex(dev);
		addCanInterface(ifindex, dev);
	}

	udev_enumerate_unref(enumerate);

	if (!list.empty())
		emit interfacesChanged();
}

QVariantMapList CanInterfaceMonitor::canInfo()
{
	QVariantMapList ret;
	QMap<QString, CanBusProfiles*> sorted;

	for (auto interface: mCanInterfaces)
		sorted.insert(interface->getUiName().toLower(), interface);

	for (auto interface: sorted)
		ret.append(interface->canInfo());

	return ret;
}

void CanInterfaceMonitor::addCanInterface(unsigned int ifindex, udev_device *dev)
{
	char const *sysname = udev_device_get_sysname(dev);

	if (ifindex == 0)
		return;

	if (!sysname || !isCanBusInterface(sysname))
		return;

	CanBusProfiles::CanBusConfig config = CanBusProfiles::CanAnyBus;
	CanBusProfile::CanProfile defaultProfile = CanBusProfile::CanProfileVecan;

	char const *configStr = udev_device_get_property_value(dev, "VE_CAN_CONFIG");
	if (configStr && strcmp(configStr, "bms-only") == 0) {
		config = CanBusProfiles::CanForcedCanBusBms;
		defaultProfile = CanBusProfile::CanProfileCanBms500;
	} else if (configStr && strcmp(configStr, "vecan-only") == 0) {
		config = CanBusProfiles::CanForcedVeCan;
	}

	auto canInterface = new CanBusProfiles(mSettings, mService, QString::fromUtf8(sysname),
										   defaultProfile, config, this);
	char const *name = udev_device_get_property_value(dev, "VE_NAME");
	if (name)
		canInterface->setUiName(name);
	mCanInterfaces.insert(ifindex, canInterface);
}

void CanInterfaceMonitor::removeCanInterface(unsigned ifindex)
{
	if (ifindex == 0)
		return;

	auto canInterface = mCanInterfaces.take(ifindex);
	if (!canInterface)
		return;

	canInterface->activeProfile()->disableProfile();

	delete canInterface;
}

void CanInterfaceMonitor::handleUdevCanBusEvent(QUdevAction action, udev_device *dev)
{
	unsigned int ifindex = udev_device_get_ifindex(dev);

	if (action == QUdevAction::Add) {
		addCanInterface(ifindex, dev);
	} else if (action == QUdevAction::Move) {
		removeCanInterface(ifindex);
		addCanInterface(ifindex, dev);
	} else if (action == QUdevAction::Remove) {
		removeCanInterface(ifindex);
	} else {
		return;
	}

	emit interfacesChanged();
}

