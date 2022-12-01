#pragma once

#include <QDBusConnection>

class VeQItem;
class VeQItemExportedDbusService;

/// Publish an existing VeQItem tree on the D-Bus.
/// Some notes:
/// * The child item of the root item will be represented by a D-Bus service. All items below that
///   level will be D-Bus objects. The paths to the objects will be the relative path from the
///   service item to the object item.
///	* Items with status Offline and Idle will not be published. In case of a service level item,
///	  no D-Bus service will be created. This means you have to set the state of service level items
///   explicitly to get them published.

class VeQItemExportedDbusServices : public QObject
{
	Q_OBJECT
public:
	VeQItemExportedDbusServices(VeQItem *root, QObject *parent = 0);
	bool open(const QString &address = "session");

private slots:
	void onChildAdded(VeQItem *item);
	void onChildRemoved(VeQItem *item);
	void onRootStateChanged();

private:
	void addServices();
	void addChild(VeQItem *item);
	void removeChild(VeQItem *item);

	static QDBusConnection getConnection(const QString &address, const QString &qtDbusName);

	QString mDbusAddress;
	QList<VeQItemExportedDbusService *> mServices;
	VeQItem *mRoot;
};
