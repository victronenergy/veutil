#pragma once

#include <QDebug>
#include <QTimer>

#include <veutil/qt/ve_qitem.hpp>

class VeQItemExample : public VeQItem
{
	Q_OBJECT

public:
	explicit VeQItemExample(VeQItemProducer *producer) :
		VeQItem(producer)
	{
	}

	QVariant getValue()
	{
		qDebug() << "someone wants the value of" << uniqueId();
		return VeQItem::getValue();
	}

	/**
	 * @brief setValue with a fake network delay
	 */
	int setValue(const QVariant &value)
	{
		VeQItem::setValue(value);

		mPendingSetValue = value;
		setState(Storing);

		// Emulate that it will take some time for the item to be applied
		QTimer::singleShot(1000, this, SLOT(setSynchronizedState()));

		return 0;
	}

protected:
	void watchedChanged()
	{
		qDebug() << uniqueId() << (mWatched ? "is being watched" : "is not watched");
	}

private slots:
	void setSynchronizedState()
	{
		produceValue(mPendingSetValue);
		produceText(mPendingSetValue.toString());
	}

private:
	QVariant mPendingSetValue;
};

class VeQItemsExample : public VeQItemProducer
{
	Q_OBJECT

public:
	VeQItemsExample(VeQItem *root, QString id, QObject *parent = 0);

	VeQItem *createItem()
	{
		return new VeQItemExample(this);
	}

private:
	void timerEvent(QTimerEvent *event);

	VeQItem *mQItemA;
	VeQItem *mQItemB;
	VeQItem *mQItemC;

private slots:
	void createDelayItem();
	void deleteItem();
};

class VeQItemConsumer : public QObject
{
	Q_OBJECT

public:
	VeQItemConsumer(VeQItem *item, QObject *parent = 0) :
		QObject(parent)
	{
		item->getValueAndChanges(this, SLOT(onValueChanged()));
		startTimer(4000);
	}

public slots:
	void onValueChanged()
	{
		VeQItem *item = static_cast<VeQItem *>(sender());
		qDebug() << item->uniqueId() << "has now value" << item->getValue();
	}

private:
	void timerEvent(QTimerEvent *event)
	{
		Q_UNUSED(event);
		deleteLater();
	}
};
