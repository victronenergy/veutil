#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitem_table_model.hpp>
#include <veutil/qt/ve_qitem_tree_model.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();
	void onChildAdded(VeQItem *item);
	void onValueChanged();
	void onTextChanged();

private:
	void open();

	Ui::MainWindow *ui;
	VeQItem *mRoot;
	VeQItemTableModel mTableModel;
	VeQItemTreeModel mTreeModel;
	bool mIntrospect;
	bool mTraceValueChanges;
	bool mTraceTextChanges;
};

#endif
