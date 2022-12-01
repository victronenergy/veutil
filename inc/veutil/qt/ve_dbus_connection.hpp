#pragma once

#include <QString>
#include <QDBusConnection>

class VeDbusConnection {
public:
	static QDBusConnection& getConnection();
	static QDBusConnection getConnection(const QString &name);

	static void setConnectionType(QDBusConnection::BusType type);
	static void setDBusAddress(const QString &address);
	static const QString getDBusAddress();
	static QDBusConnection::BusType defaultBusType();

private:
	static QDBusConnection::BusType mBusType;
	static QString mDBusAddress;
};
