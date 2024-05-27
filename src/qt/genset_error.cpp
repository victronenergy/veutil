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

QString GensetError::getDescription(QString errorId, int nrOfPhases)
{
	if (errorId == "")
		return tr("No error");
	if (errorId.startsWith("fischerpanda:", Qt::CaseInsensitive))
		return FischerPanda::getDescription(errorId, nrOfPhases);
	if (errorId.startsWith("dse:", Qt::CaseInsensitive))
		return DSE::getDescription(errorId);
	return tr("Unknown error: %1").arg(errorId);
}

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
		case  3 + 0x00: result += nrOfPhases > 1 ? tr("AC current L1 too low") : tr("AC current too low"); break;
		case  3 + 0x40: result += nrOfPhases > 1 ? tr("AC current L1 too high") : tr("AC current too high"); break;
		case  4 + 0x00: result += nrOfPhases > 1 ? tr("AC power L1 too low") : tr("AC power too low"); break;
		case  4 + 0x40: result += nrOfPhases > 1 ? tr("AC power L1 too high") : tr("AC power too high"); break;
		case  5 + 0x00: result += tr("Emergency stop"); break;
		case  6 + 0x00: result += tr("Servo current too low"); break;
		case  6 + 0x40: result += tr("Servo current too high"); break;
		case  7 + 0x00: result += tr("Oil pressure too low"); break;
		case  7 + 0x40: result += tr("Oil pressure too high"); break;
		case  8 + 0x00: result += tr("Engine temperature too low"); break;
		case  8 + 0x40: result += tr("Engine temperature too high"); break;
		case  9 + 0x00: result += tr("Winding temperature too low"); break;
		case  9 + 0x40: result += tr("Winding temperature too high"); break;
		case 10 + 0x00: result += tr("Exhaust temperature too low"); break;
		case 10 + 0x40: result += tr("Exhaust temperature too high"); break;
		case 13 + 0x00: result += tr("Starter current too low"); break;
		case 13 + 0x40: result += tr("Starter current too high"); break;
		case 14 + 0x00: result += tr("Glow current too low"); break;
		case 14 + 0x40: result += tr("Glow current too high"); break;
		case 15 + 0x00: result += tr("Glow current too low"); break;
		case 15 + 0x40: result += tr("Glow current too high"); break;
		case 16 + 0x00: result += tr("Fuel holding magnet current too low"); break;
		case 16 + 0x40: result += tr("Fuel holding magnet current too high"); break;
		case 17 + 0x00: result += tr("Stop solenoid hold coil current too low"); break;
		case 17 + 0x40: result += tr("Stop solenoid hold coil current too high"); break;
		case 18 + 0x00: result += tr("Stop solenoid pull coil current too low "); break;
		case 18 + 0x40: result += tr("Stop solenoid pull coil current too high"); break;
		case 19 + 0x00: result += tr("Optional DC out current too low"); break;
		case 19 + 0x40: result += tr("Optional DC out current too high"); break;
		case 20 + 0x00: result += tr("5V output voltage too low"); break;
		case 20 + 0x40: result += tr("5V output current too high"); break;
		case 21 + 0x00: result += tr("Boost output current too low"); break;
		case 21 + 0x40: result += tr("Boost output current too high"); break;
		case 22 + 0x40: result += tr("Panel supply current too high"); break;
		case 25 + 0x00: result += tr("Starter battery voltage too low"); break;
		case 25 + 0x40: result += tr("Starter battery voltage too high"); break;
		case 26 + 0x00: result += tr("Startup aborted (rotation too low)"); break;
		case 26 + 0x40: result += tr("Startup aborted (rotation too high)"); break;
		case 28 + 0x00: result += tr("Rotation too low"); break;
		case 28 + 0x40: result += tr("Rotation too high"); break;
		case 29 + 0x00: result += tr("Power contactor current too low"); break;
		case 29 + 0x40: result += tr("Power contactor current too high"); break;
		case 30 + 0x00: result += tr("AC voltage L2 too low"); break;
		case 30 + 0x40: result += tr("AC voltage L2 too high"); break;
		case 31 + 0x00: result += tr("AC frequency L2 too low"); break;
		case 31 + 0x40: result += tr("AC frequency L2 too high"); break;
		case 32 + 0x00: result += tr("AC current L2 too low"); break;
		case 32 + 0x40: result += tr("AC current L2 too high"); break;
		case 33 + 0x00: result += tr("AC power L2 too low"); break;
		case 33 + 0x40: result += tr("AC power L2 too high"); break;
		case 34 + 0x00: result += tr("AC voltage L3 too low"); break;
		case 34 + 0x40: result += tr("AC voltage L3 too high"); break;
		case 35 + 0x00: result += tr("AC frequency L3 too low"); break;
		case 35 + 0x40: result += tr("AC frequency L3 too high"); break;
		case 36 + 0x00: result += tr("AC current L3 too low"); break;
		case 36 + 0x40: result += tr("AC current L3 too high"); break;
		case 37 + 0x00: result += tr("AC power L3 too low"); break;
		case 37 + 0x40: result += tr("AC power L3 too high"); break;
		case 62 + 0x00: result += tr("Fuel temperature too low"); break;
		case 62 + 0x40: result += tr("Fuel temperature too high"); break;
		case 63 + 0x00: result += tr("Fuel level too low"); break;
		case 63 + 0x40: result += tr("Fuel level too high"); break;

		/* Errors (>= 128) with one code */
		case 130: result += tr("Lost control unit"); break;
		case 131: result += tr("Lost panel"); break;
		case 132: result += tr("Service needed"); break;
		case 133: result += tr("Lost 3-phase module"); break;
		case 134: result += tr("Lost AGT module"); break;
		case 135: result += tr("Synchronization failure"); break;
		case 137: result += tr("Intake airfilter"); break;
		case 139: result += tr("Lost sync. module"); break;
		case 140: result += tr("Load-balance failed"); break;
		case 141: result += tr("Sync-mode deactivated"); break;
		case 142: result += tr("Engine controller"); break;
		case 148: result += tr("Rotating field wrong"); break;
		case 149: result += tr("Fuel level sensor lost"); break;

		/* Error codes for iControl only */
		case 150: result += tr("Init failed"); break;
		case 151: result += tr("Watchdog"); break;
		case 152: result += tr("Out: winding"); break;
		case 153: result += tr("Out: exhaust"); break;
		case 154: result += tr("Out: Cyl. head"); break;
		case 155: result += tr("Inverter over temperature"); break;
		case 156: result += tr("Inverter overload"); break;
		case 157: result += tr("Inverter communication lost"); break;
		case 158: result += tr("Inverter sync failed"); break;
		case 159: result += tr("CAN communication lost"); break;
		case 160: result += tr("L1 overload"); break;
		case 161: result += tr("L2 overload"); break;
		case 162: result += tr("L3 overload"); break;
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

	auto result = "#" + QString::number(errorNumber) + " ";

	switch (errorNumber) {
	/* DSE error codes 0x1000 to 0x10FF: DSE old alarm system */
	case 0x1000: result += "Emergency stop"; break;
	case 0x1001: result += "Low oil pressure"; break;
	case 0x1002: result += "High coolant temperature"; break;
	case 0x1003: result += "High oil temperature"; break;
	case 0x1004: result += "Under speed"; break;
	case 0x1005: result += "Over speed"; break;
	case 0x1006: result += "Fail to start"; break;
	case 0x1007: result += "Fail to come to rest"; break;
	case 0x1008: result += "Loss of speed sensing"; break;
	case 0x1009: result += "Generator low voltage"; break;
	case 0x100a: result += "Generator high voltage"; break;
	case 0x100b: result += "Generator low frequency"; break;
	case 0x100c: result += "Generator high frequency"; break;
	case 0x100d: result += "Generator high current"; break;
	case 0x100e: result += "Generator earth fault"; break;
	case 0x100f: result += "Generator reverse power"; break;
	case 0x1010: result += "Air flap"; break;
	case 0x1011: result += "Oil pressure sender fault"; break;
	case 0x1012: result += "Coolant temperature sender fault"; break;
	case 0x1013: result += "Oil temperature sender fault"; break;
	case 0x1014: result += "Fuel level sender fault"; break;
	case 0x1015: result += "Magnetic pickup fault"; break;
	case 0x1016: result += "Loss of AC speed signal"; break;
	case 0x1017: result += "Charge alternator failure"; break;
	case 0x1018: result += "Low battery voltage"; break;
	case 0x1019: result += "High battery voltage"; break;
	case 0x101a: result += "Low fuel level"; break;
	case 0x101b: result += "High fuel level"; break;
	case 0x101c: result += "Generator failed to close"; break;
	case 0x101d: result += "Mains failed to close"; break;
	case 0x101e: result += "Generator failed to open"; break;
	case 0x101f: result += "Mains failed to open"; break;
	case 0x1020: result += "Mains low voltage"; break;
	case 0x1021: result += "Mains high voltage"; break;
	case 0x1022: result += "Bus failed to close"; break;
	case 0x1023: result += "Bus failed to open"; break;
	case 0x1024: result += "Mains low frequency"; break;
	case 0x1025: result += "Mains high frequency"; break;
	case 0x1026: result += "Mains failed"; break;
	case 0x1027: result += "Mains phase rotation wrong"; break;
	case 0x1028: result += "Generator phase rotation wrong"; break;
	case 0x1029: result += "Maintenance due"; break;
	case 0x102a: result += "Clock not set"; break;
	case 0x102b: result += "Local LCD configuration lost"; break;
	case 0x102c: result += "Local telemetry configuration lost"; break;
	case 0x102d: result += "Control unit not calibrated"; break;
	case 0x102e: result += "Modem power fault"; break;
	case 0x102f: result += "Generator short circuit"; break;
	case 0x1030: result += "Failure to synchronise"; break;
	case 0x1031: result += "Bus live"; break;
	case 0x1032: result += "Scheduled run"; break;
	case 0x1033: result += "Bus phase rotation wrong"; break;
	case 0x1034: result += "Priority selection error"; break;
	case 0x1035: result += "Multiset communications (MSC) data error"; break;
	case 0x1036: result += "Multiset communications (MSC) ID error"; break;
	case 0x1037: result += "Multiset communications (MSC) failure"; break;
	case 0x1038: result += "Multiset communications (MSC) too few sets"; break;
	case 0x1039: result += "Multiset communications (MSC) alarms inhibited"; break;
	case 0x103a: result += "Multiset communications (MSC) old version units"; break;
	case 0x103b: result += "Mains reverse power"; break;
	case 0x103c: result += "Minimum sets not reached"; break;
	case 0x103d: result += "Insufficient capacity available"; break;
	case 0x103e: result += "Expansion input unit not calibrated"; break;
	case 0x103f: result += "Expansion input unit failure"; break;
	case 0x1040: result += "Auxiliary sender 1 low"; break;
	case 0x1041: result += "Auxiliary sender 1 high"; break;
	case 0x1042: result += "Auxiliary sender 1 fault"; break;
	case 0x1043: result += "Auxiliary sender 2 low"; break;
	case 0x1044: result += "Auxiliary sender 2 high"; break;
	case 0x1045: result += "Auxiliary sender 2 fault"; break;
	case 0x1046: result += "Auxiliary sender 3 low"; break;
	case 0x1047: result += "Auxiliary sender 3 high"; break;
	case 0x1048: result += "Auxiliary sender 3 fault"; break;
	case 0x1049: result += "Auxiliary sender 4 low"; break;
	case 0x104a: result += "Auxiliary sender 4 high"; break;
	case 0x104b: result += "Auxiliary sender 4 fault"; break;
	case 0x104c: result += "Engine control unit (ECU) link lost"; break;
	case 0x104d: result += "Engine control unit (ECU) failure"; break;
	case 0x104e: result += "Engine control unit (ECU) error"; break;
	case 0x104f: result += "Low coolant temperature"; break;
	case 0x1050: result += "Out of sync"; break;
	case 0x1051: result += "Low Oil Pressure Switch"; break;
	case 0x1052: result += "Alternative Auxiliary Mains Fail"; break;
	case 0x1053: result += "Loss of excitation"; break;
	case 0x1054: result += "Mains kW Limit"; break;
	case 0x1055: result += "Negative phase sequence"; break;
	case 0x1056: result += "Mains ROCOF"; break;
	case 0x1057: result += "Mains vector shift"; break;
	case 0x1058: result += "Mains G59 low frequency"; break;
	case 0x1059: result += "Mains G59 high frequency"; break;
	case 0x105a: result += "Mains G59 low voltage"; break;
	case 0x105b: result += "Mains G59 high voltage"; break;
	case 0x105c: result += "Mains G59 trip"; break;
	case 0x105d: result += "Generator kW Overload"; break;
	case 0x105e: result += "Engine Inlet Temperature high"; break;
	case 0x105f: result += "Bus 1 live"; break;
	case 0x1060: result += "Bus 1 phase rotation wrong"; break;
	case 0x1061: result += "Bus 2 live"; break;
	case 0x1062: result += "Bus 2 phase rotation wrong"; break;

	/* DSE error codes 0x1100 to 0x11FF: DSE 61xx MKII */
	case 0x1100: result += "Emergency stop"; break;
	case 0x1101: result += "Low oil pressure"; break;
	case 0x1102: result += "High coolant temperature"; break;
	case 0x1103: result += "Low coolant temperature"; break;
	case 0x1104: result += "Under speed"; break;
	case 0x1105: result += "Over speed"; break;
	case 0x1106: result += "Generator Under frequency"; break;
	case 0x1107: result += "Generator Over frequency"; break;
	case 0x1108: result += "Generator low voltage"; break;
	case 0x1109: result += "Generator high voltage"; break;
	case 0x110a: result += "Battery low voltage"; break;
	case 0x110b: result += "Battery high voltage"; break;
	case 0x110c: result += "Charge alternator failure"; break;
	case 0x110d: result += "Fail to start"; break;
	case 0x110e: result += "Fail to stop"; break;
	case 0x110f: result += "Generator fail to close"; break;
	case 0x1110: result += "Mains fail to close"; break;
	case 0x1111: result += "Oil pressure sender fault"; break;
	case 0x1112: result += "Loss of magnetic pick up"; break;
	case 0x1113: result += "Magnetic pick up open circuit"; break;
	case 0x1114: result += "Generator high current"; break;
	case 0x1115: result += "Calibration lost"; break;
	case 0x1116: result += "Low fuel level"; break;
	case 0x1117: result += "CAN ECU Warning"; break;
	case 0x1118: result += "CAN ECU Shutdown"; break;
	case 0x1119: result += "CAN ECU Data fail"; break;
	case 0x111a: result += "Low oil level switch"; break;
	case 0x111b: result += "High temperature switch"; break;
	case 0x111c: result += "Low fuel level switch"; break;
	case 0x111d: result += "Expansion unit watchdog alarm"; break;
	case 0x111e: result += "kW overload alarm"; break;
	case 0x1123: result += "Maintenance alarm"; break;
	case 0x1124: result += "Loading frequency alarm"; break;
	case 0x1125: result += "Loading voltage alarm"; break;
	case 0x112e: result += "ECU protect"; break;
	case 0x112f: result += "ECU Malfunction"; break;
	case 0x1130: result += "ECU Information"; break;
	case 0x1131: result += "ECU Shutdown"; break;
	case 0x1132: result += "ECU Warning"; break;
	case 0x1133: result += "ECU HEST"; break;
	case 0x1135: result += "ECU Water In Fuel"; break;
	case 0x1139: result += "High fuel level"; break;
	case 0x113a: result += "DEF Level Low"; break;
	case 0x113b: result += "SCR Inducement"; break;

	/* DSE error codes 0x1200 to 0x12FF: DSE 72xx/73xx/61xx/74xx MKII family */
	case 0x1200: result += "Emergency stop"; break;
	case 0x1201: result += "Low oil pressure"; break;
	case 0x1202: result += "High coolant temperature"; break;
	case 0x1203: result += "Low coolant temperature"; break;
	case 0x1204: result += "Under speed"; break;
	case 0x1205: result += "Over speed"; break;
	case 0x1206: result += "Generator Under frequency"; break;
	case 0x1207: result += "Generator Over frequency"; break;
	case 0x1208: result += "Generator low voltage"; break;
	case 0x1209: result += "Generator high voltage"; break;
	case 0x120a: result += "Battery low voltage"; break;
	case 0x120b: result += "Battery high voltage"; break;
	case 0x120c: result += "Charge alternator failure"; break;
	case 0x120d: result += "Fail to start"; break;
	case 0x120e: result += "Fail to stop"; break;
	case 0x120f: result += "Generator fail to close"; break;
	case 0x1210: result += "Mains fail to close"; break;
	case 0x1211: result += "Oil pressure sender fault"; break;
	case 0x1212: result += "Loss of magnetic pick up"; break;
	case 0x1213: result += "Magnetic pick up open circuit"; break;
	case 0x1214: result += "Generator high current"; break;
	case 0x1215: result += "Calibration lost"; break;
	case 0x1216: result += "Low fuel level"; break;
	case 0x1217: result += "CAN ECU Warning"; break;
	case 0x1218: result += "CAN ECU Shutdown"; break;
	case 0x1219: result += "CAN ECU Data fail"; break;
	case 0x121a: result += "Low oil level switch"; break;
	case 0x121b: result += "High temperature switch"; break;
	case 0x121c: result += "Low fuel level switch"; break;
	case 0x121d: result += "Expansion unit watchdog alarm"; break;
	case 0x121e: result += "kW overload alarm"; break;
	case 0x121f: result += "Negative phase sequence current alarm"; break;
	case 0x1220: result += "Earth fault trip alarm"; break;
	case 0x1221: result += "Generator phase rotation alarm"; break;
	case 0x1222: result += "Auto Voltage Sense Fail"; break;
	case 0x1223: result += "Maintenance alarm"; break;
	case 0x1224: result += "Loading frequency alarm"; break;
	case 0x1225: result += "Loading voltage alarm"; break;
	case 0x1226: result += "Fuel usage running"; break;
	case 0x1227: result += "Fuel usage stopped"; break;
	case 0x1228: result += "Protections disabled"; break;
	case 0x1229: result += "Protections blocked"; break;
	case 0x122a: result += "Generator Short Circuit"; break;
	case 0x122b: result += "Mains High Current"; break;
	case 0x122c: result += "Mains Earth Fault"; break;
	case 0x122d: result += "Mains Short Circuit"; break;
	case 0x122e: result += "ECU protect"; break;
	case 0x122f: result += "ECU Malfunction"; break;
	case 0x1230: result += "ECU Information"; break;
	case 0x1231: result += "ECU Shutdown"; break;
	case 0x1232: result += "ECU Warning"; break;
	case 0x1233: result += "ECU Electrical Trip"; break;
	case 0x1234: result += "ECU After treatment"; break;
	case 0x1235: result += "ECU Water In Fuel"; break;
	case 0x1236: result += "Generator Reverse Power"; break;
	case 0x1237: result += "Generator Positive VAr"; break;
	case 0x1238: result += "Generator Negative VAr"; break;
	case 0x1239: result += "LCD Heater Low Voltage"; break;
	case 0x123a: result += "LCD Heater High Voltage"; break;
	case 0x123b: result += "DEF Level Low"; break;
	case 0x123c: result += "SCR Inducement"; break;
	case 0x123d: result += "MSC Old version"; break;
	case 0x123e: result += "MSC ID alarm"; break;
	case 0x123f: result += "MSC failure"; break;
	case 0x1240: result += "MSC priority Error"; break;
	case 0x1241: result += "Fuel Sender open circuit"; break;
	case 0x1242: result += "Over speed runaway"; break;
	case 0x1243: result += "Over frequency run away"; break;
	case 0x1244: result += "Coolant sensor open circuit"; break;
	case 0x1245: result += "Remote display link lost"; break;
	case 0x1246: result += "Fuel tank bund level"; break;
	case 0x1247: result += "Charge air temperature"; break;
	case 0x1248: result += "Fuel level high"; break;
	case 0x1249: result += "Gen breaker failed to open (v5.0+)"; break;
	case 0x124a: result += "Mains breaker failed to open (v5.0+) – 7x20 only"; break;
	case 0x124b: result += "Fail to synchronise (v5.0+) – 7x20 only"; break;
	case 0x124c: result += "AVR Data Fail (v5.0+)"; break;
	case 0x124d: result += "AVR DM1 Red Stop Lamp (v5.0+)"; break;
	case 0x124e: result += "Escape Mode (v5.0+)"; break;
	case 0x124f: result += "Coolant high temp electrical trip (v5.0+)"; break;

	/* DSE error codes 0x1300 to 0x13FF: DSE 8xxx family */
	case 0x1300: result += "Emergency stop"; break;
	case 0x1301: result += "Low oil pressure"; break;
	case 0x1302: result += "High coolant temperature"; break;
	case 0x1303: result += "Low coolant temperature"; break;
	case 0x1304: result += "Under speed"; break;
	case 0x1305: result += "Over speed"; break;
	case 0x1306: result += "Generator Under frequency"; break;
	case 0x1307: result += "Generator Over frequency"; break;
	case 0x1308: result += "Generator low voltage"; break;
	case 0x1309: result += "Generator high voltage"; break;
	case 0x130a: result += "Battery low voltage"; break;
	case 0x130b: result += "Battery high voltage"; break;
	case 0x130c: result += "Charge alternator failure"; break;
	case 0x130d: result += "Fail to start"; break;
	case 0x130e: result += "Fail to stop"; break;
	case 0x130f: result += "Generator fail to close"; break;
	case 0x1310: result += "Mains fail to close"; break;
	case 0x1311: result += "Oil pressure sender fault"; break;
	case 0x1312: result += "Loss of magnetic pick up"; break;
	case 0x1313: result += "Magnetic pick up open circuit"; break;
	case 0x1314: result += "Generator high current"; break;
	case 0x1315: result += "Calibration lost"; break;
	case 0x1316: result += "Low fuel level"; break;
	case 0x1317: result += "CAN ECU Warning"; break;
	case 0x1318: result += "CAN ECU Shutdown"; break;
	case 0x1319: result += "CAN ECU Data fail"; break;
	case 0x131a: result += "Low oil level switch"; break;
	case 0x131b: result += "High temperature switch"; break;
	case 0x131c: result += "Low fuel level switch"; break;
	case 0x131d: result += "Expansion unit watchdog alarm"; break;
	case 0x131e: result += "kW overload alarm"; break;
	case 0x131f: result += "Negative phase sequence current alarm"; break;
	case 0x1320: result += "Earth fault trip alarm"; break;
	case 0x1321: result += "Generator phase rotation alarm"; break;
	case 0x1322: result += "Auto Voltage Sense Fail"; break;
	case 0x1323: result += "Maintenance alarm"; break;
	case 0x1324: result += "Loading frequency alarm"; break;
	case 0x1325: result += "Loading voltage alarm"; break;
	case 0x1326: result += "Fuel usage running"; break;
	case 0x1327: result += "Fuel usage stopped"; break;
	case 0x1328: result += "Protections disabled"; break;
	case 0x1329: result += "Protections blocked"; break;
	case 0x132a: result += "Generator breaker failed to open"; break;
	case 0x132b: result += "Mains breaker failed to open"; break;
	case 0x132c: result += "Bus breaker failed to close"; break;
	case 0x132d: result += "Bus breaker failed to open"; break;
	case 0x132e: result += "Generator reverse power alarm"; break;
	case 0x132f: result += "Short circuit alarm"; break;
	case 0x1330: result += "Air flap closed alarm"; break;
	case 0x1331: result += "Failure to sync"; break;
	case 0x1332: result += "Bus live"; break;
	case 0x1333: result += "Bus not live"; break;
	case 0x1334: result += "Bus phase rotation"; break;
	case 0x1335: result += "Priority selection error"; break;
	case 0x1336: result += "MSC data error"; break;
	case 0x1337: result += "MSC ID error"; break;
	case 0x1338: result += "Bus low voltage"; break;
	case 0x1339: result += "Bus high voltage"; break;
	case 0x133a: result += "Bus low frequency"; break;
	case 0x133b: result += "Bus high frequency"; break;
	case 0x133c: result += "MSC failure"; break;
	case 0x133d: result += "MSC too few sets"; break;
	case 0x133e: result += "MSC alarms inhibited"; break;
	case 0x133f: result += "MSC old version units on the bus"; break;
	case 0x1340: result += "Mains reverse power alarm/mains export alarm"; break;
	case 0x1341: result += "Minimum sets not reached"; break;
	case 0x1342: result += "Insufficient capacity"; break;
	case 0x1343: result += "Out of sync"; break;
	case 0x1344: result += "Alternative aux mains fail"; break;
	case 0x1345: result += "Loss of excitation"; break;
	case 0x1346: result += "Mains ROCOF"; break;
	case 0x1347: result += "Mains vector shift"; break;
	case 0x1348: result += "Mains decoupling low frequency stage 1"; break;
	case 0x1349: result += "Mains decoupling high frequency stage 1"; break;
	case 0x134a: result += "Mains decoupling low voltage stage 1"; break;
	case 0x134b: result += "Mains decoupling high voltage stage 1"; break;
	case 0x134c: result += "Mains decoupling combined alarm"; break;
	case 0x134d: result += "Inlet Temperature"; break;
	case 0x134e: result += "Mains phase rotation alarm identifier"; break;
	case 0x134f: result += "AVR Max Trim Limit alarm"; break;
	case 0x1350: result += "High coolant temperature electrical trip alarm"; break;
	case 0x1351: result += "Temperature sender open circuit alarm"; break;
	case 0x1352: result += "Out of sync Bus"; break;
	case 0x1353: result += "Out of sync Mains"; break;
	case 0x1354: result += "Bus 1 Live"; break;
	case 0x1355: result += "Bus 1 Phase Rotation"; break;
	case 0x1356: result += "Bus 2 Live"; break;
	case 0x1357: result += "Bus 2 Phase Rotation"; break;
	case 0x1359: result += "ECU Protect"; break;
	case 0x135a: result += "ECU Malfunction"; break;
	case 0x135b: result += "Indication"; break;
	case 0x135e: result += "HEST Active"; break;
	case 0x135f: result += "DPTC Filter"; break;
	case 0x1360: result += "Water In Fuel"; break;
	case 0x1361: result += "ECU Heater"; break;
	case 0x1362: result += "ECU Cooler"; break;
	case 0x136c: result += "High fuel level"; break;
	case 0x136e: result += "Module Communication Fail (8661)"; break;
	case 0x136f: result += "Bus Module Warning (8661)"; break;
	case 0x1370: result += "Bus Module Trip (8661)"; break;
	case 0x1371: result += "Mains Module Warning (8661)"; break;
	case 0x1372: result += "Mains Module Trip (8661)"; break;
	case 0x1373: result += "Load Live (8661)"; break;
	case 0x1374: result += "Load Not Live (8661)"; break;
	case 0x1375: result += "Load Phase Rotation (8661)"; break;
	case 0x1376: result += "DEF Level Low"; break;
	case 0x1377: result += "SCR Inducement"; break;
	case 0x1378: result += "Heater Sensor Failure Alarm"; break;
	case 0x1379: result += "Mains Over Zero Sequence Volts Alarm"; break;
	case 0x137a: result += "Mains Under Positive Sequence Volts Alarm"; break;
	case 0x137b: result += "Mains Over Negative Sequence Volts Alarm"; break;
	case 0x137c: result += "Mains Asymmetry High Alarm"; break;
	case 0x137d: result += "Bus Over Zero Sequence Volts Alarm"; break;
	case 0x137e: result += "Bus Under Positive Sequence Volts Alarm"; break;
	case 0x137f: result += "Bus Over Negative Sequence Volts Alarm"; break;
	case 0x1380: result += "Bus Asymmetry High Alarm"; break;
	case 0x1381: result += "E-Trip Stop Inhibited"; break;
	case 0x1382: result += "Fuel Tank Bund Level High"; break;
	case 0x1383: result += "MSC Link 1 Data Error"; break;
	case 0x1384: result += "MSC Link 2 Data Error"; break;
	case 0x1385: result += "Bus 2 Low Voltage"; break;
	case 0x1386: result += "Bus 2 High Voltage"; break;
	case 0x1387: result += "Bus 2 Low Frequency"; break;
	case 0x1388: result += "Bus 2 High Frequency"; break;
	case 0x1389: result += "MSC Link 1 Failure"; break;
	case 0x138a: result += "MSC Link 2 Failure"; break;
	case 0x138b: result += "MSC Link 1 Too Few Sets"; break;
	case 0x138c: result += "MSC Link 2 Too Few Sets"; break;
	case 0x138d: result += "MSC Link 1 and 2 Failure"; break;
	case 0x138e: result += "Electrical Trip from 8660"; break;
	case 0x138f: result += "AVR CAN DM1 Red Stop Lamp Fault"; break;
	case 0x1390: result += "Gen Over Zero Sequence Volts Alarm"; break;
	case 0x1391: result += "Gen Under Positive Sequence Volts Alarm"; break;
	case 0x1392: result += "Gen Over Negative Sequence Volts Alarm"; break;
	case 0x1393: result += "Gen Asymmetry High Alarm"; break;
	case 0x1394: result += "Mains decoupling low frequency stage 2"; break;
	case 0x1395: result += "Mains decoupling high frequency stage 2"; break;
	case 0x1396: result += "Mains decoupling low voltage stage 2"; break;
	case 0x1397: result += "Mains decoupling high voltage stage 2"; break;
	case 0x1398: result += "Fault Ride Through event"; break;
	case 0x1399: result += "AVR Data Fail"; break;
	case 0x139a: result += "AVR Red Lamp"; break;

	/* DSE error codes 0x1400 to 0x14FF: DSE 7450 */
	case 0x1400: result += "Emergency stop"; break;
	case 0x1401: result += "Low oil pressure"; break;
	case 0x1402: result += "High coolant temperature"; break;
	case 0x1403: result += "Low coolant temperature"; break;
	case 0x1404: result += "Under speed"; break;
	case 0x1405: result += "Over speed"; break;
	case 0x1406: result += "Generator Under frequency"; break;
	case 0x1407: result += "Generator Over frequency"; break;
	case 0x1408: result += "Generator low voltage"; break;
	case 0x1409: result += "Generator high voltage"; break;
	case 0x140a: result += "Battery low voltage"; break;
	case 0x140b: result += "Battery high voltage"; break;
	case 0x140c: result += "Charge alternator failure"; break;
	case 0x140d: result += "Fail to start"; break;
	case 0x140e: result += "Fail to stop"; break;
	case 0x140f: result += "Generator fail to close"; break;
	case 0x1410: result += "Mains fail to close"; break;
	case 0x1411: result += "Oil pressure sender fault"; break;
	case 0x1412: result += "Loss of magnetic pick up"; break;
	case 0x1413: result += "Magnetic pick up open circuit"; break;
	case 0x1414: result += "Generator high current"; break;
	case 0x1415: result += "Calibration lost"; break;
	case 0x1416: result += "Low fuel level"; break;
	case 0x1417: result += "CAN ECU Warning"; break;
	case 0x1418: result += "CAN ECU Shutdown"; break;
	case 0x1419: result += "CAN ECU Data fail"; break;
	case 0x141a: result += "Low oil level switch"; break;
	case 0x141b: result += "High temperature switch"; break;
	case 0x141c: result += "Low fuel level switch"; break;
	case 0x141d: result += "Expansion unit watchdog alarm"; break;
	case 0x141e: result += "kW overload alarm"; break;
	case 0x141f: result += "Negative phase sequence current alarm"; break;
	case 0x1420: result += "Earth fault trip alarm"; break;
	case 0x1421: result += "Generator phase rotation alarm"; break;
	case 0x1422: result += "Auto Voltage Sense Fail"; break;
	case 0x1423: result += "Maintenance alarm"; break;
	case 0x1424: result += "Loading frequency alarm"; break;
	case 0x1425: result += "Loading voltage alarm"; break;
	case 0x1426: result += "Fuel usage running"; break;
	case 0x1427: result += "Fuel usage stopped"; break;
	case 0x1428: result += "Protections disabled"; break;
	case 0x1429: result += "Protections blocked"; break;
	case 0x142a: result += "Generator breaker failed to open"; break;
	case 0x142b: result += "Mains breaker failed to open"; break;
	case 0x142c: result += "Bus breaker failed to close"; break;
	case 0x142d: result += "Bus breaker failed to open"; break;
	case 0x142e: result += "Generator reverse power alarm"; break;
	case 0x142f: result += "Short circuit alarm"; break;
	case 0x1430: result += "Air flap closed alarm"; break;
	case 0x1431: result += "Failure to sync"; break;
	case 0x1432: result += "Bus live"; break;
	case 0x1433: result += "Bus not live"; break;
	case 0x1434: result += "Bus phase rotation"; break;
	case 0x1435: result += "Priority selection error"; break;
	case 0x1436: result += "MSC data error"; break;
	case 0x1437: result += "MSC ID error"; break;
	case 0x1438: result += "Bus low voltage"; break;
	case 0x1439: result += "Bus high voltage"; break;
	case 0x143a: result += "Bus low frequency"; break;
	case 0x143b: result += "Bus high frequency"; break;
	case 0x143c: result += "MSC failure"; break;
	case 0x143d: result += "MSC too few sets"; break;
	case 0x143e: result += "MSC alarms inhibited"; break;
	case 0x143f: result += "MSC old version units on the bus"; break;
	case 0x1440: result += "Mains reverse power alarm/mains export alarm"; break;
	case 0x1441: result += "Minimum sets not reached"; break;
	case 0x1442: result += "Insufficient capacity"; break;
	case 0x1443: result += "Out of sync"; break;
	case 0x1444: result += "Alternative aux mains fail"; break;
	case 0x1445: result += "Loss of excitation"; break;
	case 0x1446: result += "Mains ROCOF"; break;
	case 0x1447: result += "Mains vector shift"; break;
	case 0x1448: result += "Mains decoupling low frequency"; break;
	case 0x1449: result += "Mains decoupling high frequency"; break;
	case 0x144a: result += "Mains decoupling low voltage"; break;
	case 0x144b: result += "Mains decoupling high voltage"; break;
	case 0x144c: result += "Mains decoupling combined alarm"; break;
	case 0x144d: result += "Charge Air Temperature"; break;
	case 0x144e: result += "Mains phase rotation alarm identifier"; break;
	case 0x144f: result += "AVR Max Trim Limit alarm"; break;
	case 0x1450: result += "High coolant temperature electrical trip alarm"; break;
	case 0x1451: result += "Temperature sender open circuit alarm"; break;
	case 0x1459: result += "ECU Protect"; break;
	case 0x145a: result += "ECU Malfunction"; break;
	case 0x145b: result += "Indication"; break;
	case 0x145c: result += "ECU Red"; break;
	case 0x145d: result += "ECU Amber"; break;
	case 0x145e: result += "Electrical Trip"; break;
	case 0x145f: result += "Aftertreatment Exhaust"; break;
	case 0x1460: result += "Water In Fuel"; break;
	case 0x1461: result += "ECU Heater"; break;
	case 0x1462: result += "ECU Cooler"; break;
	case 0x1463: result += "DC Total Watts Overload"; break;
	case 0x1464: result += "High Plant Battery Temperature"; break;
	case 0x1465: result += "Low Plant Battery Temperature"; break;
	case 0x1466: result += "Low Plant Battery Voltage"; break;
	case 0x1467: result += "High Plant Battery Voltage"; break;
	case 0x1468: result += "Plant Battery Depth Of Discharge"; break;
	case 0x1469: result += "DC Battery Over Current"; break;
	case 0x146a: result += "DC Load Over Current"; break;
	case 0x146b: result += "High Total DC Current"; break;

	/* DSE error codes 0x1500 to 0x15FF: DSE 71xx/66xx/60xx/L40x/4xxx/45xx MKII family */
	case 0x1500: result += "Emergency stop"; break;
	case 0x1501: result += "Low oil pressure"; break;
	case 0x1502: result += "High coolant temperature"; break;
	case 0x1503: result += "Low coolant temperature"; break;
	case 0x1504: result += "Under speed"; break;
	case 0x1505: result += "Over speed"; break;
	case 0x1506: result += "Generator Under frequency"; break;
	case 0x1507: result += "Generator Over frequency"; break;
	case 0x1508: result += "Generator low voltage"; break;
	case 0x1509: result += "Generator high voltage"; break;
	case 0x150a: result += "Battery low voltage"; break;
	case 0x150b: result += "Battery high voltage"; break;
	case 0x150c: result += "Charge alternator failure"; break;
	case 0x150d: result += "Fail to start"; break;
	case 0x150e: result += "Fail to stop"; break;
	case 0x150f: result += "Generator fail to close"; break;
	case 0x1510: result += "Mains fail to close"; break;
	case 0x1511: result += "Oil pressure sender fault"; break;
	case 0x1512: result += "Loss of Mag Pickup signal"; break;
	case 0x1513: result += "Magnetic pick up open circuit"; break;
	case 0x1514: result += "Generator high current"; break;
	case 0x1515: result += "Calibration lost"; break;
	case 0x1517: result += "CAN ECU Warning"; break;
	case 0x1518: result += "CAN ECU Shutdown"; break;
	case 0x1519: result += "CAN ECU Data fail"; break;
	case 0x151a: result += "Low oil level switch"; break;
	case 0x151b: result += "High temperature switch"; break;
	case 0x151c: result += "Low fuel level switch"; break;
	case 0x151d: result += "Expansion unit watchdog alarm"; break;
	case 0x151e: result += "kW overload alarm"; break;
	case 0x151f: result += "Negative phase sequence alarm"; break;
	case 0x1520: result += "Earth fault trip"; break;
	case 0x1521: result += "Generator phase rotation alarm"; break;
	case 0x1522: result += "Auto Voltage Sense fail"; break;
	case 0x1524: result += "Temperature sensor open circuit"; break;
	case 0x1525: result += "Low fuel level"; break;
	case 0x1526: result += "High fuel level"; break;
	case 0x1527: result += "Water in Fuel"; break;
	case 0x1528: result += "DEF Level Low"; break;
	case 0x1529: result += "SCR Inducement"; break;
	case 0x152a: result += "Hest Active"; break;
	case 0x152b: result += "DPTC Filter"; break;
	}

	return result;
}
