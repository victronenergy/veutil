#include <veutil/qt/bms_error.hpp>

QString BmsError::getDescription(int errorNumber)
{
	QString result = "#" + QString::number(errorNumber) + " ";

	switch (errorNumber)
	{
	case VE_VDATA_BMS_ERROR_NONE:
		result += tr("No error");
		break;
	case VE_VDATA_BMS_ERROR_BATTERY_INIT:
	case VE_VDATA_BMS_ERROR_INITIALIZING:
		result += tr("Battery initialization error");
		break;
	case VE_VDATA_BMS_ERROR_NO_BATTERY_FOUND:
		result += tr("No batteries connected");
		break;
	case VE_VDATA_BMS_ERROR_UNKNOWN_PRODUCT:
		result += tr("Unknown battery");
		break;
	case VE_VDATA_BMS_ERROR_BAT_TYPE:
		result += tr("Different battery types");
		break;
	case VE_VDATA_BMS_ERROR_NR_OF_BAT:
		result += tr("No. of batteries incorrect");
		break;
	case VE_VDATA_BMS_ERROR_NO_SHUNT_FND:
		result += tr("Lynx Shunt not found");
		break;
	case VE_VDATA_BMS_ERROR_MEASURE:
		result += tr("Battery measure error");
		break;
	case VE_VDATA_BMS_ERROR_CALCULATE:
		result += tr("Internal calculation error");
		break;
	case VE_VDATA_BMS_ERROR_BAT_NR_SER:
		result += tr("No. of batteries in series incorrect");
		break;
	case VE_VDATA_BMS_ERROR_BAT_NR:
		result += tr("No. of batteries incorrect");
		break;
	case VE_VDATA_BMS_ERROR_IO_EXPANDER:
	case VE_VDATA_BMS_ERROR_HARDWARE_FAILURE:
		result += tr("Hardware error");
		break;
	case VE_VDATA_BMS_ERROR_WATCHDOG:
		result += tr("Watchdog error");
		break;
	case VE_VDATA_BMS_ERROR_OVER_VOLTAGE:
		result += tr("Over voltage");
		break;
	case VE_VDATA_BMS_ERROR_UNDER_VOLTAGE:
		result += tr("Under voltage");
		break;
	case VE_VDATA_BMS_ERROR_OVER_TEMPERATURE:
		result += tr("Over temperature");
		break;
	case VE_VDATA_BMS_ERROR_UNDER_TEMPERATURE:
		result += tr("Under temperature");
		break;
	case VE_VDATA_BMS_ERROR_UNDER_CHARGE_STANDBY:
		result += tr("Under-charge standby");
		break;
	case VE_VDATA_BMS_ERROR_ADC_FAILURE:
		result += tr("ADC error");
		break;
	case VE_VDATA_BMS_ERROR_SLAVE_FAILURE:
		result += tr("Battery comm. error");
		break;
	case VE_VDATA_BMS_ERROR_PRE_CHARGE:
	case VE_VDATA_BMS_ERROR_PRECHARGE_FAILED:
	case VE_VDATA_BMS_ERROR_VLOAD_HIGH_BEFORE_PRECHARGE:
		result += tr("Pre-Charge error");
		break;
	case VE_VDATA_BMS_ERROR_CONTACTOR:
	case VE_VDATA_BMS_ERROR_CONTACTOR_STARTUP:
		result += tr("Safety contactor error");
		break;
	case VE_VDATA_BMS_ERROR_SLAVE_UPDATE:
	case VE_VDATA_BMS_ERROR_SLAVE_UPDATE_UNAVAILABLE:
		result += tr("Battery update error");
		break;
	case VE_VDATA_BMS_ERROR_BMS_CABLE:
		result += tr("BMS cable error");
		break;
	case VE_VDATA_BMS_ERROR_REF_VOLTAGE_FAILURE:
		result += tr("Reference voltage failure");
		break;
	case VE_VDATA_BMS_ERROR_WRONG_SYSTEM_VOLTAGE:
		result += tr("Wrong system voltage");
		break;
	case VE_VDATA_BMS_ERROR_PRE_CHARGE_TIMEOUT:
		result += tr("Pre charge timeout");
		break;
	case VE_VDATA_BMS_ERROR_ATC_ATD_FAILURE:
		result += tr("ATC/ATD failure");
		break;
	case VE_VDATA_BMS_ERROR_CALIBRATION_DATA_LOST:
	case VE_VDATA_BMS_ERROR_CALIBRATION_DATA_LOST_OLD:
		result += tr("Calibration data lost");
		break;
	case VE_VDATA_BMS_ERROR_SETTINGS_DATA_INVALID:
	case VE_VDATA_BMS_ERROR_SETTINGS_DATA_INVALID_OLD:
		result += tr("Settings invalid");
		break;
	case VE_VDATA_BMS_ERROR_INTERLOCK:
		result += tr("Interlock");
		break;
	case VE_VDATA_BMS_ERROR_EMERGENCY_STOP:
		result += tr("Emergency stop");
		break;
	case VE_VDATA_BMS_ERROR_COMMUNICATION_TIMEOUT:
		result += tr("Communication timeout");
		break;
	case VE_VDATA_BMS_ERROR_SAFETY_LOCK: // MGE
	case VE_VDATA_BMS_ERROR_BATTERY_SAFETY_LOCK: // VE
		result += tr("Safety lock");
		break;
	case VE_VDATA_BMS_ERROR_TERMINAL_OVER_TEMPERATURE:
		result += tr("Terminal over temperature");
		break;
	case VE_VDATA_BMS_ERROR_HIGH_CELL_VOLTAGE:
		result += tr("High cell voltage");
		break;
	case VE_VDATA_BMS_ERROR_EXTERNAL_DISCONNECTION:
		result += tr("External disconnection");
		break;
	case VE_VDATA_BMS_ERROR_UNALLOWED_BATTERY_VOLTAGE:
		result += tr("Battery voltage not allowed");
		break;
	case VE_VDATA_BMS_ERROR_OC_STATUS_TRIGGERED:
		result += tr("Over current");
		break;
	case VE_VDATA_BMS_ERROR_VBAT_ADC_DIG_MISMATCH:
		result += tr("Battery voltage measurement mismatch");
		break;
	}

	return result;
}
