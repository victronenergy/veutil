#pragma once

#include <QtGlobal>

#ifdef CFG_VE_QITEM_EXPORT
# if CFG_VE_QITEM_EXPORT
#  define VE_QITEM_EXPORT Q_DECL_EXPORT
# else
#  define VE_QITEM_EXPORT Q_DECL_IMPORT
# endif
#else
# define VE_QITEM_EXPORT
#endif

#include <cstdint>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/unit_conversion.hpp>

class VE_QITEM_EXPORT VeQuickItem : public QObject {
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
#endif
	Q_PROPERTY(QVariant defaultValue READ getDefault NOTIFY defaultChanged)
	Q_PROPERTY(double min READ getMin NOTIFY minChanged WRITE setMin)
	Q_PROPERTY(double max READ getMax NOTIFY maxChanged WRITE setMax)
	Q_PROPERTY(double defaultMin READ getDefaultMin NOTIFY defaultMinChanged)
	Q_PROPERTY(double defaultMax READ getDefaultMax NOTIFY defaultMaxChanged)
	Q_PROPERTY(QString text READ getText NOTIFY textChanged WRITE setText)
	Q_PROPERTY(QString uid READ getUid NOTIFY uidChanged WRITE setUid)
	Q_PROPERTY(QVariant value READ getValue NOTIFY valueChanged WRITE setValueProperty)
	Q_PROPERTY(VeQItem::State state READ getState NOTIFY stateChanged)
	Q_PROPERTY(bool seen READ getSeen NOTIFY seenChanged)
	Q_PROPERTY(QString unit READ getUnit NOTIFY unitChanged WRITE setUnit)
	Q_PROPERTY(int decimals READ getDecimals NOTIFY decimalsChanged WRITE setDecimals)
	Q_PROPERTY(QString invalidText READ getInvalidText NOTIFY invalidTextChanged WRITE setInvalidText)
	Q_PROPERTY(Unit::Type displayUnit READ getDisplayUnit NOTIFY displayUnitChanged WRITE setDisplayUnit)
	Q_PROPERTY(Unit::Type sourceUnit READ getSourceUnit NOTIFY sourceUnitChanged WRITE setSourceUnit)
	Q_PROPERTY(QVariant sourceValue READ getSourceValue NOTIFY valueChanged WRITE setSourceValueProperty)
	Q_PROPERTY(double sourceMin READ getSourceMin NOTIFY minChanged WRITE setSourceMin)
	Q_PROPERTY(double sourceMax READ getSourceMax NOTIFY maxChanged WRITE setSourceMax)
	Q_PROPERTY(double defaultSourceMin READ getDefaultSourceMin NOTIFY defaultMinChanged)
	Q_PROPERTY(double defaultSourceMax READ getDefaultSourceMax NOTIFY defaultMaxChanged)
	Q_PROPERTY(bool isSetting READ getIsSetting NOTIFY isSettingChanged WRITE setIsSetting)
	Q_PROPERTY(bool isValid READ getIsValid NOTIFY isValidChanged)
	Q_PROPERTY(bool invalidate READ getInvalidate NOTIFY invalidateChanged WRITE setInvalidate)

public:
	VeQuickItem(bool isSetting = true) : QObject(),
		mItem(0),
		mValueInDisplayUnits(0),
		mMaxInDisplayUnits(0),
		mMinInDisplayUnits(0),
		mIsSetting(isSetting),
		mIsValid(0),
		mInvalidate(1)
	{
		setUid("");
	}

	virtual ~VeQuickItem()
	{
		teardown();
	}

	enum TextMode {
		FromItem,
		Assigned,
		Format,
	};

	QVariant getDefault() {
		if (!mIsSetting)
			return QVariant();
		return convertToDisplay(mItem->itemProperty("defaultValue"));
	}

	double getMax() {
		if (mSourceMax.isValid())
			return convertToDisplay(mSourceMax).toDouble();
		return getDefaultMax();
	}
	double getDefaultMax() {
		QVariant max = convertToDisplay(getDefaultSourceMax());
		return max.isValid() ? max.toDouble() : mInvalidMax;
	}
	void setMax(const double &max);

	void setSourceMax(double const &max);
	double getSourceMax() {
		if (mSourceMax.isValid())
			return mSourceMax.toDouble();
		return getDefaultSourceMax();
	}
	double getDefaultSourceMax() {
		if (!mIsSetting)
			return mInvalidMax;
		QVariant max = mItem->itemProperty("max");
		return max.isValid() ? max.toDouble() : mInvalidMax;
	}

	double getMin() {
		if (mSourceMin.isValid())
			return convertToDisplay(mSourceMin).toDouble();
		return getDefaultMin();
	}
	double getDefaultMin() {
		QVariant min = convertToDisplay(mItem->itemProperty("min"));
		return min.isValid() ? min.toDouble() : mInvalidMin;
	}
	void setMin(double const &min);

	void setSourceMin(double const &min);
	double getSourceMin() {
		if (mSourceMin.isValid())
			return mSourceMin.toDouble();
		return getDefaultSourceMax();
	}
	double getDefaultSourceMin() {
		if (!mIsSetting)
			return mInvalidMin;
		QVariant max = mItem->itemProperty("min");
		return max.isValid() ? max.toDouble() : mInvalidMin;
	}

	Q_INVOKABLE QString getText(bool force = false);
	void setText(const QString &text);
	QString getUid() { return mItem->uniqueId(); }
	void setUid(QString uid);
	Q_INVOKABLE QVariant getValue(bool force = false);
	QVariant getSourceValue() { return mItem->getValue(); }
	VeQItem::State getState() { return mItem->getState(); }
	bool getSeen() { return mItem->getSeen(); }
	Q_INVOKABLE int setValue(QVariant const &value) { return mItem->setValue(convertFromDisplay(value)); }
	void setValueProperty(QVariant value);
	void setSourceValueProperty(QVariant const &value);
	QString getUnit() const { return mUnit; }
	void setUnit(QString const &unit);
	int getDecimals() { return mDecimals; }
	void setDecimals(int decimals);

	bool getInvalidate() { return mInvalidate; }
	void setInvalidate(bool value);

	QString getInvalidText() { return mInvalidText; }
	void setInvalidText(QString const &str);

	Unit::Type getSourceUnit() { return mSourceUnit; }
	void setSourceUnit(Unit::Type unit);
	Unit::Type getDisplayUnit() { return mDisplayUnit; }
	void setDisplayUnit(Unit::Type unit);

	bool getIsSetting() { return mIsSetting; }
	void setIsSetting(bool value);

	bool getIsValid() const { return mIsValid; }

signals:
	void defaultChanged();
	void minChanged();
	void defaultMinChanged();
	void maxChanged();
	void defaultMaxChanged();
	void seenChanged();

	void textChanged();
	void uidChanged();
	void valueChanged();
	void stateChanged();
	void isValidChanged();

	void unitChanged();
	void decimalsChanged();
	void invalidTextChanged();
	void invalidateChanged();

	void sourceUnitChanged();
	void displayUnitChanged();

	void isSettingChanged();

protected slots:
	virtual void onDynamicPropertyChanged(char const *name)
	{
		if (QLatin1String(name) == "min") {
			if (!mSourceMin.isValid())
				emit minChanged();
			emit defaultMinChanged();
		} else if (QLatin1String(name) == "max") {
			if (!mSourceMax.isValid())
				emit maxChanged();
			emit defaultMaxChanged();
		} else if (QLatin1String(name) == "defaultValue") {
			emit defaultChanged();
		}
	}

	void onValueChanged() {
		const bool prevIsValid = mIsValid;
		mIsValid = getValue().isValid();
		emit valueChanged();
		if (prevIsValid != mIsValid)
			emit isValidChanged();
		if (mTextMode == TextMode::Format)
			emit textChanged();
	}

	void onItemDestroyed()
	{
		qWarning() << "item destroyed while used in QML" << sender()->objectName();
		teardown();
	}

protected:
	VeQItem *mItem;
	QVariant mSourceMax;
	QVariant mSourceMin;
	uint32_t mValueInDisplayUnits:1;
	uint32_t mMaxInDisplayUnits:1;
	uint32_t mMinInDisplayUnits:1;
	uint32_t mIsSetting:1;
	uint32_t mIsValid:1;
	uint32_t mIsAllocated:1;
	uint32_t mInvalidate:1;

private:
	void setup()
	{
		mItem->getValueAndChanges(this, SLOT(onValueChanged()));

		connect(mItem, SIGNAL(stateChanged(VeQItem::State)), SIGNAL(stateChanged()));
		emit stateChanged();

		if (mTextMode == TextMode::FromItem) {
			connect(mItem, SIGNAL(textChanged(QString)), SIGNAL(textChanged()));
			emit textChanged();
		}

		connect(mItem, SIGNAL(seenChanged()), SIGNAL(seenChanged()));
		emit seenChanged();

		connect(mItem, SIGNAL(destroyed()), SLOT(onItemDestroyed()));

		connect(mItem, SIGNAL(dynamicPropertyChanged(const char*,QVariant)), SLOT(onDynamicPropertyChanged(char const *)));
		if (mIsSetting) {
			emit minChanged();
			emit defaultMinChanged();
			emit maxChanged();
			emit defaultMaxChanged();
			emit defaultChanged();
		}
	}

	void changeTextMode(TextMode mode) {
		if (mTextMode == mode)
			return;
		mTextMode = mode;
		if (mode != TextMode::FromItem)
			disconnect(mItem, SIGNAL(textChanged(QString)));
	}

	void checkTextChanged(QString const &oldText) {
		QString text = getText();
		if (text != oldText)
			emit textChanged();
	}

	void teardown()
	{
		if (mItem == 0)
			return;

		mItem->disconnect(this);
		if (mIsAllocated) {
			delete mItem;
			mIsAllocated = false;
		}
		mItem = 0;
	}

	TextMode mTextMode = TextMode::FromItem;
	QString mAssignedText;

	QString mUnit;
	int mDecimals = 0;
	QString mInvalidText = QString::fromUtf8("--");

	static double constexpr mInvalidMin = 0;
	static double constexpr mInvalidMax = 100.0;

	QVariant convertToDisplay(QVariant const &value);
	QVariant convertFromDisplay(QVariant const &value);
	void unitConversionChanged();

	Unit::Type mSourceUnit = Unit::Default;
	Unit::Type mDisplayUnit = Unit::Default;
	UnitConverter const *mUnitConverter = nullptr;

	struct DisplayUnitValues {
		double min;
		double max;
		QVariant value;
	};
	void storeDisplayUnitValues(DisplayUnitValues &values);
	void restoreDisplayUnitValues(const DisplayUnitValues &values);
};
