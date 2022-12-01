#include <veutil/qt/ve_dbus_connection.hpp>

QDBusConnection::BusType VeDbusConnection::mBusType = QDBusConnection::SessionBus;
QString VeDbusConnection::mDBusAddress = "";

void VeDbusConnection::setConnectionType(QDBusConnection::BusType type)
{
	mBusType = type;
	mDBusAddress = "";
}

/**
 * Set the address of the dbus.
 * @note overwrites setConnectionType
 * @example
 *   tcp:host=<ip-address>,port=<tcp-port>
 *   unix:abstract=/tmp/dbus-UFmYHA8NiO
 *   session
 *   system
 */
void VeDbusConnection::setDBusAddress(const QString &address)
{
	if (address == "session") {
		setConnectionType(QDBusConnection::SessionBus);
	} else if (address == "system") {
		setConnectionType(QDBusConnection::SystemBus);
	} else {
		mDBusAddress = address;
	}
}

const QString VeDbusConnection::getDBusAddress()
{
	if (!mDBusAddress.isEmpty())
		return mDBusAddress;

	switch (mBusType)
	{
	case QDBusConnection::SystemBus:
		return QString::fromLatin1("system");
	case QDBusConnection::SessionBus:
		return QString::fromLatin1("session");
	default:
		return "";
	}
}

QDBusConnection::BusType VeDbusConnection::defaultBusType()
{
	return qgetenv("DBUS_SESSION_BUS_ADDRESS").isNull() ?
				QDBusConnection::SystemBus : QDBusConnection::SessionBus;
}

QDBusConnection &VeDbusConnection::getConnection()
{
	static bool isValid;
	static QDBusConnection mDBusConnection("not connected");

	if (isValid)
		return mDBusConnection;

	mDBusConnection = getConnection("qt-refuses-to-connect-without-a-name");
	isValid = true;

	return mDBusConnection;
}

QDBusConnection VeDbusConnection::getConnection(const QString &name)
{
	if (mDBusAddress.isEmpty()) {
		mBusType = defaultBusType();
		return QDBusConnection::connectToBus(mBusType, name);
	}

	return QDBusConnection::connectToBus(mDBusAddress, name);
}
