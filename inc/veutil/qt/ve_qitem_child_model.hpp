#pragma once

#include <QtGlobal>

#ifdef QT_QML_LIB
#include <QQmlComponent>
#include <QQuickItem>
#define QDeclarativeComponent QQmlComponent
#define QDeclarativeItem QQuickItem
#else
#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#endif

#include <veutil/qt/ve_qitem_table_model.hpp>

class VeQItemSortDelegate : public  QDeclarativeItem
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
	Q_PROPERTY(QDeclarativeComponent *sortDelegate READ sortDelegate WRITE setSortDelegate NOTIFY sortDelegateChanged)
	Q_PROPERTY(int sortValueColumn READ sortValueColumn NOTIFY sortValueColumnChanged)

public:
	VeQItemChildModel(QObject *parent = 0);
	void setFlags(VeQItemTableModel::Flags flags);

	QAbstractItemModel *sourceModel() { return mTableModel; }
	void setSourceModel(QAbstractItemModel *sourceModel);

	QString childId() { return mChildId; }
	void setChildId(const QString &childId);

	QDeclarativeComponent * sortDelegate() const { return mSortDelegate; }
	void setSortDelegate(QDeclarativeComponent *delegate);

	QHash<int, QByteArray> roleNames() const;
	QVariant data(const QModelIndex &index, int role) const;

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

	virtual void doInsertItem(VeQItem *item, int row);
	virtual void doRemove(int n);

private:
	QAbstractItemModel *mTableModel;
	QString mChildId;
	QDeclarativeComponent *mSortDelegate;
	int mSortValueColumn;
	QVector<VeQItemSortDelegate*> mSortDelegates;

	void createSortDelegate(int n);
	void addOneChild(const QModelIndex &index);
	void rebuild();
	void updateSortValueColumn();
};
