#pragma once

#include <map>
#include <QString>

#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/firmware_updater_data.hpp>

/*
 * Mind it, these VeQItems are for building an item tree to be shown / published /
 * exported, NOT for items being imported from a bus, since an item provider will
 * create items for those of its own type! Adding different items to such a branch
 * will cause trouble / crash etc.
 */
class VeQItemExportedLeaf : public VeQItem
{
	Q_OBJECT

public:
	VeQItemExportedLeaf() :
		VeQItem(0)
	{
		mIsLeaf = true;
	}

	VeQItem *createChild(QString id, bool isLeaf = true, bool isTrusted = true) override
	{
		Q_UNUSED(isLeaf);
		Q_UNUSED(isTrusted);

		qDebug() << "A child" << id << "cannot be created for" << uniqueId();
		Q_ASSERT(false);

		return nullptr;
	}
};

class VeQItemQuantity : public VeQItemExportedLeaf
{
	Q_OBJECT

public:
	VeQItemQuantity(int decimals = -1, QString const &unit = "", QVariant const &initial = QVariant()) :
		VeQItemExportedLeaf(),
		mDecimals(decimals),
		mUnit(unit)
	{
		produceValue(initial);
	}

	VeQItemQuantity(QVariant const &initial) :
		VeQItemExportedLeaf(),
		mDecimals(-1),
		mUnit("")
	{
		produceValue(initial);
	}

	void produceValue(QVariant value, State state = Synchronized, bool forceChanged = false) override
	{
		VeQItem::produceValue(value, state, forceChanged);

		QString text;
		if (!value.isValid()) {
			text = "-";
		} else if (mDecimals >= 0) {
			bool ok;
			double number = value.toDouble(&ok);
			text = ok ? QString::number(number, 'f', mDecimals) + mUnit : "";
		} else {
			text = value.toString();
		}

		VeQItem::produceText(text, state);
	}

private:
	int mDecimals;
	QString mUnit;
};


/*
 * Perhaps reflection / the metatype system can do this automagically,
 * for now it just needs to be spelled out.
 */
class VeQItemEnum : public VeQItemExportedLeaf
{
	Q_OBJECT

public:
	VeQItemEnum(std::map<int, QString> const &values) :
		VeQItemExportedLeaf(),
		mValues(values)
	{}

	void produceValue(QVariant value, State state = Synchronized, bool forceChanged = false) override
	{
		VeQItem::produceValue(value, state, forceChanged);

		bool ok;
		int val = value.toInt(&ok);
		QString text;
		if (ok) {
			std::map<int, QString>::const_iterator it = mValues.find(val);
			text = it == mValues.end() ? "-" : it->second;
		} else {
			text= "-";
		}

		VeQItem::produceText(text, state);
	}

private:
	std::map<int, QString> const mValues;
};


class VeQItemUpdateState: public VeQItemEnum
{
	Q_OBJECT

public:
	VeQItemUpdateState() :
		VeQItemEnum(
			std::map<int, QString> {
				{FirmwareUpdaterData::UpdateFileNotFound, "Update file not found"},
				{FirmwareUpdaterData::ErrorDuringUpdating, "Error during update"},
				{FirmwareUpdaterData::ErrorDuringChecking, "Error during checking"},
				{FirmwareUpdaterData::Idle, "Idle"},
				{FirmwareUpdaterData::Checking, "Checking"},
				{FirmwareUpdaterData::DownloadingAndInstalling, "Installing"},
				{FirmwareUpdaterData::Rebooting, "Rebooting"}
			}
		)
	{
		produceValue(FirmwareUpdaterData::Idle);
	}
};

/*
 * Value based starting of a remote action, always zero, when anything is
 * written to it the action will be performed and it is forcefully reset
 * to 0 again.
 */
class VeQItemAction: public VeQItemQuantity
{
	Q_OBJECT

public:
	VeQItemAction() :
		VeQItemQuantity(QVariant(0))
	{
	}

	int setValue(const QVariant &value) override
	{
		Q_UNUSED(value);
		produceValue(QVariant(0), State::Synchronized, true);
		return 0;
	}
};
