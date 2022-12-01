#include <veutil/qt/ve_qitem_sort_table_model.hpp>

void VeQItemSortTableModel::componentComplete()
{
	mCompleted = true;
	updateModel();
}

void VeQItemSortTableModel::setModel(VeQItemTableModel *model) {
	if (mTableModel == model)
		return;
	mTableModel = model;
	updateModel();
	emit modelChanged();
}

void VeQItemSortTableModel::updateModel()
{
	if (!mCompleted)
		return;
	if (mTableModel)
		mTableModel->disconnect(this);
	setSourceModel(mTableModel);
	connect(this, SIGNAL(rowsRemoved(QModelIndex, int, int)), SIGNAL(rowCountChanged()));
	connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)), SIGNAL(rowCountChanged()));
	emit rowCountChanged();
}

void VeQItemSortTableModel::setDynamicSortFilter(bool enable)
{
	if (enable == dynamicSortFilter())
		return;

	QSortFilterProxyModel::setDynamicSortFilter(enable);

	// The model can be sorted in two difference ways, either it is sorted dynamically
	// and the view has to adjust, or the view determines the sorting (pressing the
	// column headers and the model needs to update according to the view / or from
	// javascript).

	// The problematic part is that the column being sorted on, can only be set from the
	// view by calling sort. If that is not done, while enabling dynamicSortOrder it is
	// not sorted, till sort(column) is called. So do force a sort on the first column,
	// if the sort column is not set.

	if (enable) {
		if (sortColumn() == -1 && sortRole() >= Qt::UserRole)
			qWarning() << "Enabling dynamicSort with a UserRole is likely not going to do what you expect, use a column instead";
	} else {
		sort(-1);
	}

	emit dynamicSortFilterChanged();
}

void VeQItemSortTableModel::setSortRole(int role) {

	if (role == sortRole())
		return;

	if (role < 0)
		sort(-1);

	QSortFilterProxyModel::setSortRole(role);

	if (sortColumn() == -1 && role >= Qt::UserRole)
		qWarning() << "Enabling dynamicSort with a UserRole is likely not going to do what you expect, use a column instead";
}

void VeQItemSortTableModel::setFilterRegExp(const QString &regExp) {
	if (regExp == filterRegExpStr())
		return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QSortFilterProxyModel::setFilterRegularExpression(regExp);
#else
	QSortFilterProxyModel::setFilterRegExp(regExp);
#endif
	emit filterRegExpChanged();
}

void VeQItemSortTableModel::setFilterFlags(VeQItemSortTableModel::Flags flags) {
	if (mFlags == flags)
		return;
	mFlags = flags;
	emit filterFlagsChanged();
	invalidateFilter();
}

bool VeQItemSortTableModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
		return false;

	if (mFlags == 0)
		return true;

	QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

	if (mFlags.testFlag(FilterInvalid)) {
		QVariant value = sourceModel()->data(index, VeQItemTableModel::ValueRole);
		if (!value.isValid())
			return false;
	}

	if (mFlags.testFlag(FilterOffline)) {
		QVariant status = sourceModel()->data(index, VeQItemTableModel::StateRole);
		if (status.toInt() == VeQItem::Offline)
			return false;
	}

	return true;
}
