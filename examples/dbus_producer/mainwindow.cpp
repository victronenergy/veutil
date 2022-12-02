#include <QDebug>

#include <veutil/qt/ve_qitems_dbus.hpp>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mRoot(VeQItems::getRoot()),
	mTableModel(VeQItemTableModel::AddAllChildren)
{
	ui->setupUi(this);
	// open();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onChildAdded(VeQItem *item)
{
	connect(item, SIGNAL(childAdded(VeQItem *)), SLOT(onChildAdded(VeQItem *)));

	if (mTraceValueChanges)
		connect(item, SIGNAL(valueChanged(QVariant)), SLOT(onValueChanged()));

	if (mTraceTextChanges)
		connect(item, SIGNAL(textChanged(QString)), SLOT(onTextChanged()));

	if (!mIntrospect)
		return;

	VeQItemDbus *dbusItem = dynamic_cast<VeQItemDbus *>(item);
	if (dbusItem == 0)
		return;

	// just for testing.. normally you would add the values of interest
	// instead of introspecting all possible values.
	dbusItem->introspect();
}

void MainWindow::onValueChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	qDebug() << item->uniqueId() << "value" << item->getValue();
}

void MainWindow::onTextChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	qDebug() << item->uniqueId() << "text" << item->getText();
}

void MainWindow::open()
{
	mIntrospect = true;
	bool findServices = true;
	bool bulkInitOfNewService = false;
	bool prepopulate = false;
	bool treeview = true;
	bool tableview = true;
	mTraceValueChanges = true;
	mTraceTextChanges = true;

	connect(mRoot, SIGNAL(childAdded(VeQItem *)), SLOT(onChildAdded(VeQItem *)));

	if (tableview) {
		mTableModel.addItem(mRoot);
		ui->tableView->setModel(&mTableModel);
		ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	}

	if (treeview) {
		mTreeModel.setItems(mRoot);
		ui->treeView->setModel(&mTreeModel);
	}

	VeQItemDbusProducer *provider = new VeQItemDbusProducer(mRoot, "producer", findServices, bulkInitOfNewService, this);

	/*
	 * <listen>tcp:host=localhost,bind=*,port=666,family=ipv4</listen>
	 * .......
	 * <allow_anonymous/>
	 */

	// provider->open("tcp:host=192.168.4.171,port=666", "hub2");

	if (!provider->open("session")) {
		qDebug() << "dbus connection failed!";
		return;
	}

	if (prepopulate) {
		// used for testing populating before the service is registered, see examples/dbus/c_service/
		provider->services()->itemGetOrCreate("com.victronenergy.test/Test/Signed16");
		provider->services()->itemGetOrCreate("com.victronenergy.test/Test/Signed32");
		provider->services()->itemGetOrCreate("com.victronenergy.test/Test/NonExistant");
	}
}

void MainWindow::on_pushButton_clicked()
{
	open();
	ui->pushButton->setEnabled(false);
}
