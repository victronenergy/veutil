#include <QTimer>

#include "ve_qitems_example.hpp"

/**
 * @brief VeQItemsExample::VeQItemsExample
 * @param root			node to add this provider to
 * @param name			name of this provider
 * @param providerRoot	node where new services are added to
 * @param parent
 */
VeQItemsExample::VeQItemsExample(VeQItem *root, QString id, QObject *parent) :
	VeQItemProducer(root, id, parent)
{
	mQItemA = mProducerRoot->itemGetOrCreateAndProduce("This/Is/Item/A", "test");
	mQItemB = mProducerRoot->itemGetOrCreate("This/Is/Item/B");

	startTimer(1000);
	QTimer::singleShot(3000, this, SLOT(deleteItem()));
	QTimer::singleShot(5000, this, SLOT(createDelayItem()));
}

void VeQItemsExample::timerEvent(QTimerEvent *event)
{
	Q_UNUSED(event);

	if (mQItemA) {
		mQItemA->produceValue(10);
		mQItemA->produceText("10A");
	}

	int value = rand();
	mQItemB->produceValue(value);

	/* A formatted text represenation can be published as well */
	mQItemB->produceText(QString("str: %1").arg(value));

	/* Optional additional information */
	mQItemB->itemProduceProperty("min", 10);
	mQItemB->itemProduceProperty("max", 100);
	mQItemB->itemProduceProperty("defaultValue", 50);
}

void VeQItemsExample::createDelayItem()
{
	mQItemC = mProducerRoot->itemGetOrCreateAndProduce("I/Am/Late", "test");
	mQItemC->produceText("formatted string");
}

void VeQItemsExample::deleteItem()
{
	mQItemA->itemDelete();
	mQItemA = nullptr;
}
