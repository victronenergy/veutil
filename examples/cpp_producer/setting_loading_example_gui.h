#pragma once

#include <QWidget>
#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitem_loader.hpp>

namespace Ui {
class SettingLoadingExampleGui;
}

class SettingLoadingExampleGui : public QWidget
{
	Q_OBJECT

	VeQItem * mRoot;
	VeQItemLoader mLoader;
public:
	explicit SettingLoadingExampleGui(VeQItem *root, QWidget *parent = 0);
	~SettingLoadingExampleGui();

private slots:
	void on_btnLoadFakeSettings_clicked();

	void on_btnCommit_clicked();

	void on_btnDiscard_clicked();

private:
	Ui::SettingLoadingExampleGui *ui;
};
