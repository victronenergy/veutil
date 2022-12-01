#include "ve_qitem_tree_widget.hpp"
#include "ui_ve_qitem_tree_widget.h"

VeQItemTreeWidget::VeQItemTreeWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::VeQItemTreeWidget)
{
	ui->setupUi(this);
}

VeQItemTreeWidget::VeQItemTreeWidget(VeQItem *root, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::VeQItemTreeWidget)
{
	ui->setupUi(this);
	mTreeModel.setItems(root);
	ui->treeView->setModel(&mTreeModel);
}

VeQItemTreeWidget::~VeQItemTreeWidget()
{
	delete ui;
}
