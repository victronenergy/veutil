#include "ve_qitem_table_widget.hpp"
#include "ui_ve_qitem_table_widget.h"

#include <veutil/qt/ve_qitem_sort_table_model.hpp>

// Constructor which allows passing flags to the TableModel
VeQItemTableWidget::VeQItemTableWidget(VeQItem *root, VeQItemTableModel::Flags flags, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::VeQItemTableWidget),
	mTableModel(flags)
{
	ui->setupUi(this);
	if (root)
		mTableModel.addItem(root);

	VeQItemSortTableModel *sortModel = new VeQItemSortTableModel(&mTableModel, VeQItemSortTableModel::None, this);
	ui->tableView->setModel(sortModel);
	ui->tableView->setSortingEnabled(true);
	ui->tableView->setColumnWidth(0, 400);
	ui->tableView->sortByColumn(0, Qt::AscendingOrder);
}

VeQItemTableWidget::~VeQItemTableWidget()
{
	delete ui;
}
