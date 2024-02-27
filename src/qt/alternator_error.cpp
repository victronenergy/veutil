#include <veutil/qt/alternator_error.hpp>

#include <QCoreApplication>
#include <QStringList>

class Wakespeed
{
	Q_DECLARE_TR_FUNCTIONS(Wakespeed)

public:
	static QString getDescription(QString errorId);

private:
	enum class FaultCode;

	Wakespeed() = default;
};

QString AlternatorError::getDescription(QString errorId)
{
	if (errorId == "")
		return tr("No error");
	if (errorId.startsWith("wakespeed:", Qt::CaseInsensitive))
		return Wakespeed::getDescription(errorId);
	return tr("Unknown error: %1").arg(errorId);
}

/*
 * From the "Wakespeed Communications and Configurations Guide":
 *
 * Error codes. If there is a FAULTED status, the variable errorCode will
 * contain one of these...
 *
 * Note: at this time, only one error code is retained. Multi-faults will
 * only show the last one in the checking tree. Errors with + 0x8000 on them
 * will cause the regulator to re-start, others will freeze the regulator.
 * (Note combinations like 10, and 11 are not used. Because one cannot flash
 * out 0's, and kind of hard to tell if 11 is a 1+1, or a real slow 2+0)
 */
enum class Wakespeed::FaultCode {
	FC_NONE							= 0,				// No fault

	// Battery temp exceeded limit
	FC_LOOP_BAT_TEMP				= 12,
	// Battery Volts exceeded upper limit
	FC_LOOP_BAT_VOLTS				= 13,
	// Battery Volts exceeded lower limit, either damaged or sensing wire missing
	// (or engine not started!).
	FC_LOOP_BAT_LOWV				= 14 /*+ 0x8000*/,
	// Voltage at Vbat+ exceeded Max Bat Volts as defined by $CPB:
	FC_LOOP_BAT_MAXV				= 15,
	// Battery Temperature is shorted (Defective)
	FC_LOOP_SHORTED_BAT_TEMP		= 16,

	// Alternator temp exceeded limit
	FC_LOOP_ALT_TEMP				= 21,
	// Alternator seems to be spinning way to fast!
	FC_LOOP_ALT_RPMs				= 22,
	// Alternator temp reached / exceeded while ramping - this can NOT be right,
	// to reach target while ramping means way too risky.
	FC_LOOP_ALT_TEMP_RAMP			= 24,

	// Internal Field FET temperature exceed limit.
	FC_SYS_FET_TEMP					= 41,
	// A 'Required' sensor is missing, and we are configured to FAULT out.
	FC_SYS_REQIRED_SENSOR			= 42,
	// No/low voltage has been sensed on the VAlt+ line, blow fuse?
	FC_NO_VALT_VOLTAGE				= 43 /*+ 0x8000*/,
	// There is excessive voltage offset (5V or more) between VAlt+ and VBat+
	// sense lines. (This test is disabled in a split system, ala 48v battery,
	// 12v field power source on Alt+ line)
	FC_EXCESSIVE_VALT_OFFSET		= 44,
	// Voltage at VAlt+ exceeded Max Bat Volts as defined by $CPB:
	FC_LOOP_VALT_MAXV				= 45,
	// Voltage greatly exceeded expected upper limit battery limit as measured at VAlt+
	FC_LOOP_ALT_HIGHV				= 46,

	// a CAN has been received that the battery charging bus has been disconnected.
	FC_CAN_BATTERY_DISCONNECTED		= 51,
	// a command has been sent asking for the battery bus to be disconnected!
	FC_CAN_BATTERY_HVL_DISCONNECTED = 52,
	// Battery Instance number is out of range (needs to be from 1..100)
	FC_LOG_BATTINST					= 53,
	// Too many different BMS's have been found in the aggregation range,
	// internal table size exceeded.
	FC_TOO_MANY_AGGERGATION			= 54,
	// AEBus device (Discovery battery) has send a warning or fault status.
	// As there is no fore-warning of a disconnect, treat all warnings as a
	// pending disconnect and fault. But then do autorestart to see if it clears.
	FC_CAN_AEBUS_FAULTED			= 55 /*+ 0x8000*/,
	// Too many VEreg (Victron) devices for us to track
	FC_TOO_MANY_VEREG_DEVICE		= 56,
	// Waveshare noted that a command has been sent asking for the battery bus
	// to be disconnected due to Low Voltage!
	FC_CAN_BATTERY_LVL_DISCONNECTED	= 57,
	// Waveshare noted that a command has been sent asking for the battery bus
	// to be disconnected due to high current (Charge and/or discharge).
	FC_CAN_BATTERY_HC_DISCONNECTED	= 58,
	// Waveshare noted that a command has been sent asking for the battery bus
	// to be disconnected due to High Battery Temperature.
	FC_CAN_BATTERY_HT_DISCONNECTED	= 59,
	// Waveshare noted that a command has been sent asking for the battery bus
	// to be disconnected due to Low Battery Temperature.
	FC_CAN_BATTERY_LT_DISCONNECTED	= 61,

	// The 9x codes are special ones, they do not cause a true FAULT, but
	// indicate some alternator condition and mode of operation. Mostly these
	// are used to support DM_RV (aka, ISO Diag) and CAN connected monitors
	// such as the Victron Cerbo.

	// Used to signal that an existing BMS sync has been lost and we are in an
	// alt mode (ala, Gethome)
	FC_CAN_BMS_SYNC_LOST			= 91,
	// Use to indicate that the reg has been forced into Idle via perhaps the
	// Feature-in line, or some RPM based trigger.
	FC_FORCED_TO_IDLE				= 92,

	// DCDC converter failed to come ready, or CAN transmission error
	FC_DCDC_NOTREADY				= 201,
	// Primary Battery (HS) of DC-DC converter Over-voltage trip
	FC_DCDC_HS_OVP					= 202,
	// Primary Battery (HS) of DC-DC converter Under-voltage trip
	FC_DCDC_HS_UVP					= 203 /*+ 0x8000*/,
	// Secondary Battery (LS) of DC-DC converter Over-voltage trip
	FC_DCDC_LS_OVP					= 204,
	// Secondary Battery (LS) of DC-DC converter Under-voltage trip
	FC_DCDC_LS_UVP					= 205 /*+ 0x8000*/,
	// DCDC Convert too hot.
	FC_DCDC_OVER_TEMP				= 206 /*+ 0x8000*/,
	// Some value exceeded the selected DC-DC converter limits.
	FC_DCDC_MISCOFIG				= 207,
};

QString Wakespeed::getDescription(QString errorId)
{
	auto parts = errorId.split("-");
	if (parts.length() < 1)
		return tr("Unknown error: ") + errorId;

	bool ok;
	auto faultCode = parts[1].toUInt(&ok);
	if (!ok)
		return tr("Unknown error: ") + errorId;

	auto result = "#" + QString::number(faultCode) + " ";

	if ((faultCode >= 31 && faultCode <= 39) ||
		(faultCode >= 71 && faultCode <= 79) ||
		(faultCode >= 81 && faultCode <= 89) ||
		(faultCode >= 100 && faultCode <= 199)) {
		return result + tr("Internal error");
	}

	switch (static_cast<FaultCode>(faultCode)) {
	case FaultCode::FC_NONE:
		result += tr("No error");
		break;
	case FaultCode::FC_LOOP_BAT_TEMP:
		result += tr("Battery high temperature");
		break;
	case FaultCode::FC_LOOP_BAT_VOLTS:
		result += tr("Battery high voltage");
		break;
	case FaultCode::FC_LOOP_BAT_LOWV:
		result += tr("Battery low voltage");
		break;
	case FaultCode::FC_LOOP_BAT_MAXV:
		result += tr("Battery voltage exceeded configured max");
		break;
	case FaultCode::FC_LOOP_SHORTED_BAT_TEMP:
		result += tr("Battery temperature shorted");
		break;
	case FaultCode::FC_LOOP_ALT_TEMP:
	case FaultCode::FC_LOOP_ALT_TEMP_RAMP:
		result += tr("Alternator high temperature");
		break;
	case FaultCode::FC_LOOP_ALT_RPMs:
		result += tr("Alternator high RPM");
		break;
	case FaultCode::FC_SYS_FET_TEMP:
		result += tr("Field drive FET high temperature");
		break;
	case FaultCode::FC_SYS_REQIRED_SENSOR:
		result += tr("Required sensor missing");
		break;
	case FaultCode::FC_NO_VALT_VOLTAGE:
		result += tr("Alternator low voltage");
		break;
	case FaultCode::FC_EXCESSIVE_VALT_OFFSET:
		result += tr("Alternator high voltage offset");
		break;
	case FaultCode::FC_LOOP_VALT_MAXV:
		result += tr("Alternator Voltage exceeded configured max");
		break;
	case FaultCode::FC_LOOP_ALT_HIGHV:
		result += tr("Alternator high voltage");
		break;
	case FaultCode::FC_CAN_BATTERY_DISCONNECTED:
		result += tr("Battery disconnected");
		break;
	case FaultCode::FC_CAN_BATTERY_HVL_DISCONNECTED:
		result += tr("Battery high voltage disconnect");
		break;
	case FaultCode::FC_LOG_BATTINST:
		result += tr("Battery instance ouf of range");
		break;
	case FaultCode::FC_TOO_MANY_AGGERGATION:
		result += tr("Too many BMS's");
		break;
	case FaultCode::FC_CAN_AEBUS_FAULTED:
		result += tr("Battery about to disconnect");
		break;
	case FaultCode::FC_TOO_MANY_VEREG_DEVICE:
		result += tr("Too many devices to track");
		break;
	case FaultCode::FC_CAN_BATTERY_LVL_DISCONNECTED:
		result += tr("Battery low voltage disconnect");
		break;
	case FaultCode::FC_CAN_BATTERY_HC_DISCONNECTED:
		result += tr("Battery high current disconnect");
		break;
	case FaultCode::FC_CAN_BATTERY_HT_DISCONNECTED:
		result += tr("Battery high temperature disconnect");
		break;
	case FaultCode::FC_CAN_BATTERY_LT_DISCONNECTED:
		result += tr("Battery low temperature disconnect");
		break;
	case FaultCode::FC_CAN_BMS_SYNC_LOST:
		result += tr("BMS connection lost");
		break;
	case FaultCode::FC_FORCED_TO_IDLE:
		result += tr("ATC Disabled");
		break;
	case FaultCode::FC_DCDC_NOTREADY:
		result += tr("DC/DC converter not ready");
		break;
	case FaultCode::FC_DCDC_HS_OVP:
		result += tr("DC/DC high primary voltage");
		break;
	case FaultCode::FC_DCDC_HS_UVP:
		result += tr("DC/DC low primary voltage");
		break;
	case FaultCode::FC_DCDC_LS_OVP:
		result += tr("DC/DC high secondary voltage");
		break;
	case FaultCode::FC_DCDC_LS_UVP:
		result += tr("DC/DC low secondary voltage");
		break;
	case FaultCode::FC_DCDC_OVER_TEMP:
		result += tr("DC/DC high temperature");
		break;
	case FaultCode::FC_DCDC_MISCOFIG:
		result += tr("DC/DC misconfiguration");
		break;
	}

	return result;
}
