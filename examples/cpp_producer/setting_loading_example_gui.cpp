#include "setting_loading_example_gui.h"
#include "ui_setting_loading_example_gui.h"

SettingLoadingExampleGui::SettingLoadingExampleGui(VeQItem *root, QWidget *parent) :
	QWidget(parent),
	mRoot(root),
	mLoader(root),
	ui(new Ui::SettingLoadingExampleGui)
{
	ui->setupUi(this);
}

SettingLoadingExampleGui::~SettingLoadingExampleGui()
{
	delete ui;
}

void SettingLoadingExampleGui::on_btnLoadFakeSettings_clicked()
{
	mLoader.addItem("Provider/This/Is/Item/B", "test");

	const QString findMe = "Provider/I/Am/Late";
	VeQItem *item = mRoot->itemGet(findMe);

	if (!item) {
		qDebug() << findMe << "Not found";
		return;
	}

	QVariant loadedFakeValue(QString("Commit me %1").arg(rand()));
	mLoader.addItem(item, loadedFakeValue);
}

void SettingLoadingExampleGui::on_btnCommit_clicked()
{
	mLoader.commit();
}

void SettingLoadingExampleGui::on_btnDiscard_clicked()
{
	mLoader.discard();
}
