#include <QDBusMessage>
#include <QDBusVariant>
#include <QtDebug>
#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitem_exported_dbus_services.hpp>
#include "ve_qitem_exported_dbus_service.hpp"

VeQItemExportedDbusServices::VeQItemExportedDbusServices(VeQItem *root, QObject *parent):
	QObject(parent),
	mRoot(root)
{
	qDBusRegisterMetaType<ItemMap>();

	connect(mRoot, SIGNAL(childAdded(VeQItem *)), this, SLOT(onChildAdded(VeQItem *)));
	connect(mRoot, SIGNAL(childAboutToBeRemoved(VeQItem *)), this, SLOT(onChildRemoved(VeQItem *)));
}

bool VeQItemExportedDbusServices::open(const QString &address)
{
	mDbusAddress = address;
	addServices();
	return true;
}

void VeQItemExportedDbusServices::onChildAdded(VeQItem *item)
{
	connect(item, SIGNAL(stateChanged(VeQItem::State)), this, SLOT(onRootStateChanged()));

	if (item->getState() != VeQItem::Offline && item->getState() != VeQItem::Idle)
		addChild(item);
}

void VeQItemExportedDbusServices::onChildRemoved(VeQItem *item)
{
	disconnect(item);
	removeChild(item);
}

void VeQItemExportedDbusServices::onRootStateChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());

	if (item->getState() == VeQItem::Offline || item->getState() == VeQItem::Idle)
		removeChild(item);
	else
		addChild(item);
}

void VeQItemExportedDbusServices::addServices()
{
	for (int i = 0; ; ++i) {
		VeQItem *item = mRoot->itemChild(i);
		if (item == 0)
			break;
		onChildAdded(item);
	}
}

void VeQItemExportedDbusServices::addChild(VeQItem *item)
{
	foreach (VeQItemExportedDbusService *service, mServices) {
		if (service->root() == item)
			return;
	}

	QDBusConnection connection = getConnection(mDbusAddress, item->id());
	if (!connection.isConnected())
		qDebug() << "[VeQItemExportedDbusServices] Could not connect to D-Bus. Address:" << mDbusAddress
				 << "bus name:" << item->id();

	VeQItemExportedDbusService *service = new VeQItemExportedDbusService(connection, item, this);
	mServices.append(service);

	if (service->registerService())
		qDebug() << "[VeQItemExportedDbusServices] Registered service" << item->id();
	else
		qDebug() << "[VeQItemExportedDbusServices] Could not register service" << item->id();
}

void VeQItemExportedDbusServices::removeChild(VeQItem *item)
{
	foreach (VeQItemExportedDbusService *service, mServices) {
		if (service->root() == item) {
			mServices.removeOne(service);
			if (service->unregisterService())
				qDebug() << "[VeQItemExportedDbusServices] Unregistered service" << item->id();
			else
				qDebug() << "[VeQItemExportedDbusServices] Could not unregister service" << item->id();
			delete service;
			break;
		}
	}
}

QDBusConnection VeQItemExportedDbusServices::getConnection(const QString &address,
													const QString &qtDbusName)
{
	// FIXME: find non blocking version, all signals / slots are stalled for
	// 30 secs or so if this fails when connection by tcp/ip...
	if (address == "session")
		return QDBusConnection::connectToBus(QDBusConnection::SessionBus, qtDbusName);

	if (address == "system")
		return QDBusConnection::connectToBus(QDBusConnection::SystemBus, qtDbusName);

	return QDBusConnection::connectToBus(address, qtDbusName);
}
