#include "ve_qitems_example.hpp"
#include <QApplication>

#include <ve_qitem_table_widget.hpp>
#include <ve_qitem_tree_widget.hpp>
#include <setting_loading_example_gui.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// Get the root of the tree
	VeQItem *mRoot = VeQItems::getRoot();
	mRoot->setId("Root");

	VeQItemsExample *provider = new VeQItemsExample(mRoot, "Provider");

	VeQItemTableModel::Flags flags = VeQItemTableModel::AddAllChildren |
		VeQItemTableModel::WithText |
		VeQItemTableModel::WithSettingInfo;
	VeQItemTableWidget *widget = new VeQItemTableWidget(mRoot, flags);
	widget->show();

#if 0
	VeQItemTreeWidget *treeWidget = new VeQItemTreeWidget(mRoot);
	treeWidget->show();
#endif

	SettingLoadingExampleGui *settingsExample = new SettingLoadingExampleGui(mRoot);
	settingsExample->show();

	// to check the watch being unset above widget must be disabled!
	new VeQItemConsumer(provider->services()->itemGetOrCreate("This/Is/Item/B"));

	return a.exec();
}
