#pragma once

#include <QtGlobal>

#include <QQmlComponent>
#include <QQuickItem>

#include <veutil/qt/ve_qitem_table_model.hpp>

class VeQItemSortDelegate : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QVariant sortValue READ sortValue WRITE setValue NOTIFY valueChanged)

public:
	QVariant sortValue() { return mValue; }
	void setValue(const QVariant &value ) {
		if (mValue == value)
			return;
		mValue = value;
		emit valueChanged();
	}

signals:
	void valueChanged();

private:
	QVariant mValue;
};

class VeQItemChildModel : public VeQItemTableModel
{
	Q_OBJECT
	Q_PROPERTY(QAbstractItemModel *model READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
	Q_PROPERTY(QString childId READ childId WRITE setChildId NOTIFY childIdChanged)
	Q_PROPERTY(QQmlComponent *sortDelegate READ sortDelegate WRITE setSortDelegate NOTIFY sortDelegateChanged)
	Q_PROPERTY(int sortValueColumn READ sortValueColumn NOTIFY sortValueColumnChanged)

public:
	VeQItemChildModel(QObject *parent = 0);
	void setFlags(VeQItemTableModel::Flags flags);

	QAbstractItemModel *sourceModel() { return mTableModel; }
	void setSourceModel(QAbstractItemModel *sourceModel);

	QString childId() { return mChildId; }
	void setChildId(const QString &childId);

	QQmlComponent * sortDelegate() const { return mSortDelegate; }
	void setSortDelegate(QQmlComponent *delegate);

	QHash<int, QByteArray> roleNames() const override;
	QVariant data(const QModelIndex &index, int role) const override;

	int sortValueColumn() const { return mSortValueColumn; }

signals:
	void sourceModelChanged();
	void childIdChanged();
	void sortDelegateChanged();
	void sortValueColumnChanged();

protected slots:
	void onRowsInserted(const QModelIndex &parent, int first, int last);
	void onRowsRemoved(const QModelIndex &parent, int first, int last);
	void onModelReset();
	void onLayoutChanged();
	void onLayoutAboutToBeChanged();
	void onSourceModelDestroyed();

	void onSortDelegateValueChanged();

protected:
	void clear();

	void doInsertItem(VeQItem *item, int row) override;
	void doRemove(int n) override;

private:
	QAbstractItemModel *mTableModel;
	QString mChildId;
	QQmlComponent *mSortDelegate;
	int mSortValueColumn;
	QVector<VeQItemSortDelegate*> mSortDelegates;

	void createSortDelegate(int n);
	void addOneChild(const QModelIndex &index);
	void rebuild();
	void updateSortValueColumn();
};
