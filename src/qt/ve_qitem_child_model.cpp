#include <QtGlobal>

#if defined(QT_QML_LIB)
#include <QQmlContext>
#include <QQmlEngine>
#define QDeclarativeContext QQmlContext
#define QDeclarativeEngine QQmlEngine
#elif defined(QT_DECLARATIVE_LIB)
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#endif

#include <veutil/qt/ve_qitem_child_model.hpp>

/*
 * This implements the VeQItemChildModel, which can be used as:
 *
 * 	VeQItemSortTableModel {
 *		id: items
 *
 *		.. some selection criteria
 *	}
 *
 *	VeQItemChildModel {
 *		id: values
 *		model: items
 *		childId: "Some/Value"
 *
 * 		sortDelegate: VeQItemSortDelegate {
 *			// This values is added an additional column and can be used for sorting.
 *			// It must be selected as such, see sortColumn: childValues.sortValueColumn below.
 *			//
 *			// There are two additional variables available here.
 *			//   item: which is item here in the childValues model
 *			//   buddy: the associated item in the sourceModel. 'items' in this case.
 *			sortValue: ""
 *		}
 *	}
 *
 * Which will fill the values model with the child items, given by childId of
 * the items in the items model and keep it in sync. Since the VeQItemChildModel
 * is derived from VeQItemTableModel, it can be sorted and filtered again by a
 * VeQItemSortTableModel. The sortValueColumn is added to sort on anything you woul
 * like.
 *
 * NOTE: support for persistant indexes is not implemented!
 */

VeQItemChildModel::VeQItemChildModel(QObject *parent) :
	VeQItemTableModel(VeQItemTableModel::AddNonLeaves | VeQItemTableModel::WithText, parent),
	mTableModel(nullptr),
	mSortDelegate(nullptr)
{
	updateSortValueColumn();

	// qt 5 has roleNames as virtual and setRoleNames is removed
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	setRoleNames(roleNames());
#endif
}

void VeQItemChildModel::setFlags(VeQItemTableModel::Flags flags)
{
	VeQItemTableModel::setFlags(flags);
	updateSortValueColumn();
}

void VeQItemChildModel::updateSortValueColumn()
{
	int old = mSortValueColumn;

	mSortValueColumn = mColumns.indexOf("sortValue");
	if (mSortValueColumn == -1) {
		mSortValueColumn = mColumns.count();
		mColumns.append("sortValue");
	}

	if (old != mSortValueColumn)
		emit sortValueColumnChanged();
}

// "properties" / custom roles when the model is used for Qt Quick
QHash<int, QByteArray> VeQItemChildModel::roleNames() const
{
	static QHash<int, QByteArray> roles;

	if (roles.isEmpty()) {
		roles = VeQItemTableModel::roleNames();
		roles[SortValueRole] = "sortValue";
		roles[BuddyRole] = "buddy";
	}

	return roles;
}

QVariant VeQItemChildModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole &&  index.column() == mSortValueColumn)
		role = SortValueRole;

	if (role == SortValueRole) {
		if (!index.isValid() || index.row() >= mSortDelegates.count())
			return QVariant();
		VeQItemSortDelegate *delegate = mSortDelegates[index.row()];
		if (!delegate)
			return QVariant();
		return delegate->sortValue();
	}

	if (role == BuddyRole) {
		QVariant var = mTableModel->data(mTableModel->index(index.row(), 0), VeQItemTableModel::ItemRole);
		VeQItem *buddy = qvariant_cast<VeQItem *>(var);
		return QVariant::fromValue(buddy);
	}

	return VeQItemTableModel::data(index, role);
}

void VeQItemChildModel::setSourceModel(QAbstractItemModel *model)
{
	if (mTableModel == model)
		return;

	if (mTableModel) {
		mTableModel->disconnect(this);
		clear();
	}

	mTableModel = model;

	if (mTableModel) {

		for (int n = 0; n < mTableModel->rowCount(); n++)
			addOneChild(mTableModel->index(n, 0));

		connect(mTableModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onRowsInserted(QModelIndex,int,int)));
		connect(mTableModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onRowsRemoved(QModelIndex,int,int)));
		connect(mTableModel, SIGNAL(modelReset()), SLOT(onModelReset()));
		connect(mTableModel, SIGNAL(layoutAboutToBeChanged()), SLOT(onLayoutAboutToBeChanged()));
		connect(mTableModel, SIGNAL(layoutChanged()), SLOT(onLayoutChanged()));
		connect(mTableModel, SIGNAL(destroyed()), this, SLOT(onSourceModelDestroyed()));
	}

	emit sourceModelChanged();
}

void VeQItemChildModel::setChildId(const QString &childId)
{
	mChildId = childId;
	if (mTableModel)
		onModelReset();
}

void VeQItemChildModel::setSortDelegate(QDeclarativeComponent *delegate) {
	if (mSortDelegate == delegate)
		return;

	mSortDelegate = delegate;

	qDeleteAll(mSortDelegates);
	for (int n = 0; n < mVector.count(); n++)
		createSortDelegate(n);

	emit sortDelegateChanged();
}

void VeQItemChildModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	for (int n = first; n <= last; n++) {
		QModelIndex index = mTableModel->index(n, 0, parent);
		addOneChild(index);
	}
}

void VeQItemChildModel::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	for (int n = last; n >= first; n--)
		remove(n);
}

void VeQItemChildModel::onModelReset()
{
	beginResetModel();
	rebuild();
	endResetModel();
}

void VeQItemChildModel::onLayoutAboutToBeChanged()
{
	emit layoutAboutToBeChanged();

	// If persistant indexes are used, the old indices should be stored here.
	// For example as a map of the current index, and a persistent index of
	// the proxy model..
	if (!persistentIndexList().empty())
		qWarning() << "the VeQItemChildModel does not support persistent indexes!";
}

void VeQItemChildModel::onLayoutChanged()
{
	rebuild();

	// For persistant index support, it should be figured out here what went where,
	// and the model should be updated with changePersistentIndex(). It is not
	// implemented, since qml doesn't use persistant indexes.

	emit layoutChanged();
}

void VeQItemChildModel::onSourceModelDestroyed()
{
	setSourceModel(nullptr);
}

void VeQItemChildModel::onSortDelegateValueChanged()
{
	VeQItemSortDelegate *delegate = qobject_cast<VeQItemSortDelegate *>(sender());

	if (!delegate)
		return;

	int n = mSortDelegates.indexOf(delegate);
	if (n >= 0) {
		QModelIndex from = createIndex(n, mSortValueColumn);
		QModelIndex till = createIndex(n, mSortValueColumn);
		emit dataChanged(from, till);
	}
}

void VeQItemChildModel::clear()
{
	qDeleteAll(mSortDelegates);
	VeQItemTableModel::clear();
}

void VeQItemChildModel::doRemove(int n)
{
	if (n < mSortDelegates.count()) {
		VeQItemSortDelegate *delegate = mSortDelegates[n];
		mSortDelegates.remove(n);
		delete delegate;
	}
	VeQItemTableModel::doRemove(n);
}

void VeQItemChildModel::doInsertItem(VeQItem *item, int row)
{
	VeQItemTableModel::doInsertItem(item, row);
	createSortDelegate(row);
}

void VeQItemChildModel::createSortDelegate(int n)
{
	if (!mSortDelegate)
		return;

	QDeclarativeContext *context = new QDeclarativeContext(mSortDelegate->creationContext());
	context->setContextProperty("item", mVector[n]);

	QVariant var = mTableModel->data(mTableModel->index(n, 0), VeQItemTableModel::ItemRole);
	VeQItem *buddy = qvariant_cast<VeQItem *>(var);
	context->setContextProperty("buddy", buddy);

	QObject *object = mSortDelegate->create(context);
	context->setParent(object);
	VeQItemSortDelegate *delegate = qobject_cast<VeQItemSortDelegate*>(object);

	// NOTE: nullptrs are inserted as well, to keep the vectors equal in length!
	mSortDelegates.insert(n, delegate);

	if (!delegate) {
		delete context;
		qWarning() << "the sort delegate must be a VeQItemSortDelegate";
		return;
	}

	connect(delegate, SIGNAL(valueChanged()), SLOT(onSortDelegateValueChanged()));
}

void VeQItemChildModel::addOneChild(const QModelIndex &index)
{
	QVariant var = mTableModel->data(index, VeQItemTableModel::ItemRole);
	VeQItem *parent = qvariant_cast<VeQItem *>(var);

	if (parent) {
		VeQItem *item = (mChildId.isEmpty() ? parent : parent->itemGetOrCreate(mChildId));
		setupValueChanges(item, NoOptions, index.row()); // note also inserts the item!
	} else {
		qWarning() << "The model must have a valid ItemRole";
	}
}

void VeQItemChildModel::rebuild()
{
	clear();
	for (int n = 0; n < mTableModel->rowCount(); n++)
		addOneChild(mTableModel->index(n, 0));
}
