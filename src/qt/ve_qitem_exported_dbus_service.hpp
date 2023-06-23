#pragma once

#include <QDBusConnection>
#include <QDBusVirtualObject>
#include <QList>
#include <QVariantMap>

#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp> // contains StringMap typedef

/// Represents a single D-Bus service and handles all method calls.
/// This is used by VeQItemExportedDbusTree to publish a VeQItem tree on the D-Bus.
class VeQItemExportedDbusService : public QDBusVirtualObject
{
	Q_OBJECT

public:
	VeQItemExportedDbusService(const QDBusConnection &connection, VeQItem *root, QObject *parent = 0);

	QString serviceName() const;
	VeQItem *root();
	bool registerService();
	bool unregisterService();
	QString introspect(const QString &path) const override;
	bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;

private slots:
	void onChildAdded(VeQItem *child);
	void onChildAboutToBeRemoved(VeQItem *child);
	void onValueChanged();
	void onTextChanged();
	void onDynamicPropertyChanged(char const *name);

private:
	bool handleGetValue(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *item);
	bool handleGetText(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *item);
	bool handleSetValue(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *producer);
	bool handleGetMin(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *item);
	bool handleGetMax(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *item);
	bool handleGetDefault(const QDBusMessage &message, const QDBusConnection &connection,
						VeQItem *item);
	bool handleGetItems(const QDBusMessage &message, const QDBusConnection &connection);

	void addPending(VeQItem *item, VeQItem::Properties property);
	Q_INVOKABLE void processPending();

	void connectItem(VeQItem *item);
	void disconnectItem(VeQItem *item);

	/// This will create a map for GetValue on a node if map is of type QVariantMap.
	/// If it is a StringMap the map for GetText will be created.
	template<typename Map>
	void buildTree(VeQItem *root, VeQItem *item, Map &map)
	{
		for (VeQItem *child: item->itemChildren()) {
			if (child->isLeaf()) {
				QString path = child->getRelId(root);
				if (path.startsWith('/'))
					path.remove(0, 1);
				addTreeValue(path, child, map);
			} else {
				buildTree(root, child, map);
			}
		}
	}

	void addTreeValue(const QString &path, VeQItem *item, QVariantMap &map)
	{
		map[path] = denormalizeVariant(item->getValue());
	}

	void addTreeValue(const QString &path, VeQItem *item, StringMap &map)
	{
		map[path] = item->getText();
	}

	static void normalizeVariant(QVariant &v);
	static void normalizeMap(QVariantMap &map);
	static void normalizeList(QVariantList &list);
	static QVariant &denormalizeVariant(QVariant &v);
	static QVariant denormalizeVariant(const QVariant &v);

	QDBusConnection mConnection;
	VeQItem *mRoot;
	QList<QPair<VeQItem *, VeQItem::Properties>> mPendingChanges;
};
