#pragma once

#include <QFlags>
#include <QSortFilterProxyModel>
#include <QStringList>

#include <veutil/qt/ve_qitem_table_model.hpp>

#if defined(QT_QML_LIB)
#include <QQmlParserStatus>
#define QDeclarativeParserStatus QQmlParserStatus
#elif defined(QT_DECLARATIVE_LIB)
#include <QDeclarativeParserStatus>
#else
#include <veutil/qt/no_declarative_parser.hpp>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif
class VeQItemSortTableModel : public QSortFilterProxyModel, public QDeclarativeParserStatus
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
#endif
	Q_INTERFACES(QDeclarativeParserStatus)
	Q_ENUMS(Flags)
	Q_PROPERTY(Flags filterFlags READ filterFlags WRITE setFilterFlags NOTIFY filterFlagsChanged)
	Q_PROPERTY(QString filterRegExp READ filterRegExpStr WRITE setFilterRegExp NOTIFY filterRegExpChanged)
	Q_PROPERTY(VeQItemTableModel *model READ model WRITE setModel NOTIFY modelChanged)
	Q_PROPERTY(bool dynamicSortFilter READ dynamicSortFilter WRITE setDynamicSortFilter NOTIFY dynamicSortFilterChanged)
	Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)
	Q_PROPERTY(int sortColumn READ sortColumn WRITE setSortColumn NOTIFY sortColumnChanged)
	Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
	enum Flag {
		None = 0,
		FilterInvalid = 1,
		FilterOffline = 2,
	};

	Q_DECLARE_FLAGS(Flags, Flag)

	VeQItemSortTableModel(QObject *parent = 0) :
		QSortFilterProxyModel(parent),
		mCompleted(false),
		mTableModel(nullptr)
	{
	}

	VeQItemSortTableModel(VeQItemTableModel *model, Flags flags = FilterInvalid, QObject *parent = 0) :
		QSortFilterProxyModel(parent),
		mCompleted(true),
		mTableModel(model),
		mFlags(flags)
	{
		setSourceModel(mTableModel);
	}

	void classBegin() override {}
	void componentComplete() override;

	VeQItemTableModel *model() { return mTableModel;	}
	void setModel(VeQItemTableModel *model);

	void updateModel();

	void setDynamicSortFilter(bool enable);

	void setSortRole(int role);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QString filterRegExpStr() { return filterRegularExpression().pattern(); }
#else
	QString filterRegExpStr() { return filterRegExp().pattern(); }
#endif
	void setFilterRegExp(QString const &regExp);

	Flags filterFlags() { return mFlags; }
	void setFilterFlags(Flags flags);

	Q_INVOKABLE int sortColumn() {
		return QSortFilterProxyModel::sortColumn();
	}

	void setSortColumn(int column) {
		if (column == sortColumn())
			return;
		sort(column, sortOrder());
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return QSortFilterProxyModel::rowCount(parent);
	}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	Q_INVOKABLE void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override {
#else
	Q_INVOKABLE void sort(int column, int order = Qt::AscendingOrder) {
#endif
		bool columnChanged = sortColumn() != column;
		QSortFilterProxyModel::sort(column, Qt::SortOrder(order));
		if (columnChanged)
			emit sortColumnChanged();
	}

	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

signals:
	void filterRegExpChanged();
	void filterFlagsChanged();
	void modelChanged();
	void dynamicSortFilterChanged();
	void sortRoleChanged();
	void sortColumnChanged();
	void rowCountChanged();

private:
	bool mCompleted;
	VeQItemTableModel *mTableModel;
	Flags mFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VeQItemSortTableModel::Flags)

#undef QDeclarativeParserStatus
