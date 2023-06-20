#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <veutil/qt/ve_qitem.hpp>

class VeQItemTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	VeQItemTreeModel(VeQItem *root = 0, QObject *parent = 0);

	void setItems(VeQItem *root);
	QVariant data(const QModelIndex &index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private slots:
	void onChildAboutTobeAdded(VeQItem *item);
	void onChildAdded(VeQItem *item);
	void onItemAboutToBeRemoved(VeQItem *item);
	void onItemRemoved(VeQItem *item);
	void onValueChanged();
	void setupValueChanges(VeQItem *item);

private:
	VeQItem *mItemRoot;
	QList<QString> mColumns;
};
