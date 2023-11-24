#include <qstringlist.h>
#include "veutil/qt/notificationdescriptions.h"

#include <veutil/qt/charger_error.hpp>
#include <veutil/qt/bms_error.hpp>
#include <veutil/qt/vebus_error.hpp>
#include <veutil/qt/wakespeed_error.hpp>
#include <veutil/qt/dbus_service_type.h>


NotificationDescriptions::NotificationDescriptions()
{
	addBatteryAlarms();
	addDcMeterAlarms();
	addAlternatorAlarms();
	addVebusAlarms();
	addMultiRsAlarms();
	addSolarChargerAlarms();
	addAcChargerAlarms();
	addDigitalInputAlarms();
	addSystemCalcAlarms();
	addGeneratorStartStopAlarms();
	addDigitalInputAlarms();
	addVecanAlarms();
	addEssAlarms();
	addTankAlarms();
	addPlatformAlarms();
}


QString NotificationDescriptions::getDescription(QString serviceName, QString trigger, QVariant value)
{
	DBusServiceType::ServiceType type = DBusServiceType::getServiceType(serviceName);
	QString description;

	switch (type) {
	case DBusServiceType::DBUS_SERVICE_BATTERY:
		description = getBatteryAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_FUELCELL:
	case DBusServiceType::DBUS_SERVICE_DCSOURCE:
	case DBusServiceType::DBUS_SERVICE_DCLOAD:
	case DBusServiceType::DBUS_SERVICE_DCSYSTEM:
		mDcMeterAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_ALTERNATOR:
		description = getAlternatorAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_MULTI:
		description = getVebusAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_MULTI_RS:
		description = getMultiRsAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_SOLAR_CHARGER:
		description = getSolarChagerAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_AC_CHARGER:
		description = getAcChargerAlarmDescription(trigger, value);
		break;
	case DBusServiceType::DBUS_SERVICE_INVERTER:
		description = mInverterAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_SYSTEM_CALC:
		description = mSystemCalcAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_GENERATOR_STARTSTOP:
		description = mGeneratorStartStopAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_DIGITAL_INPUT:
		description = mDigitalInputAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_VECAN:
		description = mVecanAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_HUB4:
		description = mEssAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_TANK:
		description = mTankAlarms[trigger];
		break;
	case DBusServiceType::DBUS_SERVICE_PLATFORM:
		description = mPlatformAlarms[trigger];
		break;
	default:
		break;
	}

	return tr(qPrintable(description));
}


QString NotificationDescriptions::getBatteryAlarmDescription(QString trigger, QVariant value)
{
	QString description;

	if (trigger == "/ErrorCode") {
		int error = value.toInt();
		description = getChargerErrorDescription(error);
	} else {
		description = getDescriptionFromTrigger(trigger, mBatteryAlarms);
	}

	return description;
}

QString NotificationDescriptions::getAcChargerAlarmDescription(QString trigger, QVariant value)
{
	QString description;

	if (trigger == "/ErrorCode") {
		int error = value.toInt();
		description = getChargerErrorDescription(error);
	} else {
		description = mAcChargerAlarms[trigger];
	}

	return description;
}

QString NotificationDescriptions::getSolarChagerAlarmDescription(QString trigger, QVariant value)
{
	QString description;

	if (trigger == "/ErrorCode") {
		int error = value.toInt();
		description = getChargerErrorDescription(error);
	} else {
		description = mSolarChargerAlarms[trigger];
	}

	return description;
}

QString NotificationDescriptions::getMultiRsAlarmDescription(QString trigger, QVariant value)
{
	QString description;

	if (trigger == "/ErrorCode") {
		int error = value.toInt();
		description = getChargerErrorDescription(error);
	} else {
		description = mMultiRsAlarms[trigger];
	}

	return description;
}

QString NotificationDescriptions::getAlternatorAlarmDescription(QString trigger, QVariant value)
{
	QString description;

	if (trigger == "/ErrorCode") {
		int error = value.toInt();
		description = getAlternatorErrorDescription(error);
	} else {
		description = mAlternatorAlarms[trigger];
	}

	return description;
}

QString NotificationDescriptions::getVebusAlarmDescription(QString trigger, QVariant value) {
	QString description;
	if (trigger == "/VebusError") {
		int error = value.toInt();
		description = getVebusErrorDescription(error);
	} else {
		description = mVebusAlarms[trigger];
	}

	return description;
}

QString NotificationDescriptions::getAlternatorErrorDescription(int error)
{
	// Wakespeed is the only alternator controller we support for now
	return WakespeedError::getDescription(error);
}

QString NotificationDescriptions::getVebusErrorDescription(int error)
{
	return VebusError::getDescription(error);
}

QString NotificationDescriptions::getChargerErrorDescription(int error)
{
	return ChargerError::getDescription(error);
}

QString NotificationDescriptions::getBmsErrorDescription(int error)
{
	return BmsError::getDescription(error);
}

QString NotificationDescriptions::getDescriptionFromTrigger(QString trigger, AlarmDescription descriptions)
{
	QString description;
	if (descriptions.contains(trigger)) {
		return descriptions[trigger];
	}

	foreach (const QString &key, descriptions.keys()) {
		QRegExp regExp(key);
		if (regExp.exactMatch(trigger)) {
			description = descriptions.value(key);
			// Format the description with index numbers found in the trigger
			if (description.contains("%1")) {
				int pos = 0;
				QRegExp indexRegExp("/(\\d+)/");
				while ((pos = indexRegExp.indexIn(trigger, pos)) != -1) {
					description = description.arg(indexRegExp.cap(1));
					pos += indexRegExp.matchedLength();
				}
			}
			return description;
		}
	}
	return "";
}

QString NotificationDescriptions::getTypeString(int type) const
{
	switch (type)
	{
	case WARNING:
		return tr("Warning");
	case ALARM:
		return tr("Alarm");
	case NOTIFICATION:
		return tr("Notification");
	default:
		Q_ASSERT(false);
		return "error";
	}
}

void NotificationDescriptions::addBatteryAlarms()
{
	mBatteryAlarms.insert("/Alarms/LowVoltage",							tr("Low voltage"));
	mBatteryAlarms.insert("/Alarms/HighVoltage",						tr("High voltage"));
	mBatteryAlarms.insert("/Alarms/HighCurrent",						tr("High current"));
	mBatteryAlarms.insert("/Alarms/HighChargeCurrent",					tr("High charge current"));
	mBatteryAlarms.insert("/Alarms/HighDischargeCurrent",				tr("High discharge current"));
	mBatteryAlarms.insert("/Alarms/HighChargeTemperature",				tr("High charge temperature"));
	mBatteryAlarms.insert("/Alarms/LowChargeTemperature",				tr("Low charge temperature"));
	mBatteryAlarms.insert("/Alarms/LowSoc",								tr("Low SOC"));
	mBatteryAlarms.insert("/Alarms/StateOfHealth",						tr("State of health"));
	mBatteryAlarms.insert("/Alarms/LowStarterVoltage",					tr("Low starter voltage"));
	mBatteryAlarms.insert("/Alarms/HighStarterVoltage",					tr("High starter voltage"));
	mBatteryAlarms.insert("/Alarms/LowTemperature",						tr("Low temperature"));
	mBatteryAlarms.insert("/Alarms/HighTemperature",					tr("High Temperature"));
	mBatteryAlarms.insert("/Alarms/MidVoltage",							tr("Mid-point voltage"));
	mBatteryAlarms.insert("/Alarms/LowFusedVoltage",					tr("Low-fused voltage"));
	mBatteryAlarms.insert("/Alarms/HighFusedVoltage",					tr("High-fused voltage"));
	mBatteryAlarms.insert("/Alarms/FuseBlown",							tr("Fuse blown"));
	mBatteryAlarms.insert("/Alarms/HighInternalTemperature",			tr("High internal temperature"));
	mBatteryAlarms.insert("/Alarms/InternalFailure",					tr("Internal failure"));
	mBatteryAlarms.insert("/Alarms/BatteryTemperatureSensor",			tr("Battery temperature sensor"));
	mBatteryAlarms.insert("/Alarms/CellImbalance",						tr("Cell imbalance"));
	mBatteryAlarms.insert("/Alarms/LowCellVoltage",						tr("Low cell voltage"));
	mBatteryAlarms.insert("/Alarms/Contactor",							tr("Bad contactor"));
	mBatteryAlarms.insert("/Alarms/BmsCable",							tr("BMS cable fault"));

	//RegExp will be evaluated and description formated
	mBatteryAlarms.insert("^/Distributor/\\d+/Alarms/Fuse/\\d+/Blown$",	tr("Fuse %2 of distributor %1 blown"));
	mBatteryAlarms.insert("^/Distributor/\\d+/Alarms/ConnectionLost$",	tr("Distributor %1 connection lost"));
}


void NotificationDescriptions::addDcMeterAlarms()
{
	mDcMeterAlarms.insert("/Alarms/LowVoltage",							tr("Low voltage"));
	mDcMeterAlarms.insert("/Alarms/HighVoltage",						tr("High voltage"));
	mDcMeterAlarms.insert("/Alarms/LowStarterVoltage",					tr("Low aux voltage"));
	mDcMeterAlarms.insert("/Alarms/HighStarterVoltage",					tr("High aux voltage"));
	mDcMeterAlarms.insert("/Alarms/LowTemperature",						tr("Low temperature"));
	mDcMeterAlarms.insert("/Alarms/HighTemperature",					tr("High Temperature"));
}

void NotificationDescriptions::addAlternatorAlarms()
{
	mAlternatorAlarms.insert("/Alarms/LowVoltage",						tr("Low voltage"));
	mAlternatorAlarms.insert("/Alarms/HighVoltage",						tr("High voltage"));
	mAlternatorAlarms.insert("/Alarms/LowStarterVoltage",				tr("Low aux voltage"));
	mAlternatorAlarms.insert("/Alarms/HighStarterVoltage",				tr("High aux voltage"));
	mAlternatorAlarms.insert("/Alarms/LowTemperature",					tr("Low temperature"));
	mAlternatorAlarms.insert("/Alarms/HighTemperature",					tr("High Temperature"));
}

void NotificationDescriptions::addVebusAlarms() {
	mVebusAlarms.insert("/Alarms/TemperatureSensor",					tr("Temperature sense error"));
	mVebusAlarms.insert("/Alarms/VoltageSensor",						tr("Voltage sense error"));
	mVebusAlarms.insert("/Alarms/LowBattery",							tr("Low battery voltage"));
	mVebusAlarms.insert("/Alarms/Ripple",								tr("High DC ripple"));
	mVebusAlarms.insert("/Alarms/PhaseRotation",						tr("Wrong phase rotation detected"));

	// Phase 1
	mVebusAlarms.insert("/Alarms/L1/HighTemperature",					tr("High Temperature on L1"));
	mVebusAlarms.insert("/Alarms/L1/Overload",							tr("Inverter overload on L1"));

	// Phase 2
	mVebusAlarms.insert("/Alarms/L2/HighTemperature",					tr("High Temperature on L2"));
	mVebusAlarms.insert("/Alarms/L2/Overload",							tr("Inverter overload on L2"));

	// Phase 3
	mVebusAlarms.insert("/Alarms/L3/HighTemperature",					tr("High Temperature on L3"));
	mVebusAlarms.insert("/Alarms/L3/Overload",							tr("Inverter overload on L3"));

	// Grid alarm
	mVebusAlarms.insert("/Alarms/GridLost",								tr("Grid lost"));

	// DC voltage and current alarms
	mVebusAlarms.insert("/Alarms/HighDcVoltage",						tr("High DC voltage"));
	mVebusAlarms.insert("/Alarms/HighDcCurrent",						tr("High DC current"));

	// VE.Bus BMS related alarms
	mVebusAlarms.insert("/Alarms/BmsPreAlarm",							tr("BMS pre-alarm"));
	mVebusAlarms.insert("/Alarms/BmsConnectionLost",					tr("BMS connection lost"));
}

void NotificationDescriptions::addMultiRsAlarms() {
	mMultiRsAlarms.insert("/Alarms/LowVoltage",							tr("Low batttery voltage"));
	mMultiRsAlarms.insert("/Alarms/HighVoltage",						tr("High battery voltage"));
	mMultiRsAlarms.insert("/Alarms/LowVoltageAcOut",					tr("Low AC voltage"));
	mMultiRsAlarms.insert("/Alarms/HighVoltageAcOut",					tr("High AC voltage"));
	mMultiRsAlarms.insert("/Alarms/HighTemperature",					tr("High temperature"));
	mMultiRsAlarms.insert("/Alarms/Overload",							tr("Inverter overload"));
	mMultiRsAlarms.insert("/Alarms/Ripple",								tr("High DC ripple"));
	mMultiRsAlarms.insert("/Alarms/LowSoc",								tr("Low SOC"));
}

void NotificationDescriptions::addSolarChargerAlarms()
{
	mSolarChargerAlarms.insert("/Alarms/LowVoltage",					tr("Low battery voltage"));
	mSolarChargerAlarms.insert("/Alarms/HighVoltage",					tr("High battery voltage"));

}

void NotificationDescriptions::addAcChargerAlarms()
{
	mSolarChargerAlarms.insert("/Alarms/LowVoltage",					tr("Low battery voltage"));
	mSolarChargerAlarms.insert("/Alarms/HighVoltage",					tr("High battery voltage"));
}

void NotificationDescriptions::addInverterAlarms()
{
	mInverterAlarms.insert("/Alarms/LowVoltage",						tr("Low batttery voltage"));
	mInverterAlarms.insert("/Alarms/HighVoltage",						tr("High battery voltage"));
	mInverterAlarms.insert("/Alarms/LowVoltageAcOut",					tr("Low AC voltage"));
	mInverterAlarms.insert("/Alarms/HighVoltageAcOut",					tr("High AC voltage"));
	mInverterAlarms.insert("/Alarms/LowTemperature",					tr("Low temperature"));
	mInverterAlarms.insert("/Alarms/HighTemperature",					tr("High temperature"));
	mInverterAlarms.insert("/Alarms/Overload",							tr("Inverter overload"));
	mInverterAlarms.insert("/Alarms/Ripple",							tr("High DC ripple"));
	mInverterAlarms.insert("/Alarms/LowSoc",							tr("Low SOC"));
}

void NotificationDescriptions::addGeneratorStartStopAlarms()
{
	mGeneratorStartStopAlarms.insert("/Alarms/NoGeneratorAtAcIn",		tr("Generator not detected at AC input"));
	mGeneratorStartStopAlarms.insert("/Alarms/ServiceIntervalExceeded", tr("Service interval exceeded"));
	mGeneratorStartStopAlarms.insert("/Alarms/AutoStartDisabled",		tr("Auto start function is disabled"));
}

void NotificationDescriptions::addDigitalInputAlarms()
{
	mDigitalInputAlarms.insert("/Alarm","");
}

void NotificationDescriptions::addVecanAlarms()
{
	mVecanAlarms.insert("/Alarms/SameUniqueNameUsed",					tr("Please set the VE.Can number to a free one"));
}

void NotificationDescriptions::addEssAlarms()
{
	mEssAlarms.insert("/Alarms/NoGridMeter",							tr("Grid meter not found #49"));
}

void NotificationDescriptions::addSystemCalcAlarms()
{
	mSystemCalcAlarms.insert("/Dc/Battery/Alarms/CircuitBreakerTripped",tr("Circuit breaker tripped"));
	mSystemCalcAlarms.insert("/Dvcc/Alarms/FirmwareInsufficient",		tr("DVCC with incompatible firmware #48"));

}

void NotificationDescriptions::addTankAlarms()
{
	mTankAlarms.insert("/Alarms/Low/State",								tr("Low level alarm"));
	mTankAlarms.insert("/Alarms/High/State",								tr("High level alarm"));
}

void NotificationDescriptions::addPlatformAlarms()
{
	mPlatformAlarms.insert("/Device/DataPartitionError", tr("#42 Storage is corrupt on this device"));
}
