#pragma once

#include <QObject>

#include <veutil/qt/canbus_interfaces.hpp>
#include <veutil/qt/q_udev.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp>

class CanInterfaceMonitor : public QObject
{
	Q_OBJECT

public:
	CanInterfaceMonitor(VeQItemSettings *settings, VeQItem *service, QObject *parent = nullptr);
	void enumerate();
	QVariantMapList canInfo();

signals:
	void interfacesChanged();

private slots:
	void handleUdevCanBusEvent(QUdevAction action, struct udev_device *dev);

private:
	QUdevMonitor mCanMonitor;

	void addCanInterface(unsigned int ifindex, udev_device *dev);
	void removeCanInterface(unsigned ifindex);

	VeQItemSettings *mSettings;
	VeQItem *mService;
	QMap<unsigned, CanBusProfiles*> mCanInterfaces;
};
