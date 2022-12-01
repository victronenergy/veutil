#ifndef VE_QITEM_TABLE_WIDGET_HPP
#define VE_QITEM_TABLE_WIDGET_HPP

#include <QWidget>
#include <veutil/qt/ve_qitem_table_model.hpp>

namespace Ui {
class VeQItemTableWidget;
}

class VE_QITEM_EXPORT VeQItemTableWidget : public QWidget
{
	Q_OBJECT

public:
	explicit VeQItemTableWidget(VeQItem *root,
				VeQItemTableModel::Flags flags = VeQItemTableModel::AddAllChildren, QWidget *parent = 0);
	~VeQItemTableWidget();

	VeQItemTableModel *getModel() { return &mTableModel; }
private:
	Ui::VeQItemTableWidget *ui;
	VeQItemTableModel mTableModel;
};

#endif
