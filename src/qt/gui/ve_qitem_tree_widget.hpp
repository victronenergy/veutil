#ifndef VE_QITEM_TREE_WIDGET_HPP
#define VE_QITEM_TREE_WIDGET_HPP

#include <QWidget>

#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitem_tree_model.hpp>

namespace Ui {
class VeQItemTreeWidget;
}

class VeQItemTreeWidget : public QWidget
{
		Q_OBJECT

public:
		explicit VeQItemTreeWidget(QWidget *parent = 0);
		explicit VeQItemTreeWidget(VeQItem *root, QWidget *parent = 0);

		~VeQItemTreeWidget();

private:
		VeQItemTreeModel mTreeModel;
		Ui::VeQItemTreeWidget *ui;
};

#endif
