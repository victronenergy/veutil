#pragma once

#include <list>
#include <map>
#include <QString>
#include <QObject>

class UnitConversion;

// Note: a QObject is needed to register the enum in QML, hence it is not an enum class,
// so Unit.Celsius can be used in both c++ and QML.

class Unit : public QObject {
	Q_OBJECT
	Q_ENUMS(Type)

public:
	enum Type {
		Default = -1,

		// Temperature
		Celsius,
		Kelvin,
		Fahrenheit,

		// Volume
		CubicMeter,
		Litre,
		ImperialGallon,
		UsGallon,
	};
};

class UnitConverter : public QObject {
	Q_OBJECT

public:
	double convert(double value, Unit::Type from, Unit::Type to) const;
	QString unitToString(Unit::Type unit) const;
	UnitConversion const *get(Unit::Type unit) const { auto conv = mUnits.find(unit); return (conv == mUnits.end() ? nullptr : conv->second); }

	// qml is broken for cross class enums, see https://bugreports.qt.io/browse/QTBUG-58454
	Q_INVOKABLE double convert(double value, int from, int to) const {
		return convert(value, static_cast<Unit::Type>(from), static_cast<Unit::Type>(to));
	}
	Q_INVOKABLE QString unitToString(int unit) const {
		return unitToString(static_cast<Unit::Type>(unit));
	}

protected:
	UnitConverter(Unit::Type defaultUnit, std::map<Unit::Type, UnitConversion *> units) :
		QObject(),
		mDefaultUnit(defaultUnit),
		mUnits(units)
	{
	}
	virtual ~UnitConverter();

	Unit::Type mDefaultUnit;
	const std::map<Unit::Type, UnitConversion *> mUnits;
};

// Specific converters...
class TemperatureConverter : public UnitConverter {
public:
	TemperatureConverter();
};

class VolumeConverter : public UnitConverter {
public:
	VolumeConverter();
};

// The common one.
class UnitConverters : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(UnitConverters)

public:
	static UnitConverters &instance()
	{
		static UnitConverters instance;
		return instance;
	}

	const TemperatureConverter temperature;
	const VolumeConverter volume;

	UnitConverter const *get(Unit::Type unit);

	// for testing / laziness, normally a specific converter will be used.
	double convert(double value, Unit::Type from, Unit::Type to);
	QString convertToString(double value, Unit::Type from, int decimals, Unit::Type to);

	// Broken QML calls..
	Q_INVOKABLE double convert(double value, int from, int to) {
		return convert(value, static_cast<Unit::Type>(from), static_cast<Unit::Type>(to));
	}
	Q_INVOKABLE QString convertToString(double value, int from, int decimals, int to) {
		return convertToString(value, static_cast<Unit::Type>(from), decimals, static_cast<Unit::Type>(to));
	}

protected:
	const std::list<UnitConverter const *> mUnitConverters;

private:
	UnitConverters() :
		mUnitConverters({
			&temperature,
			&volume,
		})
	{}
};

