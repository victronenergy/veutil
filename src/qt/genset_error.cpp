#include <veutil/qt/genset_error.hpp>

#include <QCoreApplication>
#include <QStringList>

class FischerPanda
{
	Q_DECLARE_TR_FUNCTIONS(FischerPanda)

public:
	static QString getDescription(QString errorId, int nrOfPhases);

private:
	FischerPanda() = default;
};

class DSE
{
	Q_DECLARE_TR_FUNCTIONS(DSE)

public:
	static QString getDescription(QString errorId);

private:
	DSE() = default;
};

class Hatz
{
	Q_DECLARE_TR_FUNCTIONS(Hatz)

public:
	static QString getDescription(QString errorId);

private:
	static constexpr quint32 SpnAndFmiToNumber(quint32 spn, quint8 fmi) {
		return spn << 8 | fmi;
	}

	Hatz() = default;
};

class CRE
{
	Q_DECLARE_TR_FUNCTIONS(CRE)

public:
	static QString getDescription(QString errorId);

private:
	CRE() = default;
};

class DEIF
{
	Q_DECLARE_TR_FUNCTIONS(DEIF)

public:
	static QString getDescription(QString errorId);

private:
	DEIF() = default;
};

class Cummins
{
	Q_DECLARE_TR_FUNCTIONS(CUMMINS)

public:
	static QString getDescription(QString errorId);

private:
	static constexpr quint32 SpnAndFmiToNumber(quint32 spn, quint8 fmi) {
		return spn << 8 | fmi;
	}

	Cummins() = default;
};

QString GensetError::getDescription(QString errorId, int nrOfPhases)
{
	if (errorId == "")
		return tr("No error");
	if (errorId.startsWith("fischerpanda:", Qt::CaseInsensitive))
		return FischerPanda::getDescription(errorId, nrOfPhases);
	if (errorId.startsWith("dse:", Qt::CaseInsensitive))
		return DSE::getDescription(errorId);
	if (errorId.startsWith("hatz:", Qt::CaseInsensitive))
		return Hatz::getDescription(errorId);
	if (errorId.startsWith("cre:", Qt::CaseInsensitive))
		return CRE::getDescription(errorId);
	if (errorId.startsWith("deif:", Qt::CaseInsensitive))
		return DEIF::getDescription(errorId);
	if (errorId.startsWith("cummins:", Qt::CaseInsensitive))
		return Cummins::getDescription(errorId);
	return tr("Unknown error: %1").arg(errorId);
}

/* Source: https://knowledgebase.fischerpanda.de/?page_id=25 */
QString FischerPanda::getDescription(QString errorId, int nrOfPhases)
{
	auto parts = errorId.split("-");
	if (parts.length() < 1)
		return tr("Unknown error: ") + errorId;

	bool ok;
	auto errorNumber = parts[1].toUInt(&ok);
	if (!ok)
		return tr("Unknown error: ") + errorId;

	auto result = "#" + QString::number(errorNumber) + " ";

	switch (errorNumber)
	{
		case 0: result += tr("No error"); break;

		/* Errors (alarms) < 128 with two codes; break; 0-63: low alarm; break; 64-127: high alarm */
		case  1 + 0x00: result += nrOfPhases > 1 ? tr("AC voltage L1 too low") : tr("AC voltage too low"); break;
		case  1 + 0x40: result += nrOfPhases > 1 ? tr("AC voltage L1 too high") : tr("AC voltage too high"); break;
		case  2 + 0x00: result += nrOfPhases > 1 ? tr("AC frequency L1 too low") : tr("AC frequency too low"); break;
		case  2 + 0x40: result += nrOfPhases > 1 ? tr("AC frequency L1 too high") : tr("AC frequency too high"); break;
		case  3 + 0x40: result += nrOfPhases > 1 ? tr("AC current L1 too high") : tr("AC current too high"); break;
		case  4 + 0x40: result += nrOfPhases > 1 ? tr("AC power L1 too high") : tr("AC power too high"); break;
		case  5 + 0x00: result += tr("Emergency stop"); break;
		case  6 + 0x40: result += tr("Servo current too high"); break;
		case  7 + 0x00: result += tr("Oil pressure too low"); break;
		case  7 + 0x40: result += tr("Oil pressure too high"); break;
		case  8 + 0x00: result += tr("Engine temperature too low"); break;
		case  8 + 0x40: result += tr("Engine temperature too high"); break;
		case  9 + 0x00: result += tr("Winding temperature too low"); break;
		case  9 + 0x40: result += tr("Winding temperature too high"); break;
		case 10 + 0x00: result += tr("Exhaust temperature too low"); break;
		case 10 + 0x40: result += tr("Exhaust temperature too high"); break;
		case 11 + 0x00: result += tr("Electronic temperature low"); break;
		case 11 + 0x40: result += tr("Electronic temperature high"); break;
		case 13 + 0x00: result += tr("Starter voltage too low"); break;
		case 13 + 0x40: result += tr("Starter current too high"); break;
		case 14 + 0x00: result += tr("Glow voltage too low"); break;
		case 14 + 0x40: result += tr("Glow current too high"); break;
		case 15 + 0x00: result += tr("Cold-Start-Aid voltage too high"); break;
		case 15 + 0x40: result += tr("Cold-Start-Aid current too high"); break;
		case 16 + 0x00: result += tr("Fuel holding magnet voltage too low"); break;
		case 16 + 0x40: result += tr("Fuel holding magnet current too high"); break;
		case 17 + 0x00: result += tr("Stop solenoid hold coil voltage too low"); break;
		case 17 + 0x40: result += tr("Stop solenoid hold coil current too high"); break;
		case 18 + 0x00: result += tr("Stop solenoid pull coil voltage too low "); break;
		case 18 + 0x40: result += tr("Stop solenoid pull coil current too high"); break;
		case 19 + 0x00: result += tr("Fan/water pump voltage too low"); break;
		case 19 + 0x40: result += tr("Fan/water pump current too high"); break;
		case 20 + 0x00: result += tr("Current sensor voltage low"); break;
		case 20 + 0x40: result += tr("Current sensor current high"); break;
		case 21 + 0x00: result += tr("Boost output voltage too low"); break;
		case 21 + 0x40: result += tr("Boost output current too high"); break;
		case 22 + 0x00: result += tr("Bus supply voltage too low"); break;
		case 22 + 0x40: result += tr("Bus supply current too high"); break;
		case 25 + 0x00: result += tr("Starter battery voltage too low"); break;
		case 25 + 0x40: result += tr("Starter battery voltage too high"); break;
		case 26 + 0x00: result += tr("Rotation too low"); break;
		case 26 + 0x40: result += tr("Rotation too high"); break;
		case 28 + 0x00: result += tr("Unexpected stop/problem with fuel supply"); break;
		case 29 + 0x00: result += tr("Power contactor voltage too low"); break;
		case 29 + 0x40: result += tr("Power contactor current too high"); break;
		case 30 + 0x00: result += tr("AC voltage L2 too low"); break;
		case 30 + 0x40: result += tr("AC voltage L2 too high"); break;
		case 31 + 0x00: result += tr("AC frequency L2 too low"); break;
		case 31 + 0x40: result += tr("AC frequency L2 too high"); break;
		case 32 + 0x40: result += tr("AC current L2 too high"); break;
		case 33 + 0x40: result += tr("AC power L2 too high"); break;
		case 34 + 0x00: result += tr("AC voltage L3 too low"); break;
		case 34 + 0x40: result += tr("AC voltage L3 too high"); break;
		case 35 + 0x00: result += tr("AC frequency L3 too low"); break;
		case 35 + 0x40: result += tr("AC frequency L3 too high"); break;
		case 36 + 0x40: result += tr("AC current L3 too high"); break;
		case 37 + 0x40: result += tr("AC power L3 too high"); break;
		case 38 + 0x00: result += tr("Output Inverter voltage too low"); break;
		case 38 + 0x40: result += tr("Output Inverter current too  high"); break;
		case 39 + 0x00: result += tr("Universal output (1A) voltage too low"); break;
		case 39 + 0x40: result += tr("Universal output (1A) current too high"); break;
		case 40 + 0x00: result += tr("Universal output (5A) voltage too low"); break;
		case 40 + 0x40: result += tr("Universal output (5A) current too high"); break;
		case 41 + 0x00: result += tr("AGT DC voltage 1 low"); break;
		case 41 + 0x40: result += tr("AGT DC voltage 1 high"); break;
		case 42 + 0x00: result += tr("AGT DC current 1 low"); break;
		case 42 + 0x40: result += tr("AGT DC current 1 high"); break;
		case 43 + 0x00: result += tr("AGT DC voltage 2 low"); break;
		case 43 + 0x40: result += tr("AGT DC voltage 2 high"); break;
		case 44 + 0x00: result += tr("AGT DC current 2 low"); break;
		case 44 + 0x40: result += tr("AGT DC current 2 high"); break;
		case 45 + 0x00: result += tr("AGT B6 cooler low"); break;
		case 45 + 0x40: result += tr("AGT B6 cooler high"); break;
		case 46 + 0x00: result += tr("AGT B6 rail (-) low"); break;
		case 46 + 0x40: result += tr("AGT B6 rail (-) high"); break;
		case 47 + 0x00: result += tr("AGT B6 rail (+) low"); break;
		case 47 + 0x40: result += tr("AGT B6 rail (+) high"); break;
		case 62 + 0x00: result += tr("Fuel temperature too low"); break;
		case 62 + 0x40: result += tr("Fuel temperature too high"); break;
		case 63 + 0x00: result += tr("Fuel level too low"); break;

		/* Errors (>= 128) with one code */
		case 130: result += tr("Lost control unit"); break;
		case 131: result += tr("Lost panel"); break;
		case 132: result += tr("Service needed"); break;
		case 133: result += tr("Lost 3-phase module"); break;
		case 134: result += tr("Lost AGT module"); break;
		case 135: result += tr("Synchronization failure"); break;
		case 136: result += tr("Lost external ECU"); break;
		case 137: result += tr("Intake airfilter"); break;
		case 138: result += tr("Diagnostic message (ECU)"); break;
		case 139: result += tr("Lost sync. module"); break;
		case 140: result += tr("Load-balance failed"); break;
		case 141: result += tr("Sync-mode deactivated"); break;
		case 142: result += tr("Red Stop Lamp (RSL)"); break;
		case 143: result += tr("Amber Warning Lamp (AWL)"); break;
		case 144: result += tr("Malfunction Indicator Lamp (MIL)"); break;
		case 145: result += tr("Protect Lamp (PL)"); break;
		case 148: result += tr("Rotating field wrong"); break;
		case 149: result += tr("Fuel level sensor lost"); break;
		case 167: result += tr("Starting without inverter"); break;
		case 168: result += tr("Bus #1 dead"); break;
		case 169: result += tr("Start request denied"); break;
		case 170: result += tr("Remote start denied"); break;
		case 171: result += tr("Forced switch off load relay"); break;
		case 172: result += tr("Synchronization Module is offline"); break;
		case 173: result += tr("Lost BMS"); break;
		case 174: result += tr("Converter DC Link Voltage Low/Reverse"); break;
		case 175: result += tr("Converter DC Link Current Low"); break;
		case 176: result += tr("Converter DC Precharge Voltage Low"); break;
		case 177: result += tr("Converter DC Precharge Voltage High"); break;
		case 178: result += tr("Converter IGBT/MOSFET Driver Error"); break;
		case 179: result += tr("Converter Error Power Control Loop"); break;
		case 180: result += tr("Converter AC Frequency Detection"); break;
		case 181: result += tr("Converter Control Value Fail"); break;
		case 245: result += tr("Factory setting changed"); break;
		case 251: result += tr("Parameter changed in admin mode"); break;
		case 252: result += tr("Manual Intervention (ext. System)"); break;

		/* Error codes for iControl only */
		case 150: result += tr("Init failed"); break;
		case 151: result += tr("Watchdog"); break;
		case 152: result += tr("Inverter temperature high L1"); break;
		case 153: result += tr("Inverter temperature high L2"); break;
		case 154: result += tr("Inverter temperature high L3"); break;
		case 155: result += tr("Inverter temperature high DC link"); break;
		case 156: result += tr("Inverter overload"); break;
		case 157: result += tr("Inverter communication lost"); break;
		case 163: result += tr("DC overload"); break;
		case 164: result += tr("DC overvoltage"); break;
		case 165: result += tr("Emergency stop"); break;
		case 166: result += tr("No connection"); break;
	}

	return result;
}

QString DSE::getDescription(QString errorId)
{
	auto parts = errorId.split("-");
	if (parts.length() < 1)
		return tr("Unknown error: ") + errorId;

	bool ok;
	auto errorNumber = parts[1].toUInt(&ok);
	if (!ok)
		return tr("Unknown error: ") + errorId;

	switch (errorNumber) {
	/* DSE error codes 0x1000 to 0x10FF: DSE old alarm system */
	case 0x1000: return "Emergency stop";
	case 0x1001: return "Low oil pressure";
	case 0x1002: return "High coolant temperature";
	case 0x1003: return "High oil temperature";
	case 0x1004: return "Under speed";
	case 0x1005: return "Over speed";
	case 0x1006: return "Fail to start";
	case 0x1007: return "Fail to come to rest";
	case 0x1008: return "Loss of speed sensing";
	case 0x1009: return "Generator low voltage";
	case 0x100a: return "Generator high voltage";
	case 0x100b: return "Generator low frequency";
	case 0x100c: return "Generator high frequency";
	case 0x100d: return "Generator high current";
	case 0x100e: return "Generator earth fault";
	case 0x100f: return "Generator reverse power";
	case 0x1010: return "Air flap";
	case 0x1011: return "Oil pressure sender fault";
	case 0x1012: return "Coolant temperature sender fault";
	case 0x1013: return "Oil temperature sender fault";
	case 0x1014: return "Fuel level sender fault";
	case 0x1015: return "Magnetic pickup fault";
	case 0x1016: return "Loss of AC speed signal";
	case 0x1017: return "Charge alternator failure";
	case 0x1018: return "Low battery voltage";
	case 0x1019: return "High battery voltage";
	case 0x101a: return "Low fuel level";
	case 0x101b: return "High fuel level";
	case 0x101c: return "Generator failed to close";
	case 0x101d: return "Mains failed to close";
	case 0x101e: return "Generator failed to open";
	case 0x101f: return "Mains failed to open";
	case 0x1020: return "Mains low voltage";
	case 0x1021: return "Mains high voltage";
	case 0x1022: return "Bus failed to close";
	case 0x1023: return "Bus failed to open";
	case 0x1024: return "Mains low frequency";
	case 0x1025: return "Mains high frequency";
	case 0x1026: return "Mains failed";
	case 0x1027: return "Mains phase rotation wrong";
	case 0x1028: return "Generator phase rotation wrong";
	case 0x1029: return "Maintenance due";
	case 0x102a: return "Clock not set";
	case 0x102b: return "Local LCD configuration lost";
	case 0x102c: return "Local telemetry configuration lost";
	case 0x102d: return "Control unit not calibrated";
	case 0x102e: return "Modem power fault";
	case 0x102f: return "Generator short circuit";
	case 0x1030: return "Failure to synchronise";
	case 0x1031: return "Bus live";
	case 0x1032: return "Scheduled run";
	case 0x1033: return "Bus phase rotation wrong";
	case 0x1034: return "Priority selection error";
	case 0x1035: return "Multiset communications (MSC) data error";
	case 0x1036: return "Multiset communications (MSC) ID error";
	case 0x1037: return "Multiset communications (MSC) failure";
	case 0x1038: return "Multiset communications (MSC) too few sets";
	case 0x1039: return "Multiset communications (MSC) alarms inhibited";
	case 0x103a: return "Multiset communications (MSC) old version units";
	case 0x103b: return "Mains reverse power";
	case 0x103c: return "Minimum sets not reached";
	case 0x103d: return "Insufficient capacity available";
	case 0x103e: return "Expansion input unit not calibrated";
	case 0x103f: return "Expansion input unit failure";
	case 0x1040: return "Auxiliary sender 1 low";
	case 0x1041: return "Auxiliary sender 1 high";
	case 0x1042: return "Auxiliary sender 1 fault";
	case 0x1043: return "Auxiliary sender 2 low";
	case 0x1044: return "Auxiliary sender 2 high";
	case 0x1045: return "Auxiliary sender 2 fault";
	case 0x1046: return "Auxiliary sender 3 low";
	case 0x1047: return "Auxiliary sender 3 high";
	case 0x1048: return "Auxiliary sender 3 fault";
	case 0x1049: return "Auxiliary sender 4 low";
	case 0x104a: return "Auxiliary sender 4 high";
	case 0x104b: return "Auxiliary sender 4 fault";
	case 0x104c: return "Engine control unit (ECU) link lost";
	case 0x104d: return "Engine control unit (ECU) failure";
	case 0x104e: return "Engine control unit (ECU) error";
	case 0x104f: return "Low coolant temperature";
	case 0x1050: return "Out of sync";
	case 0x1051: return "Low Oil Pressure Switch";
	case 0x1052: return "Alternative Auxiliary Mains Fail";
	case 0x1053: return "Loss of excitation";
	case 0x1054: return "Mains kW Limit";
	case 0x1055: return "Negative phase sequence";
	case 0x1056: return "Mains ROCOF";
	case 0x1057: return "Mains vector shift";
	case 0x1058: return "Mains G59 low frequency";
	case 0x1059: return "Mains G59 high frequency";
	case 0x105a: return "Mains G59 low voltage";
	case 0x105b: return "Mains G59 high voltage";
	case 0x105c: return "Mains G59 trip";
	case 0x105d: return "Generator kW Overload";
	case 0x105e: return "Engine Inlet Temperature high";
	case 0x105f: return "Bus 1 live";
	case 0x1060: return "Bus 1 phase rotation wrong";
	case 0x1061: return "Bus 2 live";
	case 0x1062: return "Bus 2 phase rotation wrong";

	/* DSE error codes 0x1100 to 0x11FF: DSE 61xx MKII */
	case 0x1100: return "Emergency stop";
	case 0x1101: return "Low oil pressure";
	case 0x1102: return "High coolant temperature";
	case 0x1103: return "Low coolant temperature";
	case 0x1104: return "Under speed";
	case 0x1105: return "Over speed";
	case 0x1106: return "Generator Under frequency";
	case 0x1107: return "Generator Over frequency";
	case 0x1108: return "Generator low voltage";
	case 0x1109: return "Generator high voltage";
	case 0x110a: return "Battery low voltage";
	case 0x110b: return "Battery high voltage";
	case 0x110c: return "Charge alternator failure";
	case 0x110d: return "Fail to start";
	case 0x110e: return "Fail to stop";
	case 0x110f: return "Generator fail to close";
	case 0x1110: return "Mains fail to close";
	case 0x1111: return "Oil pressure sender fault";
	case 0x1112: return "Loss of magnetic pick up";
	case 0x1113: return "Magnetic pick up open circuit";
	case 0x1114: return "Generator high current";
	case 0x1115: return "Calibration lost";
	case 0x1116: return "Low fuel level";
	case 0x1117: return "CAN ECU Warning";
	case 0x1118: return "CAN ECU Shutdown";
	case 0x1119: return "CAN ECU Data fail";
	case 0x111a: return "Low oil level switch";
	case 0x111b: return "High temperature switch";
	case 0x111c: return "Low fuel level switch";
	case 0x111d: return "Expansion unit watchdog alarm";
	case 0x111e: return "kW overload alarm";
	case 0x1123: return "Maintenance alarm";
	case 0x1124: return "Loading frequency alarm";
	case 0x1125: return "Loading voltage alarm";
	case 0x112e: return "ECU protect";
	case 0x112f: return "ECU Malfunction";
	case 0x1130: return "ECU Information";
	case 0x1131: return "ECU Shutdown";
	case 0x1132: return "ECU Warning";
	case 0x1133: return "ECU HEST";
	case 0x1135: return "ECU Water In Fuel";
	case 0x1139: return "High fuel level";
	case 0x113a: return "DEF Level Low";
	case 0x113b: return "SCR Inducement";

	/* DSE error codes 0x1200 to 0x12FF: DSE 72xx/73xx/61xx/74xx MKII family */
	case 0x1200: return "Emergency stop";
	case 0x1201: return "Low oil pressure";
	case 0x1202: return "High coolant temperature";
	case 0x1203: return "Low coolant temperature";
	case 0x1204: return "Under speed";
	case 0x1205: return "Over speed";
	case 0x1206: return "Generator Under frequency";
	case 0x1207: return "Generator Over frequency";
	case 0x1208: return "Generator low voltage";
	case 0x1209: return "Generator high voltage";
	case 0x120a: return "Battery low voltage";
	case 0x120b: return "Battery high voltage";
	case 0x120c: return "Charge alternator failure";
	case 0x120d: return "Fail to start";
	case 0x120e: return "Fail to stop";
	case 0x120f: return "Generator fail to close";
	case 0x1210: return "Mains fail to close";
	case 0x1211: return "Oil pressure sender fault";
	case 0x1212: return "Loss of magnetic pick up";
	case 0x1213: return "Magnetic pick up open circuit";
	case 0x1214: return "Generator high current";
	case 0x1215: return "Calibration lost";
	case 0x1216: return "Low fuel level";
	case 0x1217: return "CAN ECU Warning";
	case 0x1218: return "CAN ECU Shutdown";
	case 0x1219: return "CAN ECU Data fail";
	case 0x121a: return "Low oil level switch";
	case 0x121b: return "High temperature switch";
	case 0x121c: return "Low fuel level switch";
	case 0x121d: return "Expansion unit watchdog alarm";
	case 0x121e: return "kW overload alarm";
	case 0x121f: return "Negative phase sequence current alarm";
	case 0x1220: return "Earth fault trip alarm";
	case 0x1221: return "Generator phase rotation alarm";
	case 0x1222: return "Auto Voltage Sense Fail";
	case 0x1223: return "Maintenance alarm";
	case 0x1224: return "Loading frequency alarm";
	case 0x1225: return "Loading voltage alarm";
	case 0x1226: return "Fuel usage running";
	case 0x1227: return "Fuel usage stopped";
	case 0x1228: return "Protections disabled";
	case 0x1229: return "Protections blocked";
	case 0x122a: return "Generator Short Circuit";
	case 0x122b: return "Mains High Current";
	case 0x122c: return "Mains Earth Fault";
	case 0x122d: return "Mains Short Circuit";
	case 0x122e: return "ECU protect";
	case 0x122f: return "ECU Malfunction";
	case 0x1230: return "ECU Information";
	case 0x1231: return "ECU Shutdown";
	case 0x1232: return "ECU Warning";
	case 0x1233: return "ECU Electrical Trip";
	case 0x1234: return "ECU After treatment";
	case 0x1235: return "ECU Water In Fuel";
	case 0x1236: return "Generator Reverse Power";
	case 0x1237: return "Generator Positive VAr";
	case 0x1238: return "Generator Negative VAr";
	case 0x1239: return "LCD Heater Low Voltage";
	case 0x123a: return "LCD Heater High Voltage";
	case 0x123b: return "DEF Level Low";
	case 0x123c: return "SCR Inducement";
	case 0x123d: return "MSC Old version";
	case 0x123e: return "MSC ID alarm";
	case 0x123f: return "MSC failure";
	case 0x1240: return "MSC priority Error";
	case 0x1241: return "Fuel Sender open circuit";
	case 0x1242: return "Over speed runaway";
	case 0x1243: return "Over frequency run away";
	case 0x1244: return "Coolant sensor open circuit";
	case 0x1245: return "Remote display link lost";
	case 0x1246: return "Fuel tank bund level";
	case 0x1247: return "Charge air temperature";
	case 0x1248: return "Fuel level high";
	case 0x1249: return "Gen breaker failed to open (v5.0+)";
	case 0x124a: return "Mains breaker failed to open (v5.0+) – 7x20 only";
	case 0x124b: return "Fail to synchronise (v5.0+) – 7x20 only";
	case 0x124c: return "AVR Data Fail (v5.0+)";
	case 0x124d: return "AVR DM1 Red Stop Lamp (v5.0+)";
	case 0x124e: return "Escape Mode (v5.0+)";
	case 0x124f: return "Coolant high temp electrical trip (v5.0+)";

	/* DSE error codes 0x1300 to 0x13FF: DSE 8xxx family */
	case 0x1300: return "Emergency stop";
	case 0x1301: return "Low oil pressure";
	case 0x1302: return "High coolant temperature";
	case 0x1303: return "Low coolant temperature";
	case 0x1304: return "Under speed";
	case 0x1305: return "Over speed";
	case 0x1306: return "Generator Under frequency";
	case 0x1307: return "Generator Over frequency";
	case 0x1308: return "Generator low voltage";
	case 0x1309: return "Generator high voltage";
	case 0x130a: return "Battery low voltage";
	case 0x130b: return "Battery high voltage";
	case 0x130c: return "Charge alternator failure";
	case 0x130d: return "Fail to start";
	case 0x130e: return "Fail to stop";
	case 0x130f: return "Generator fail to close";
	case 0x1310: return "Mains fail to close";
	case 0x1311: return "Oil pressure sender fault";
	case 0x1312: return "Loss of magnetic pick up";
	case 0x1313: return "Magnetic pick up open circuit";
	case 0x1314: return "Generator high current";
	case 0x1315: return "Calibration lost";
	case 0x1316: return "Low fuel level";
	case 0x1317: return "CAN ECU Warning";
	case 0x1318: return "CAN ECU Shutdown";
	case 0x1319: return "CAN ECU Data fail";
	case 0x131a: return "Low oil level switch";
	case 0x131b: return "High temperature switch";
	case 0x131c: return "Low fuel level switch";
	case 0x131d: return "Expansion unit watchdog alarm";
	case 0x131e: return "kW overload alarm";
	case 0x131f: return "Negative phase sequence current alarm";
	case 0x1320: return "Earth fault trip alarm";
	case 0x1321: return "Generator phase rotation alarm";
	case 0x1322: return "Auto Voltage Sense Fail";
	case 0x1323: return "Maintenance alarm";
	case 0x1324: return "Loading frequency alarm";
	case 0x1325: return "Loading voltage alarm";
	case 0x1326: return "Fuel usage running";
	case 0x1327: return "Fuel usage stopped";
	case 0x1328: return "Protections disabled";
	case 0x1329: return "Protections blocked";
	case 0x132a: return "Generator breaker failed to open";
	case 0x132b: return "Mains breaker failed to open";
	case 0x132c: return "Bus breaker failed to close";
	case 0x132d: return "Bus breaker failed to open";
	case 0x132e: return "Generator reverse power alarm";
	case 0x132f: return "Short circuit alarm";
	case 0x1330: return "Air flap closed alarm";
	case 0x1331: return "Failure to sync";
	case 0x1332: return "Bus live";
	case 0x1333: return "Bus not live";
	case 0x1334: return "Bus phase rotation";
	case 0x1335: return "Priority selection error";
	case 0x1336: return "MSC data error";
	case 0x1337: return "MSC ID error";
	case 0x1338: return "Bus low voltage";
	case 0x1339: return "Bus high voltage";
	case 0x133a: return "Bus low frequency";
	case 0x133b: return "Bus high frequency";
	case 0x133c: return "MSC failure";
	case 0x133d: return "MSC too few sets";
	case 0x133e: return "MSC alarms inhibited";
	case 0x133f: return "MSC old version units on the bus";
	case 0x1340: return "Mains reverse power alarm/mains export alarm";
	case 0x1341: return "Minimum sets not reached";
	case 0x1342: return "Insufficient capacity";
	case 0x1343: return "Out of sync";
	case 0x1344: return "Alternative aux mains fail";
	case 0x1345: return "Loss of excitation";
	case 0x1346: return "Mains ROCOF";
	case 0x1347: return "Mains vector shift";
	case 0x1348: return "Mains decoupling low frequency stage 1";
	case 0x1349: return "Mains decoupling high frequency stage 1";
	case 0x134a: return "Mains decoupling low voltage stage 1";
	case 0x134b: return "Mains decoupling high voltage stage 1";
	case 0x134c: return "Mains decoupling combined alarm";
	case 0x134d: return "Inlet Temperature";
	case 0x134e: return "Mains phase rotation alarm identifier";
	case 0x134f: return "AVR Max Trim Limit alarm";
	case 0x1350: return "High coolant temperature electrical trip alarm";
	case 0x1351: return "Temperature sender open circuit alarm";
	case 0x1352: return "Out of sync Bus";
	case 0x1353: return "Out of sync Mains";
	case 0x1354: return "Bus 1 Live";
	case 0x1355: return "Bus 1 Phase Rotation";
	case 0x1356: return "Bus 2 Live";
	case 0x1357: return "Bus 2 Phase Rotation";
	case 0x1359: return "ECU Protect";
	case 0x135a: return "ECU Malfunction";
	case 0x135b: return "Indication";
	case 0x135e: return "HEST Active";
	case 0x135f: return "DPTC Filter";
	case 0x1360: return "Water In Fuel";
	case 0x1361: return "ECU Heater";
	case 0x1362: return "ECU Cooler";
	case 0x136c: return "High fuel level";
	case 0x136e: return "Module Communication Fail (8661)";
	case 0x136f: return "Bus Module Warning (8661)";
	case 0x1370: return "Bus Module Trip (8661)";
	case 0x1371: return "Mains Module Warning (8661)";
	case 0x1372: return "Mains Module Trip (8661)";
	case 0x1373: return "Load Live (8661)";
	case 0x1374: return "Load Not Live (8661)";
	case 0x1375: return "Load Phase Rotation (8661)";
	case 0x1376: return "DEF Level Low";
	case 0x1377: return "SCR Inducement";
	case 0x1378: return "Heater Sensor Failure Alarm";
	case 0x1379: return "Mains Over Zero Sequence Volts Alarm";
	case 0x137a: return "Mains Under Positive Sequence Volts Alarm";
	case 0x137b: return "Mains Over Negative Sequence Volts Alarm";
	case 0x137c: return "Mains Asymmetry High Alarm";
	case 0x137d: return "Bus Over Zero Sequence Volts Alarm";
	case 0x137e: return "Bus Under Positive Sequence Volts Alarm";
	case 0x137f: return "Bus Over Negative Sequence Volts Alarm";
	case 0x1380: return "Bus Asymmetry High Alarm";
	case 0x1381: return "E-Trip Stop Inhibited";
	case 0x1382: return "Fuel Tank Bund Level High";
	case 0x1383: return "MSC Link 1 Data Error";
	case 0x1384: return "MSC Link 2 Data Error";
	case 0x1385: return "Bus 2 Low Voltage";
	case 0x1386: return "Bus 2 High Voltage";
	case 0x1387: return "Bus 2 Low Frequency";
	case 0x1388: return "Bus 2 High Frequency";
	case 0x1389: return "MSC Link 1 Failure";
	case 0x138a: return "MSC Link 2 Failure";
	case 0x138b: return "MSC Link 1 Too Few Sets";
	case 0x138c: return "MSC Link 2 Too Few Sets";
	case 0x138d: return "MSC Link 1 and 2 Failure";
	case 0x138e: return "Electrical Trip from 8660";
	case 0x138f: return "AVR CAN DM1 Red Stop Lamp Fault";
	case 0x1390: return "Gen Over Zero Sequence Volts Alarm";
	case 0x1391: return "Gen Under Positive Sequence Volts Alarm";
	case 0x1392: return "Gen Over Negative Sequence Volts Alarm";
	case 0x1393: return "Gen Asymmetry High Alarm";
	case 0x1394: return "Mains decoupling low frequency stage 2";
	case 0x1395: return "Mains decoupling high frequency stage 2";
	case 0x1396: return "Mains decoupling low voltage stage 2";
	case 0x1397: return "Mains decoupling high voltage stage 2";
	case 0x1398: return "Fault Ride Through event";
	case 0x1399: return "AVR Data Fail";
	case 0x139a: return "AVR Red Lamp";

	/* DSE error codes 0x1400 to 0x14FF: DSE 7450 */
	case 0x1400: return "Emergency stop";
	case 0x1401: return "Low oil pressure";
	case 0x1402: return "High coolant temperature";
	case 0x1403: return "Low coolant temperature";
	case 0x1404: return "Under speed";
	case 0x1405: return "Over speed";
	case 0x1406: return "Generator Under frequency";
	case 0x1407: return "Generator Over frequency";
	case 0x1408: return "Generator low voltage";
	case 0x1409: return "Generator high voltage";
	case 0x140a: return "Battery low voltage";
	case 0x140b: return "Battery high voltage";
	case 0x140c: return "Charge alternator failure";
	case 0x140d: return "Fail to start";
	case 0x140e: return "Fail to stop";
	case 0x140f: return "Generator fail to close";
	case 0x1410: return "Mains fail to close";
	case 0x1411: return "Oil pressure sender fault";
	case 0x1412: return "Loss of magnetic pick up";
	case 0x1413: return "Magnetic pick up open circuit";
	case 0x1414: return "Generator high current";
	case 0x1415: return "Calibration lost";
	case 0x1416: return "Low fuel level";
	case 0x1417: return "CAN ECU Warning";
	case 0x1418: return "CAN ECU Shutdown";
	case 0x1419: return "CAN ECU Data fail";
	case 0x141a: return "Low oil level switch";
	case 0x141b: return "High temperature switch";
	case 0x141c: return "Low fuel level switch";
	case 0x141d: return "Expansion unit watchdog alarm";
	case 0x141e: return "kW overload alarm";
	case 0x141f: return "Negative phase sequence current alarm";
	case 0x1420: return "Earth fault trip alarm";
	case 0x1421: return "Generator phase rotation alarm";
	case 0x1422: return "Auto Voltage Sense Fail";
	case 0x1423: return "Maintenance alarm";
	case 0x1424: return "Loading frequency alarm";
	case 0x1425: return "Loading voltage alarm";
	case 0x1426: return "Fuel usage running";
	case 0x1427: return "Fuel usage stopped";
	case 0x1428: return "Protections disabled";
	case 0x1429: return "Protections blocked";
	case 0x142a: return "Generator breaker failed to open";
	case 0x142b: return "Mains breaker failed to open";
	case 0x142c: return "Bus breaker failed to close";
	case 0x142d: return "Bus breaker failed to open";
	case 0x142e: return "Generator reverse power alarm";
	case 0x142f: return "Short circuit alarm";
	case 0x1430: return "Air flap closed alarm";
	case 0x1431: return "Failure to sync";
	case 0x1432: return "Bus live";
	case 0x1433: return "Bus not live";
	case 0x1434: return "Bus phase rotation";
	case 0x1435: return "Priority selection error";
	case 0x1436: return "MSC data error";
	case 0x1437: return "MSC ID error";
	case 0x1438: return "Bus low voltage";
	case 0x1439: return "Bus high voltage";
	case 0x143a: return "Bus low frequency";
	case 0x143b: return "Bus high frequency";
	case 0x143c: return "MSC failure";
	case 0x143d: return "MSC too few sets";
	case 0x143e: return "MSC alarms inhibited";
	case 0x143f: return "MSC old version units on the bus";
	case 0x1440: return "Mains reverse power alarm/mains export alarm";
	case 0x1441: return "Minimum sets not reached";
	case 0x1442: return "Insufficient capacity";
	case 0x1443: return "Out of sync";
	case 0x1444: return "Alternative aux mains fail";
	case 0x1445: return "Loss of excitation";
	case 0x1446: return "Mains ROCOF";
	case 0x1447: return "Mains vector shift";
	case 0x1448: return "Mains decoupling low frequency";
	case 0x1449: return "Mains decoupling high frequency";
	case 0x144a: return "Mains decoupling low voltage";
	case 0x144b: return "Mains decoupling high voltage";
	case 0x144c: return "Mains decoupling combined alarm";
	case 0x144d: return "Charge Air Temperature";
	case 0x144e: return "Mains phase rotation alarm identifier";
	case 0x144f: return "AVR Max Trim Limit alarm";
	case 0x1450: return "High coolant temperature electrical trip alarm";
	case 0x1451: return "Temperature sender open circuit alarm";
	case 0x1459: return "ECU Protect";
	case 0x145a: return "ECU Malfunction";
	case 0x145b: return "Indication";
	case 0x145c: return "ECU Red";
	case 0x145d: return "ECU Amber";
	case 0x145e: return "Electrical Trip";
	case 0x145f: return "Aftertreatment Exhaust";
	case 0x1460: return "Water In Fuel";
	case 0x1461: return "ECU Heater";
	case 0x1462: return "ECU Cooler";
	case 0x1463: return "DC Total Watts Overload";
	case 0x1464: return "High Plant Battery Temperature";
	case 0x1465: return "Low Plant Battery Temperature";
	case 0x1466: return "Low Plant Battery Voltage";
	case 0x1467: return "High Plant Battery Voltage";
	case 0x1468: return "Plant Battery Depth Of Discharge";
	case 0x1469: return "DC Battery Over Current";
	case 0x146a: return "DC Load Over Current";
	case 0x146b: return "High Total DC Current";

	/* DSE error codes 0x1500 to 0x15FF: DSE 71xx/66xx/60xx/L40x/4xxx/45xx MKII family */
	case 0x1500: return "Emergency stop";
	case 0x1501: return "Low oil pressure";
	case 0x1502: return "High coolant temperature";
	case 0x1503: return "Low coolant temperature";
	case 0x1504: return "Under speed";
	case 0x1505: return "Over speed";
	case 0x1506: return "Generator Under frequency";
	case 0x1507: return "Generator Over frequency";
	case 0x1508: return "Generator low voltage";
	case 0x1509: return "Generator high voltage";
	case 0x150a: return "Battery low voltage";
	case 0x150b: return "Battery high voltage";
	case 0x150c: return "Charge alternator failure";
	case 0x150d: return "Fail to start";
	case 0x150e: return "Fail to stop";
	case 0x150f: return "Generator fail to close";
	case 0x1510: return "Mains fail to close";
	case 0x1511: return "Oil pressure sender fault";
	case 0x1512: return "Loss of Mag Pickup signal";
	case 0x1513: return "Magnetic pick up open circuit";
	case 0x1514: return "Generator high current";
	case 0x1515: return "Calibration lost";
	case 0x1517: return "CAN ECU Warning";
	case 0x1518: return "CAN ECU Shutdown";
	case 0x1519: return "CAN ECU Data fail";
	case 0x151a: return "Low oil level switch";
	case 0x151b: return "High temperature switch";
	case 0x151c: return "Low fuel level switch";
	case 0x151d: return "Expansion unit watchdog alarm";
	case 0x151e: return "kW overload alarm";
	case 0x151f: return "Negative phase sequence alarm";
	case 0x1520: return "Earth fault trip";
	case 0x1521: return "Generator phase rotation alarm";
	case 0x1522: return "Auto Voltage Sense fail";
	case 0x1524: return "Temperature sensor open circuit";
	case 0x1525: return "Low fuel level";
	case 0x1526: return "High fuel level";
	case 0x1527: return "Water in Fuel";
	case 0x1528: return "DEF Level Low";
	case 0x1529: return "SCR Inducement";
	case 0x152a: return "Hest Active";
	case 0x152b: return "DPTC Filter";

	default: return "Unknown error";
	}
}

QString Hatz::getDescription(QString errorId)
{
	const auto dashIdx = errorId.indexOf("-");
	if (dashIdx < 1)
		return tr("Unknown error: ") + errorId;

	errorId.remove(0, dashIdx + 1);

	const auto parts = errorId.split('.');
	if (parts.size() != 2)
		return tr("Unknown error: ") + errorId;

	bool ok[2];
	const auto spn = parts[0].toUInt(&ok[0]);
	const auto fmi = parts[1].toUInt(&ok[1]);
	if (!ok[0] || !ok[1])
		return tr("Unknown error: ") + errorId;

	auto result = "#" + errorId + " ";

	/* These SPN+FMI fault codes and their descriptions are taken
	 * from the Hatz 05666801.1823/Fehlerlist V529 document */
	switch (SpnAndFmiToNumber(spn, fmi)) {
	case SpnAndFmiToNumber( 100,  7): result += tr("Oil pressure"); break;
	case SpnAndFmiToNumber( 110,  7): result += tr("Cylinder head overtemperature"); break;
	case SpnAndFmiToNumber( 167,  7): result += tr("Charge control"); break;
	case SpnAndFmiToNumber( 190,  1): result += tr("Speed higher than expected"); break;
	case SpnAndFmiToNumber( 190, 15): result += tr("Overspeed"); break;
	case SpnAndFmiToNumber( 175, 15): /* Fall-through */
	case SpnAndFmiToNumber( 175, 16): result += tr("Oiltemperature higher than expected"); break;
	case SpnAndFmiToNumber( 175,  3): result += tr("Oiltemperature open circuit / short to power"); break;
	case SpnAndFmiToNumber( 175,  4): result += tr("Oiltemperature short to ground"); break;
	case SpnAndFmiToNumber(  91,  3): result += tr("Analog setpoint high / short to power"); break;
	case SpnAndFmiToNumber(  91,  4): result += tr("Analog setpoint low / short to ground"); break;
	case SpnAndFmiToNumber( 695,  9): result += tr("TSC1 message receive timeout"); break;
	case SpnAndFmiToNumber( 986, 12): result += tr("CM1 message receive timeout"); break;
	case SpnAndFmiToNumber( 168, 15): result += tr("Battery voltage high"); break;
	case SpnAndFmiToNumber( 168, 17): result += tr("Battery voltage low"); break;
	case SpnAndFmiToNumber( 636,  2): result += tr("Speed signal distorted"); break;
	case SpnAndFmiToNumber(1079,  3): result += tr("Internal 5V sensor supply high"); break;
	case SpnAndFmiToNumber(1079,  4): result += tr("Internal 5V sensor supply low"); break;
	case SpnAndFmiToNumber( 108,  0): result += tr("Barometric pressure high"); break;
	case SpnAndFmiToNumber( 108,  1): result += tr("Barometric pressure low"); break;
	case SpnAndFmiToNumber(1347,  3): result += tr("Output fuelpump short to power"); break;
	case SpnAndFmiToNumber(1347,  5): result += tr("Output fuelpump short to ground"); break;
	case SpnAndFmiToNumber( 676,  3): result += tr("Output glow plug short to power"); break;
	case SpnAndFmiToNumber( 676,  5): result += tr("Output glow plug short to ground"); break;
	case SpnAndFmiToNumber( 651,  5): result += tr("Injector open circuit/low side short to ground"); break;
	case SpnAndFmiToNumber( 651,  6): result += tr("Injector coil internal short circuit"); break;
	case SpnAndFmiToNumber( 651,  3): result += tr("Injector low side short to power"); break;
	case SpnAndFmiToNumber( 915, 14): result += tr("Service hours expired"); break;
	case SpnAndFmiToNumber( 629, 31): /* Fall-through */
	case SpnAndFmiToNumber( 630, 12): /* Fall-through */
	case SpnAndFmiToNumber( 628, 13): /* Fall-through */
	case SpnAndFmiToNumber(1634,  2): result += tr("Processor failure"); break;
	}

	return result;
}

QString CRE::getDescription(QString errorId)
{
	auto parts = errorId.split("-");
	if (parts.length() < 1)
		return tr("Unknown error: ") + errorId;

	bool ok;
	auto errorNumber = parts[1].toUInt(&ok);
	if (!ok)
		return tr("Unknown error: ") + errorId;

	switch (errorNumber) {
		case 0: return "Fault";
		case 1: return "Alarm";

		default: return "Unknown error";
	}
}

QString DEIF::getDescription(QString errorId)
{
	auto parts = errorId.split("-");
	if (parts.length() < 1)
		return tr("Unknown error: ") + errorId;

	bool ok;
	auto errorNumber = parts[1].toUInt(&ok);
	if (!ok)
		return tr("Unknown error: ") + errorId;

	switch (errorNumber) {
		case 0: return "G -P> 1";
		case 1: return "G -P> 2";
		case 2: return "G -P> 3";
		case 3: return "G I> 1";
		case 4: return "G I> 2";
		case 5: return "G I> 3";
		case 6: return "G I> 4";
		case 7: return "G I> inv.";
		case 8: return "G Iv>";
		case 9: return "G I>> 1";
		case 10: return "G I>> 2";
		case 11: return "G U> 1";
		case 12: return "G U> 2";
		case 13: return "G U< 1";
		case 14: return "G U< 2";
		case 15: return "G U< 3";
		case 16: return "G f> 1";
		case 17: return "G f> 2";
		case 18: return "G f> 3";
		case 19: return "G f< 1";
		case 20: return "G f< 2";
		case 21: return "G f< 3";
		case 22: return "BB U> 1";
		case 23: return "BB U> 2";
		case 24: return "BB U> 3";
		case 25: return "BB U< 1";
		case 26: return "BB U< 2";
		case 27: return "BB U< 3";
		case 28: return "BB U< 4";
		case 29: return "BB f> 1";
		case 30: return "BB f> 2";
		case 31: return "BB f> 3";
		case 32: return "BB f< 1";
		case 33: return "BB f< 2";
		case 34: return "BB f< 3";
		case 35: return "BB f< 4";
		case 36: return "df/dt (ROCOF)";
		case 37: return "Vector jump";
		case 38: return "BB pos. seq. volt. low";
		case 39: return "G P> 1";
		case 40: return "G P> 2";
		case 41: return "G P> 3";
		case 42: return "G P> 4";
		case 43: return "G P> 5";
		case 44: return "Unbalance curr. 1";
		case 45: return "Unbalance volt.";
		case 46: return "G -Q>";
		case 47: return "G Q>";
		case 48: return "Gen. neg. seq. I";
		case 49: return "Generator neg. seq. U";
		case 50: return "Gen. zero seq. I";
		case 51: return "Zero seq. U";
		case 52: return "Directional overcurrent 1";
		case 53: return "Directional overcurrent 2";
		case 54: return "BB unbalance U";
		case 70: return "U and Q< 1";
		case 71: return "U and Q< 2";
		case 72: return "GB ext. trip";
		case 73: return "MB ext. trip";
		case 78: return "G P dep. Q<";
		case 79: return "G P dep. Q>";
		case 80: return "Synchronising window";
		case 81: return "Synchronising failure GB";
		case 82: return "Synchronising failure MB";
		case 83: return "Phase seq. failure A";
		case 84: return "GB open failure";
		case 85: return "GB close failure";
		case 86: return "GB pos. failure";
		case 87: return "MB open failure";
		case 88: return "MB close failure";
		case 89: return "MB pos. failure";
		case 90: return "Close before excitation failure";
		case 91: return "Phase seq. failure B";
		case 96: return "GOVERNOR regulation fail.";
		case 97: return "Deload error";
		case 98: return "AVR regulation fail.";
		case 112: return "Digital alarm input 39";
		case 113: return "Digital alarm input 40";
		case 114: return "Digital alarm input 41";
		case 115: return "Digital alarm input 42";
		case 116: return "Digital alarm input 43";
		case 117: return "Digital alarm input 44";
		case 118: return "Digital alarm input 45";
		case 119: return "Digital alarm input 46";
		case 120: return "Digital alarm input 47";
		case 121: return "Digital alarm input 48";
		case 122: return "Digital alarm input 49";
		case 123: return "Digital alarm input 50";
		case 172: return "Digital alarm input 4 (Emergency stop)";
		case 183: return "M-Logic alarm 1";
		case 184: return "M-Logic alarm 2";
		case 185: return "M-Logic alarm 3";
		case 186: return "M-Logic alarm 4";
		case 187: return "M-Logic alarm 5";
		case 188: return "M-Logic extended alarm 1";
		case 189: return "M-Logic extended alarm 2";
		case 190: return "M-Logic extended alarm 3";
		case 191: return "M-Logic extended alarm 4";
		case 208: return "Multi input terminal 20.1";
		case 209: return "Multi input terminal 20.2";
		case 210: return "W. fail. 20";
		case 211: return "Multi input terminal 21.1";
		case 212: return "Multi input terminal 21.2";
		case 213: return "W. fail. 21";
		case 214: return "Multi input terminal 22.1";
		case 215: return "Multi input terminal 22.2";
		case 216: return "W. fail. 22";
		case 217: return "Overspeed 1";
		case 218: return "Overspeed 2";
		case 219: return "Crank failure";
		case 220: return "Running feedback failure";
		case 221: return "MPU wire failure";
		case 222: return "Hz/V failure";
		case 223: return "Start failure";
		case 224: return "Stop failure";
		case 225: return "U< aux. term. 1";
		case 226: return "U> aux. term. 1";
		case 229: return "Underspeed 1";
		case 230: return "Multi input terminal 23.1";
		case 231: return "Multi input terminal 23.2";
		case 232: return "W. fail. 23";
		case 240: return "Service timer 1";
		case 241: return "Service timer 2";
		case 242: return "Stop coil wire break";
		case 244: return "Engine heater 1";
		case 245: return "Battery test";
		case 246: return "Battery asymmetry 1";
		case 247: return "Battery asymmetry 2";
		case 248: return "Max. ventilation 1";
		case 249: return "Max. ventilation 2";
		case 250: return "Block switchboard error";
		case 251: return "Stop switchboard error";
		case 252: return "Unit not in auto";
		case 253: return "Fuel pump logic";
		case 912: return "Fail class: Block start + breaker on";
		case 913: return "Fail class: Warning";
		case 914: return "Fail class: Trip GB";
		case 915: return "Fail class: Trip GB + stop with cooling down";
		case 916: return "Fail class: Trip GB + stop without cooling down";
		case 917: return "Fail class: Trip MB";
		case 918: return "Fail class: Safety stop";
		case 919: return "Fail class: If MB present Trip MB else Trip GB";
		case 920: return "Fail class: Controlled stop (deload + cooldown)";

		default: return "Unknown error";
	}
}

QString Cummins::getDescription(QString errorId)
{
	const auto dashIdx = errorId.indexOf("-");
	if (dashIdx < 1)
		return tr("Unknown error: ") + errorId;

	errorId.remove(0, dashIdx + 1);

	const auto parts = errorId.split('.');
	if (parts.size() != 2)
		return tr("Unknown error: ") + errorId;

	bool ok[2];
	const auto spn = parts[0].toUInt(&ok[0]);
	const auto fmi = parts[1].toUInt(&ok[1]);
	if (!ok[0] || !ok[1])
		return tr("Unknown error: ") + errorId;

	/* These SPN+FMI fault codes and their descriptions are taken
	 * from the Cummins Customer Engineering Bulletin 00793 issued 7 April 2020 */
	switch (SpnAndFmiToNumber(spn, fmi)) {
		case SpnAndFmiToNumber(175,  0): return ("#1 Oil temp exceeds limit"); break;
		case SpnAndFmiToNumber(1675,14): return ("#4 No start after 20 seconds"); break;
		case SpnAndFmiToNumber(98,   1): return ("#6 Oil level below minimum"); break;
		case SpnAndFmiToNumber(2444, 0): return ("#12 High output voltage"); break;
		case SpnAndFmiToNumber(2444, 1): return ("#13 Low output voltage"); break;
		case SpnAndFmiToNumber(2436, 0): return ("#14 High output frequency"); break;
		case SpnAndFmiToNumber(2436, 1): return ("#15 Low output frequency"); break;
		case SpnAndFmiToNumber(51,   2): return ("#19 Throttle actuator fault"); break;
		case SpnAndFmiToNumber(8890, 0): return ("#25 High internal voltage"); break;
		case SpnAndFmiToNumber(8890, 1): return ("#26 Low internal voltage"); break;
		case SpnAndFmiToNumber(589,  2): return ("#27 System frequency unknown"); break;
		case SpnAndFmiToNumber(168,  0): return ("#29 High battery voltage"); break;
		case SpnAndFmiToNumber(190,  0): return ("#31 High engine speed"); break;
		case SpnAndFmiToNumber(1675, 8): return ("#32 Low cranking speed"); break;
		case SpnAndFmiToNumber(9206, 0): return ("#34 Inverter temp too high"); break;
		case SpnAndFmiToNumber(190, 11): return ("#36 Engine stopped, unknown"); break;
		case SpnAndFmiToNumber(2448, 0): return ("#38 Output over generator limit"); break;
		case SpnAndFmiToNumber(190,  2): return ("#45 Engine speed unknown"); break;
		case SpnAndFmiToNumber(51,   0): return ("#49 Throttle limit exceeded"); break;
		case SpnAndFmiToNumber(1440,14): return ("#52 Injector/lift pump faulty"); break;
		case SpnAndFmiToNumber(175,  2): return ("#53 Oil temp unknown"); break;
		case SpnAndFmiToNumber(105,  2): return ("#54 Manifold Air Temp unknown"); break;
		case SpnAndFmiToNumber(102,  2): return ("#56 MAP sensor unknown"); break;
		case SpnAndFmiToNumber(2448, 6): return ("#67 AC output short to ground"); break;
		case SpnAndFmiToNumber(3353, 4): return ("#81 System short detected"); break;
		case SpnAndFmiToNumber(6814, 2): return ("#82 Vent pressure unknown"); break;
		case SpnAndFmiToNumber(234, 13): return ("Controller error"); break;
		default:
			return QString("Unknown SPN/FMI (%1/%2)").arg(spn).arg(fmi);
	}
}
