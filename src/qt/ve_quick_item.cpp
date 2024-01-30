#include <math.h>

#include <veutil/qt/ve_quick_item.hpp>

/* QT Quick Part */

void VeQuickItem::setMax(const double &max)
{
	setSourceMax(convertFromDisplay(QVariant(max)).toDouble());
	mMaxInDisplayUnits = true;
}

void VeQuickItem::setSourceMax(const double &max)
{
	mMaxInDisplayUnits = false;
	if (mSourceMax == max)
		return;
	mSourceMax = max;
	emit maxChanged();
}

void VeQuickItem::setMin(const double &min)
{
	setSourceMin(convertFromDisplay(QVariant(min)).toDouble());
	mMinInDisplayUnits = true;
}

void VeQuickItem::setSourceMin(const double &min)
{
	mMinInDisplayUnits = false;
	if (mSourceMin == min)
		return;
	mSourceMin = min;
	emit minChanged();
}

QString VeQuickItem::getText(bool force)
{
	if (!mItem)
		return QString();

	switch (mTextMode) {
	case TextMode::FromItem: {
		QString text = mInvalidate ? mItem->getText(force) : mItem->getLastValidText();
		if (text.isEmpty())
			return mInvalidText;
		return text;
	}
	case TextMode::Assigned:
		return mAssignedText;
	case TextMode::Format:
		QVariant value = mInvalidate ? getValue(force) : mItem->getLastValidValue();
		if (!value.isValid())
			return mInvalidText;
		return QString::number(value.toDouble(), 'f', mDecimals) + mUnit;
	}

	return QString();
}

void VeQuickItem::setText(const QString &text)
{
	QString oldText = getText();

	changeTextMode(TextMode::Assigned);
	mAssignedText = text;

	checkTextChanged(oldText);
}

void VeQuickItem::setUid(QString uid)
{
	if (mItem) {
		// check it changed at all
		if (uid == mItem->uniqueId())
			return;

		if (mItem->uniqueId().isEmpty() && mItem->getValue().isValid())
			qDebug() << "Changing an uid of an item with valid value set is weird, ignoring value" <<
						mItem->getValue() << "for" << uid;
	}

	teardown();
	if (uid == "") {
		mItem = new VeQItemLocal(0);
		mIsAllocated = true;
	} else {
		mItem = VeQItems::getRoot()->itemGetOrCreate(uid, true, false);
		mIsAllocated = false;
	}
	if (mItem)
		setup();
	emit uidChanged();
}

QVariant VeQuickItem::getValue(bool force)
{
	if (!mItem)
		return QVariant();
	QVariant value = mInvalidate ? mItem->getValue(force) : mItem->getLastValidValue();
	return convertToDisplay(value);
}

/**
 * Handle changes of the value property.
 *
 * In qml it should be possible to set the value property of local items (the once
 * "produced"), but not the once being bound (the once "consumed"). In the latter
 * case the local value would otherwise be updated before it is even send to the remote
 * side (which might refuse the change, or not even be available at the moment etc). In
 * such a case the value property should be updated after the value was actually changed
 * and confirmed to us, which is exactly what setValue does and that should be used
 * instead in such cases (and will act exactly the same on local items).
 *
 * For example:
 *
 * VeQuickItem {
 *   value: "This should work fine!"
 * }
 *
 * VeQuickItem {
 *   value: "This should be refused"
 *   uid: "Some/Remote/Value"
 * }
 *
 * The latter should be changed to:
 *
 *  VeQuickItem {
 *   uid: "Some/Remote/Value"
 *
 *   Component.onCompleted: setValue("This should work as well")
 *   onValueChanged: console.log("the remote value changed to " + value)
 * }
 *
 */
void VeQuickItem::setValueProperty(QVariant value)
{
	setSourceValueProperty(convertFromDisplay(value));
	mValueInDisplayUnits = true;
}

/**
 * The VeQuickItem can perform some unit conversions.
 *
 * VeQuickItem {
 *   sourceUnit: Unit.Celsius // optional
 *   displayUnit: Unit.Fahrenheit
 *
 *   sourceValue: 50
 *   sourceMin: 0
 *   sourceMax: 100
 *   decimals: 1
 * }
 *
 * Will define a value between 0 and 100 °C with a value of 50°C displayed in Fahrenheit:
 *   // min -> 32 °F
 *   // max -> 212 °F
 *   // value -> 122 °F
 *   // unit -> '°F'
 *   // text -> 122.0 °F
 *
 * For a simply bound Item adding an unit should enough to support different units.
 *
 * VeQuickItem {
 *   uid: "something"
 *   displayUnit: Unit.Fahrenheit
 * }
 *
 * min / max can be assigned e.g. to limit it to the value of another
 * value.
 *
 * NOTE: the source min / max will change along! sourceMin / sourceMax
 * can also be set and min / max will change accordingly.
 *
 * NOTE: the min / max can differ slightly from what is assigned due to rounding errors.
 * In normal cases that shouldn't happen though.
 *
 * The non overwritten (as in the values defined by the item) are available as
 * defaultMin / defaultSourceMin to allow construct like:
 *
 * min: (condition ? something : defaultMin).
 * The sourceUnit is optional, a default will be used when not set.
 */

void VeQuickItem::setSourceValueProperty(const QVariant &value)
{
	if (!mItem)
		return;

	if (mItem->uniqueId() != "") {
		qDebug() << "ignoring request to set value on bound qml Item, please use setValue instead";
		return;
	}

	mValueInDisplayUnits = false;
	mItem->produceValue(value);
	mItem->produceText(value.toString());
}

void VeQuickItem::setUnit(const QString &unit)
{
	QString oldText = getText();

	changeTextMode(TextMode::Format);

	if (mUnit != unit) {
		mUnit = unit;
		emit unitChanged();
	}

	checkTextChanged(oldText);
}

void VeQuickItem::setDecimals(int decimals)
{
	QString oldText = getText();

	changeTextMode(TextMode::Format);

	if (mDecimals != decimals) {
		mDecimals = decimals;
		emit decimalsChanged();
	}

	checkTextChanged(oldText);
}

void VeQuickItem::setInvalidate(bool value)
{
	if (mInvalidate == value)
		return;
	mInvalidate = value;
	emit invalidateChanged();
}

void VeQuickItem::setInvalidText(const QString &str)
{
	if (mInvalidText == str)
		return;

	QString oldText = getText();
	mInvalidText = str;
	emit invalidTextChanged();

	checkTextChanged(oldText);
}

void VeQuickItem::setSourceUnit(Unit::Type unit)
{
	if (mSourceUnit == unit)
		return;

	DisplayUnitValues prevValues;
	storeDisplayUnitValues(prevValues);

	mSourceUnit = unit;
	emit sourceUnitChanged();
	unitConversionChanged();

	restoreDisplayUnitValues(prevValues);
}

void VeQuickItem::setDisplayUnit(Unit::Type unit)
{
	if (mDisplayUnit == unit || !mItem)
		return;

	DisplayUnitValues prevValues;
	storeDisplayUnitValues(prevValues);

	changeTextMode(TextMode::Format);
	mDisplayUnit = unit;
	mUnitConverter = UnitConverters::instance().get(unit);
	if (mUnitConverter) {
		setUnit(mUnitConverter->unitToString(unit));
	} else {
		qWarning() << unit << "is not a valid display unit for" << mItem->uniqueId();
	}
	emit displayUnitChanged();
	unitConversionChanged();

	restoreDisplayUnitValues(prevValues);
}

void VeQuickItem::setIsSetting(bool value)
{
	if (mIsSetting == value)
		return;

	mIsSetting = value;
	if (mIsSetting) {
		getDefaultMin();
		getDefaultMax();
		getDefault();
	}
	emit isSettingChanged();
}

QVariant VeQuickItem::convertToDisplay(const QVariant &value)
{
	if (!mUnitConverter)
		return value;

	bool ok;
	double ret = mUnitConverter->convert(value.toDouble(&ok), mSourceUnit, mDisplayUnit);
	if (!ok)
		return QVariant();

	// Try to keep rounded figures, doubles don't have infinite precision.
	// E.g. returning 99.99999999999999999 will cause unexpected behaviour when comparing
	// with 100, to which the max might be set. Obviously it assumes there are no valid
	// values below the 1e-6 order nor idiotic high numbers.
	ret = round(ret * 1e6) / 1e6;

	return ret;
}

QVariant VeQuickItem::convertFromDisplay(const QVariant &value)
{
	if (!mUnitConverter)
		return value;

	return mUnitConverter->convert(value.toDouble(), mDisplayUnit, mSourceUnit);
}

void VeQuickItem::unitConversionChanged()
{
	emit valueChanged();
	emit textChanged();
	emit minChanged();
	emit maxChanged();
	emit defaultChanged();
}

void VeQuickItem::storeDisplayUnitValues(DisplayUnitValues &values)
{
	if (mValueInDisplayUnits)
		values.value = getValue();
	if (mMaxInDisplayUnits)
		values.max = getMax();
	if (mMinInDisplayUnits)
		values.min = getMin();
}

void VeQuickItem::restoreDisplayUnitValues(const DisplayUnitValues &values)
{
	if (mValueInDisplayUnits)
		setValueProperty(values.value);
	if (mMaxInDisplayUnits)
		setMax(values.max);
	if (mMinInDisplayUnits)
		setMin(values.min);
}
