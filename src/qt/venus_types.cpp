#include <veutil/qt/venus_types.hpp>

#include <vector>
#include <QTranslator>
#include <QStringList>

VenusServiceType venusServiceType(const QString &serviceName)
{
	if (!serviceName.startsWith("com.victronenergy."))
		return VenusServiceType::UNKNOWN;

	QStringList elements = serviceName.split(".");
	if (elements.count() < 3)
		return VenusServiceType::UNKNOWN;

	QString type = elements[2];

	if (type == "battery")
		return VenusServiceType::BATTERY;

	if (type == "vebus")
		return VenusServiceType::MULTI;

	if (type == "multi")
		return VenusServiceType::MULTI_RS;

	if (type == "solarcharger")
		return VenusServiceType::SOLAR_CHARGER;

	if (type == "pvinverter")
		return VenusServiceType::PV_INVERTER;

	if (type == "charger")
		return VenusServiceType::AC_CHARGER;

	if (type == "tank")
		return VenusServiceType::TANK;

	if (type == "grid")
		return VenusServiceType::GRID_METER;

	if (type == "genset")
		return VenusServiceType::GENSET;

	if (type == "motordrive")
		return VenusServiceType::MOTOR_DRIVE;

	if (type == "inverter")
		return VenusServiceType::INVERTER;

	if (type == "system")
		return VenusServiceType::SYSTEM_CALC;

	if (type == "temperature")
		return VenusServiceType::TEMPERATURE_SENSOR;

	if (type == "generator")
		return VenusServiceType::GENERATOR_STARTSTOP;

	if (type == "pulsemeter")
		return VenusServiceType::PULSE_COUNTER;

	if (type == "digitalinput")
		return VenusServiceType::DIGITAL_INPUT;

	if (type == "unsupported")
		return VenusServiceType::UNSUPPORTED;

	if (type == "meteo")
		return VenusServiceType::METEO;

	if (type == "vecan")
		return VenusServiceType::VECAN;

	if (type == "evcharger")
		return VenusServiceType::EV_CHARGER;

	if (type == "acload")
		return VenusServiceType::AC_LOAD;

	if (type == "hub4")
		return VenusServiceType::HUB4;

	if (type == "fuelcell")
		return VenusServiceType::FUEL_CELL;

	if (type == "dcsource")
		return VenusServiceType::DC_SOURCE;

	if (type == "alternator")
		return VenusServiceType::ALTERNATOR;

	if (type == "dcload")
		return VenusServiceType::DC_LOAD;

	if (type == "dcsystem")
		return VenusServiceType::DC_SYSTEM;

	if (type == "platform")
		return VenusServiceType::PLATFORM;

	if (type == "dcdc")
		return VenusServiceType::DC_DC;

	return VenusServiceType::UNKNOWN;
}

// N2K types actually; this add translation support for them.
QString const venusFluidTypeName(unsigned int type)
{
	static std::vector<QString> fluidTypes;

	// Init once, _after_ the language is set..
	if (fluidTypes.size() == 0) {
		fluidTypes = {
			QObject::tr("Fuel"),
			QObject::tr("Fresh water"),
			QObject::tr("Waste water"),
			QObject::tr("Live well"),
			QObject::tr("Oil"),
			QObject::tr("Black water (sewage)"),
			QObject::tr("Gasoline"),
			QObject::tr("Diesel"),
			QObject::tr("LPG"),
			QObject::tr("LNG"),
			QObject::tr("Hydraulic oil"),
			QObject::tr("Raw water"),
		};
	}

	if (type < fluidTypes.size())
		return fluidTypes[type];

	return QObject::tr("Unknown");
}

// Convert to stored language setting to the language as expected by poeditor.
QString venusPoEditorLanguage(const QString &languageSetting)
{
	if (languageSetting == "zh")
		return "zh-CN";
	if (languageSetting == "se")
		return "sv";
	return languageSetting;
}
