#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDebug>
#include <veutil/qt/ve_qitem.hpp>
#include "ve_qitem_exported_dbus_service.hpp"

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(StringMap)

VeQItemExportedDbusService::VeQItemExportedDbusService(const QDBusConnection &connection, VeQItem *root,
												   QObject *parent):
	QDBusVirtualObject(parent),
	mConnection(connection),
	mRoot(root)
{
	Q_ASSERT(root != 0);
	connectItem(root);
}

VeQItem *VeQItemExportedDbusService::root()
{
	return mRoot;
}

QString VeQItemExportedDbusService::serviceName() const
{
	return mConnection.name();
}

bool VeQItemExportedDbusService::registerService()
{
	if (!mConnection.registerVirtualObject("/", this, QDBusConnection::SubPath)) {
		qDebug() << "[VeQItemExportedDbusService] Could not register virtual object";
		return false;
	}

	if (!mConnection.registerService(serviceName())) {
		qDebug() << "[VeQItemExportedDbusService] Could not register service" << serviceName();
		return false;
	}

	return true;
}

bool VeQItemExportedDbusService::unregisterService()
{
	if (!mConnection.unregisterService(serviceName())) {
		qDebug() << "[VeQItemExportedDbusService] Could not unregister service" << serviceName();
		return false;
	}

	return true;
}

QString VeQItemExportedDbusService::introspect(const QString &path) const
{
	VeQItem *item = mRoot->itemGet(path);
	if (item == 0)
		return QString();

	QString result = QString(
				"  <interface name=\"com.victronenergy.BusItem\">\n"
				"    <method name=\"GetValue\">\n"
				"      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
				"    </method>\n"
				"    <method name=\"GetText\">\n"
				"      <arg direction=\"out\" type=\"s\" name=\"value\"/>\n"
				"    </method>\n"
				"%1"
				"%2"
				"  </interface>\n")
			.arg(!item->hasChildren()
				 ? "    <method name=\"SetValue\">\n"
				   "      <arg direction=\"in\" type=\"v\" name=\"value\"/>\n"
				   "      <arg direction=\"out\" type=\"i\" name=\"retval\"/>\n"
				   "    </method>\n"
				   "    <method name=\"GetMin\">\n"
				   "      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
				   "    </method>\n"
				   "    <method name=\"GetMax\">\n"
				   "      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
				   "    </method>\n"
				   "    <method name=\"GetDefault\">\n"
				   "      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
				   "    </method>\n"
				 : "",
			path == "/"
				 ? "    <method name=\"GetItems\">\n"
				   "      <arg direction=\"out\" type=\"a{sa{sv}}\" name=\"items\"/>\n"
				   "    </method>\n"
				   "    <signal name=\"ItemsChanged\">\n"
				   "      <arg direction=\"out\" type=\"a{sa{sv}}\" name=\"changes\"/>\n"
				   "    </signal>\n"
				 : "")
			;

	for (VeQItem *child: item->itemChildren())
		result.append(QString("  <node name=\"%1\"/>\n").arg(child->id()));

	return result;
}

bool VeQItemExportedDbusService::handleMessage(const QDBusMessage &message,
											 const QDBusConnection &connection)
{
	switch (message.type()) {
	case QDBusMessage::MethodCallMessage:
	{
		QString member = message.member();
		VeQItem *item = mRoot->itemGet(message.path());
		if (item == 0)
			return false;

		if (member == "GetValue")
			return handleGetValue(message, connection, item);
		if (member == "GetText")
			return handleGetText(message, connection, item);
		if (member == "GetItems" && message.path() == "/")
			return handleGetItems(message, connection);

		if (!item->isLeaf())
			return false;

		if (member == "SetValue")
			return handleSetValue(message, connection, item);
		else if (member == "GetMin")
			return handleGetMin(message, connection, item);
		else if (member == "GetMax")
			return handleGetMax(message, connection, item);
		else if (member == "GetDefault")
			return handleGetDefault(message, connection, item);
		break;
	}
	default:
		qDebug() << "[VeQItemExportedDbusService] unhandled message type:" << message.type();
		break;
	}
	return false;
}

bool VeQItemExportedDbusService::handleGetValue(const QDBusMessage &message,
											  const QDBusConnection &connection, VeQItem *item)
{
	if (!item->hasChildren()) {
		QDBusMessage reply = message.createReply(QVariant::fromValue(QDBusVariant(denormalizeVariant(item->getValue()))));
		return connection.send(reply);
	} else {
		QVariantMap map;
		buildTree(item, item, map);
		QDBusMessage reply = message.createReply(QVariant::fromValue(QDBusVariant(map)));
		return connection.send(reply);
	}
}

bool VeQItemExportedDbusService::handleGetText(const QDBusMessage &message,
											 const QDBusConnection &connection, VeQItem *item)
{
	if (!item->hasChildren()) {
		QDBusMessage reply = message.createReply(QVariant::fromValue(item->getText()));
		return connection.send(reply);
	} else {
		StringMap map;
		buildTree(item, item, map);
		QDBusMessage reply = message.createReply(
			QVariant::fromValue(QDBusVariant(QVariant::fromValue(map))));
		return connection.send(reply);
	}
}

bool VeQItemExportedDbusService::handleSetValue(const QDBusMessage &message,
											  const QDBusConnection &connection, VeQItem *producer)
{
	QList<QVariant> args = message.arguments();
	if (args.size() != 1) {
		QDBusMessage reply = message.createErrorReply(QDBusError::InvalidArgs,
													  "Expected 1 argument");
		return connection.send(reply);
	}
	QVariant v = args.first();
	normalizeVariant(v);
	int r = producer->setValue(v);
	QDBusMessage reply = message.createReply(r);
	return connection.send(reply);
}

bool VeQItemExportedDbusService::handleGetMin(const QDBusMessage &message,
											const QDBusConnection &connection, VeQItem *item)
{
	QDBusMessage reply = message.createReply(
				QVariant::fromValue(QDBusVariant(denormalizeVariant(item->itemProperty("min")))));
	return connection.send(reply);
}

bool VeQItemExportedDbusService::handleGetMax(const QDBusMessage &message,
											const QDBusConnection &connection, VeQItem *item)
{
	QDBusMessage reply = message.createReply(
				QVariant::fromValue(QDBusVariant(denormalizeVariant(item->itemProperty("max")))));
	return connection.send(reply);
}

bool VeQItemExportedDbusService::handleGetDefault(const QDBusMessage &message, const QDBusConnection &connection,
												VeQItem *item)
{
	QDBusMessage reply = message.createReply(
				QVariant::fromValue(QDBusVariant(denormalizeVariant(item->itemProperty("defaultValue")))));
	return connection.send(reply);
}

bool VeQItemExportedDbusService::handleGetItems(const QDBusMessage &message,
											  const QDBusConnection &connection)
{
	ItemMap items;

	mRoot->foreachParentFirst([&items,this](VeQItem *item){
		if (!item->hasChildren()) {
			QMap<QString, QVariant> m;
			m.insert("Value", denormalizeVariant(item->getValue()));
			m.insert("Text", item->getText());

			QVariant v;
			v = item->itemProperty("min");
			if (v.isValid())
				m.insert("Min", v); // No need to denormalize as v is valid
			v = item->itemProperty("max");
			if (v.isValid())
				m.insert("Max", v);
			v = item->itemProperty("defaultValue");
			if (v.isValid())
				m.insert("Default", v);
			items.insert(item->getRelId(mRoot), m);
		}
	});

	QDBusMessage reply = message.createReply(QVariant::fromValue(items));

	return connection.send(reply);
}

void VeQItemExportedDbusService::addPending(VeQItem *item, VeQItem::Properties property)
{
	for (QPair<VeQItem *, VeQItem::Properties> &i : mPendingChanges) {
		if (i.first == item) {
			i.second |= property;
			return;
		}
	}

	if (mPendingChanges.isEmpty())
		QMetaObject::invokeMethod(this, "processPending", Qt::QueuedConnection);

	mPendingChanges.append(qMakePair(item, property));
}

void VeQItemExportedDbusService::processPending()
{
	ItemMap items;

	for (QPair<VeQItem *, VeQItem::Properties> const &i : mPendingChanges) {
		VeQItem *item = i.first;
		VeQItem::Properties flags = i.second;
		QMap<QString, QVariant> m;

		if (flags.testFlag(VeQItem::Value))
			m.insert("Value", denormalizeVariant(item->getLocalValue()));
		if (flags.testFlag(VeQItem::Text))
			m.insert("Text", item->getText());
		if (flags.testFlag(VeQItem::Default))
			m.insert("Default", denormalizeVariant(item->itemProperty("defaultValue")));
		if (flags.testFlag(VeQItem::Min))
			m.insert("Min", denormalizeVariant(item->itemProperty("min")));
		if (flags.testFlag(VeQItem::Max))
			m.insert("Max", denormalizeVariant(item->itemProperty("max")));

		items.insert(item->getRelId(mRoot), m);
	}

	QDBusMessage message = QDBusMessage::createSignal("/", "com.victronenergy.BusItem", "ItemsChanged");
	message << QVariant::fromValue(items);
	if (!mConnection.send(message)) {
		qDebug() << "[VeQItemExportedDbusService] Could not send ItemsChanged signal for" << mRoot->id();
		return;
	}

	mPendingChanges.clear();
}

void VeQItemExportedDbusService::connectItem(VeQItem *item)
{
	connect(item, SIGNAL(valueChanged(QVariant)), this, SLOT(onValueChanged()));
	connect(item, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged()));
	connect(item, SIGNAL(dynamicPropertyChanged(const char*,QVariant)),
			this, SLOT(onDynamicPropertyChanged(const char*)));

	connect(item, SIGNAL(childAdded(VeQItem*)), this, SLOT(onChildAdded(VeQItem *)));
	connect(item, SIGNAL(childAboutToBeRemoved(VeQItem*)), this, SLOT(onChildAboutToBeRemoved(VeQItem *)));

	for (VeQItem *child: item->itemChildren())
		connectItem(child);
}

void VeQItemExportedDbusService::disconnectItem(VeQItem *item)
{
	item->disconnect(this);

	for (VeQItem *child: item->itemChildren())
		disconnectItem(child);
}

void VeQItemExportedDbusService::onChildAdded(VeQItem *child)
{
	connectItem(child);
}

void VeQItemExportedDbusService::onChildAboutToBeRemoved(VeQItem *child)
{
	// flush to make sure there are no dangling pointers in the queue
	processPending();
	disconnectItem(child);
}

void VeQItemExportedDbusService::onValueChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	addPending(item, VeQItem::Value);
}

void VeQItemExportedDbusService::onTextChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	addPending(item, VeQItem::Text);
}

void VeQItemExportedDbusService::onDynamicPropertyChanged(char const *name)
{
	VeQItem *item = static_cast<VeQItem *>(sender());

	if (strcmp(name, "min") == 0)
		addPending(item, VeQItem::Min);
	else if (strcmp(name, "max") == 0)
		addPending(item, VeQItem::Max);
	else if (strcmp(name, "defaultValue") == 0)
		addPending(item, VeQItem::Default);
}

void VeQItemExportedDbusService::normalizeVariant(QVariant &v)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	if (v.userType() == QVariant::Map) {
#else
	if (v.userType() == QMetaType::QVariantMap) {
#endif
		QVariantMap m = v.toMap();
		normalizeMap(m);
	} else if (v.userType() == qMetaTypeId<QDBusVariant>()) {
		v = qvariant_cast<QDBusVariant>(v).variant();
		normalizeVariant(v);
	} else if (v.userType() == qMetaTypeId<QDBusArgument>()) {
		QDBusArgument arg = qvariant_cast<QDBusArgument>(v);
		if (arg.currentSignature() == "a{sv}") {
			QVariantMap m = qdbus_cast<QVariantMap>(arg);
			normalizeMap(m);
			v = m;
		} else  if (arg.currentSignature() == "av") {
			QVariantList vl = qdbus_cast<QVariantList>(arg);
			normalizeList(vl);
			v = vl;
		} else  if (arg.currentSignature() == "ai") {
			// An empty list of integers is used by victron to encode an invalid
			// value, because D-Bus itself does not define an invalid (or null)
			// value.
			QList<int> vl = qdbus_cast<QList<int> >(arg);
			if (vl.size() == 0)
				v = QVariant();
		} else {
			qDebug() <<	"Cannot handle signature:" << arg.currentSignature();
		}
	}
}

void VeQItemExportedDbusService::normalizeMap(QVariantMap &map)
{
	for (QVariantMap::iterator it = map.begin(); it != map.end(); ++it)
		normalizeVariant(it.value());
}

void VeQItemExportedDbusService::normalizeList(QVariantList &list)
{
	for (QVariantList::iterator it = list.begin(); it != list.end(); ++it)
		normalizeVariant(*it);
}

QVariant & VeQItemExportedDbusService::denormalizeVariant(QVariant &v)
{
	if (!v.isValid())
		v = QVariant::fromValue(QList<int>());

	return v;
}

QVariant VeQItemExportedDbusService::denormalizeVariant(const QVariant &v)
{
	if (!v.isValid())
		return QVariant::fromValue(QList<int>());

	return v;
}
