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

QString GensetError::getDescription(QString errorId, int nrOfPhases)
{
	if (errorId == "")
		return tr("No error");
	if (errorId.startsWith("fischerpanda:", Qt::CaseInsensitive))
		return FischerPanda::getDescription(errorId, nrOfPhases);
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
