#include "veutil/qt/dbus_service_type.h"
#include <QStringList>

DBusServiceType::DBusServiceType(QObject *parent)
	: QObject{parent}
{
}

DBusServiceType::ServiceType DBusServiceType::getServiceType(QString serviceName)
{
	if (!serviceName.startsWith("com.victronenergy."))
		return DBUS_SERVICE_UNKNOWN;

	QStringList elements = serviceName.split(".");
	if (elements.count() < 3)
		return DBUS_SERVICE_UNKNOWN;

	QString type = elements[2];

	if (type == "battery")
		return DBUS_SERVICE_BATTERY;

	if (type == "vebus")
		return DBUS_SERVICE_MULTI;

	if (type == "multi")
		return DBUS_SERVICE_MULTI_RS;

	if (type == "solarcharger")
		return DBUS_SERVICE_SOLAR_CHARGER;

	if (type == "pvinverter")
		return DBUS_SERVICE_PV_INVERTER;

	if (type == "charger")
		return DBUS_SERVICE_AC_CHARGER;

	if (type == "tank")
		return DBUS_SERVICE_TANK;

	if (type == "grid")
		return DBUS_SERVICE_GRIDMETER;

	if (type == "genset")
		return DBUS_SERVICE_GENSET;

	if (type == "motordrive")
		return DBUS_SERVICE_MOTOR_DRIVE;

	if (type == "inverter")
		return DBUS_SERVICE_INVERTER;

	if (type == "system")
		return DBUS_SERVICE_SYSTEM_CALC;

	if (type == "temperature")
		return DBUS_SERVICE_TEMPERATURE_SENSOR;

	if (type == "generator")
		return DBUS_SERVICE_GENERATOR_STARTSTOP;

	if (type == "pulsemeter")
		return DBUS_SERVICE_PULSE_COUNTER;

	if (type == "digitalinput")
		return DBUS_SERVICE_DIGITAL_INPUT;

	if (type == "unsupported")
		return DBUS_SERVICE_UNSUPPORTED;

	if (type == "meteo")
		return DBUS_SERVICE_METEO;

	if (type == "vecan")
		return DBUS_SERVICE_VECAN;

	if (type == "evcharger")
		return DBUS_SERVICE_EVCHARGER;

	if (type == "acload")
		return DBUS_SERVICE_ACLOAD;

	if (type == "hub4")
		return DBUS_SERVICE_HUB4;

	if (type == "fuelcell")
		return DBUS_SERVICE_FUELCELL;

	if (type == "dcsource")
		return DBUS_SERVICE_DCSOURCE;

	if (type == "alternator")
		return DBUS_SERVICE_ALTERNATOR;

	if (type == "dcload")
		return DBUS_SERVICE_DCLOAD;

	if (type == "dcsystem")
		return DBUS_SERVICE_DCSYSTEM;

	if (type == "platform")
		return DBUS_SERVICE_PLATFORM;

	return DBUS_SERVICE_UNKNOWN;
}


