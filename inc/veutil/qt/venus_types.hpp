#pragma once

#include <QString>

/// Common constants / simple functions as shared by Venus Qt applications.

enum class VenusServiceType
{
	UNSUPPORTED = -2,
	UNKNOWN = -1,
	MULTI,
	BATTERY,
	SOLAR_CHARGER,
	PV_INVERTER,
	AC_CHARGER,
	TANK,
	GRID_METER,
	GENSET,
	MOTOR_DRIVE,
	INVERTER,
	SYSTEM_CALC,
	TEMPERATURE_SENSOR,
	GENERATOR_STARTSTOP,
	DIGITAL_INPUT,
	PULSE_COUNTER,
	METEO,
	VECAN,
	EV_CHARGER,
	HUB4,
	AC_LOAD,
	FUEL_CELL,
	DC_SOURCE,
	ALTERNATOR,
	DC_LOAD,
	DC_SYSTEM,
	MULTI_RS,
	PLATFORM,
	DC_DC,
};

VenusServiceType venusServiceType(QString const &serviceName);
QString const venusFluidTypeName(unsigned int type);
QString venusPoEditorLanguage(QString const &languageSetting);
