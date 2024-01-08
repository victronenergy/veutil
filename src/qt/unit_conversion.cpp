#include <cmath>
#include <QDebug>
#include <QString>

#include <veutil/qt/unit_conversion.hpp>

// Basic interface to convert a base unit to a derived one and its reverse.
class UnitConversion {
public:
	UnitConversion(QString const &unit) :
		mUnit(unit)
	{
	}
	virtual ~UnitConversion() {}

	virtual double toBase(double value) = 0;
	virtual double toDerived(double value) = 0;

	QString const &unit() const { return mUnit; }

private:
	QString mUnit;
};

// Stub, a base unit does not need to be converted to itself.
class UnitBase : public UnitConversion
{
public:
	UnitBase(QString const &unit) :
		UnitConversion(unit)
	{
	}

	double toBase(double value) override { return value; }
	double toDerived(double value) override { return value; }
};

// Linear conversion from a base unit to a derived one and its inverse.
// Most common units difference are linear, there are some logs etc (dB).
// If needed they can implement their own class.
class UnitLinear : public UnitConversion {
public:
	UnitLinear(QString const &unit, double scale, double offset = 0) :
		UnitConversion(unit),
		mScale(scale),
		mOffset(offset)
	{
	}

	double toBase(double value) override {
		return (value - mOffset) / mScale;
	}

	double toDerived(double value) override {
		return value * mScale + mOffset;
	}

private:
	double mScale;
	double mOffset;
};

// Note: it is assumed the unit info is allocated, hence needs to be freed.
UnitConverter::~UnitConverter()
{
	for (auto it = mUnits.begin(); it != mUnits.end(); it++)
		delete it->second;
}

// The actual conversion routine, which can convert from any known unit to
// another one.
//
// Note: it will error and return NaN when used incorrectly, you cannot convert
// meter to seconds e.g. it is simply considered a programming error.
double UnitConverter::convert(double value, Unit::Type from, Unit::Type to) const
{
	if (from == Unit::Default)
		from = mDefaultUnit;

	if (to == Unit::Default)
		to = mDefaultUnit;

	auto fromConversion = mUnits.find(from);
	auto toConversion = mUnits.find(to);

	if (fromConversion == mUnits.end()) {
		qDebug() << "no conversion found for " << int(from);
		return nan("");
	}

	if (toConversion == mUnits.end()) {
		qDebug() << "no conversion found for " << int(to);
		return nan("");
	}

	double baseValue = fromConversion->second->toBase(value);
	return toConversion->second->toDerived(baseValue);
}

QString UnitConverter::unitToString(Unit::Type unit) const
{
	auto unitDef = mUnits.find(unit);

	if (unitDef == mUnits.end())
		return QString();

	return unitDef->second->unit();
}

// Temperature conversions
TemperatureConverter::TemperatureConverter() :
	UnitConverter(Unit::Celsius, {
		{ Unit::Celsius, new UnitBase(QString::fromUtf8("°C")) },
		{ Unit::Kelvin, new UnitLinear("K", 1, 273.15) },
		{ Unit::Fahrenheit, new UnitLinear(QString::fromUtf8("°F"), 9.0 / 5.0, 32) }
	}) {}

// Volume conversions
VolumeConverter::VolumeConverter() :
	UnitConverter(Unit::CubicMeter, {
		{ Unit::CubicMeter, new UnitBase("m<sup>3</sup>") },
		{ Unit::Litre, new UnitLinear("L", 1.0 * 1000.0) },
		{ Unit::ImperialGallon, new UnitLinear("gall", 219.969157) },
		{ Unit::UsGallon, new UnitLinear("gall", 264.172052) },
	}) {}


const UnitConverter *UnitConverters::get(Unit::Type unit)
{
	for (auto quantity : mUnitConverters)
		if (quantity->get(unit))
			return quantity;

	return nullptr;
}

double UnitConverters::convert(double value, Unit::Type from, Unit::Type to)
{
	auto quantity = get(to);
	return (quantity ? quantity->convert(value, from, to) : nan(""));
}

QString UnitConverters::convertToString(double value, Unit::Type from, int decimals, Unit::Type to)
{
	auto quantity = get(to);
	if (!quantity)
		return QString();

	UnitConversion const *conv = quantity->get(to);
	if (!conv)
		return QString();

	return QString::number(quantity->convert(value, from, to), 'f', decimals) + conv->unit();
}
