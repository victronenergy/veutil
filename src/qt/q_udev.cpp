#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef ARPHRD_CAN
#define ARPHRD_CAN 280
#endif

#include <veutil/qt/q_udev.hpp>

bool isCanBusInterface(char const *interface)
{
	struct ifreq ifr = {};
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);

	qstrncpy(ifr.ifr_name, interface, IFNAMSIZ);
	bool ret = ioctl(fd, SIOCGIFHWADDR, &ifr) == 0 && ifr.ifr_hwaddr.sa_family == ARPHRD_CAN;
	close(fd);

	return ret;
}

void QUdev::dumpDevice(struct udev_device *dev)
{
	qDebug() << "syspath:   " << udev_device_get_syspath(dev);
	qDebug() << "sysname:   " << udev_device_get_sysname(dev);
	qDebug() << "sysnum:    " << udev_device_get_sysnum(dev);
	qDebug() << "devpath:   " << udev_device_get_devpath(dev);
	qDebug() << "devnode:   " << udev_device_get_devnode(dev);
	qDebug() << "devtype:   " << udev_device_get_devtype(dev);
	qDebug() << "subsystem: " << udev_device_get_subsystem(dev);
	qDebug() << "driver:    " << udev_device_get_driver(dev);
}

QUdevList QUdev::enumerate(char const *subsystem) const
{
	struct udev_enumerate  *enumerate = udev_enumerate_new(mUdev);
	udev_enumerate_add_match_subsystem(enumerate, subsystem);
	udev_enumerate_add_match_is_initialized(enumerate);
	QUdevList list = scan(enumerate);
	udev_enumerate_unref(enumerate);

	return list;
}

// Scans and wraps the result in a List which owns the devices. It uses the c
// struct udev_enumerate type as argument, so all c function can be used to
// determine what is enumerated, see enumerate as an example.
// Likewise the list contains pointers to the struct udev_device so the c
// functions can be used to get more information.
QUdevList QUdev::scan(udev_enumerate *enumerate) const
{
	QUdevList ret;

	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *list = udev_enumerate_get_list_entry(enumerate);

	struct udev_list_entry *entry;
	udev_list_entry_foreach(entry, list) {
		const char *name = udev_list_entry_get_name(entry);
		if (name) {
			struct udev_device *dev = udev_device_new_from_syspath(mUdev, name);
			ret.append(dev);
		}
	}

	return ret;
}

QUdevMonitor::QUdevMonitor(QUdev *qudev) :
	QObject(qudev)
{
	// NOTE: the udev argument is there to make sure events are received
	// after udev rules are processed.
	mMonitor = udev_monitor_new_from_netlink(qudev->udev(), "udev");
	if (!mMonitor)
		qCritical() << "Failed to create udev monitor";
}

QUdevMonitor::~QUdevMonitor()
{
	udev_monitor_unref(mMonitor);
}

bool QUdevMonitor::enableReceiving()
{
	if (mNotifier)
		return true;

	if (udev_monitor_enable_receiving(mMonitor) < 0) {
		qCritical() << "Enabling the udev monitor failed";
		return false;
	}

	int fd = udev_monitor_get_fd(mMonitor);
	if (fd < 0) {
		qCritical() << "Unable to get file descriptor of the udev monitor" << fd;
		return false;
	}

	mNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(mNotifier, SIGNAL(activated(int)), SLOT(handleUdevReceive()));

	return true;
}

void QUdevMonitor::handleUdevReceive()
{
	struct udev_device *dev;

	for (;;) {
		dev = udev_monitor_receive_device(mMonitor);
		if (!dev)
			break;

		QUdevAction action = getUdevDeviceAction(dev);
		emit udevEvent(action, dev);

		udev_device_unref(dev);
	}
}

QUdevAction QUdevMonitor::getUdevDeviceAction(struct udev_device *dev)
{
	const char *action = udev_device_get_action(dev);

	if (!action)
		return QUdevAction::None;

	if (strcmp(action, "add") == 0)
		return QUdevAction::Add;
	if (strcmp(action, "remove") == 0)
		return QUdevAction::Remove;
	if (strcmp(action, "move") == 0)
		return QUdevAction::Move;
	if (strcmp(action, "change") == 0)
		return QUdevAction::Change;
	if (strcmp(action, "online") == 0)
		return QUdevAction::Online;
	if (strcmp(action, "offline") == 0)
		return QUdevAction::Offline;

	return QUdevAction::Unknown;
}
