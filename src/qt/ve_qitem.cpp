#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QStringList>

#include <veutil/qt/ve_qitem.hpp>

VeQItem::VeQItem(VeQItemProducer *producer, QObject *parent) :
	QObject(parent),
	mState(Idle),
	mStateWhilePreviewing(Idle),
	mTextState(Idle),
	mTextStateWhilePreviewing(Idle),
	mProducer(producer),
	mIsLeaf(false),
	mWatched(false),
	mSeen(false)
{
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
VeQItem::VeQItem(VeQItemProducer *producer) :
	VeQItem(producer, nullptr)
{
}
#pragma GCC diagnostic pop

// NOTE: to remove branches, call item->itemDelete() instead of delete item, otherwise
// the signal for the item itself will not be emitted.
VeQItem::~VeQItem()
{
	// Destruct the tree from the childs up, so the parents remain valid during desctruction.
	// The default QObject destructor runs the otherway around and then partially destucted
	// objects are emitting signals.
	forAllChildrenSafe([](VeQItem *child) { child->itemDelete(); });
}

void VeQItem::setParent(QObject *parent)
{
	QObject::setParent(parent);
	foreachParentFirst(this, SLOT(resetId(VeQItem *, void *)), 0);
}

void VeQItem::getValueAndChanges(QObject *obj, const char *member, bool fetch, bool queued)
{
	connect(this, SIGNAL(valueChanged(QVariant)), obj, member);
	updateWatched();
	if (fetch) {
		if (queued)
			qRegisterMetaType<VeQItem *>();

		Qt::ConnectionType type(queued ? Qt::QueuedConnection : Qt::AutoConnection);
		connect(this, SIGNAL(initValue(QVariant)), obj, member, type);
		emit initValue(getValue());
		disconnect(this, SIGNAL(initValue(QVariant)), obj,  member);
	}

	// there is BUG in qt 4, see https://bugreports.qt.io/browse/QTBUG-4844
	// which causes disconnectNotify not to be called when the receiver is deleted.
	// Hence monitor such deletions and explicitly disconnect the receiver upon destruction.
	connect(obj, SIGNAL(destroyed(QObject*)), SLOT(receiverDestroyed(QObject*)));
}

void VeQItem::commitPreview()
{
	setValue(mValue);
}

void VeQItem::discardPreview()
{
	if (mState != Preview)
		return;

	mState = mStateWhilePreviewing;
	mValue = mValueWhilePreviewing;
	mTextState = mTextStateWhilePreviewing;
	mText = mTextWhilePreviewing;
	emit valueChanged(mValue);
	emit stateChanged(mState);
	emit textChanged(mText);
	emit textStateChanged(mTextState);
}

void VeQItem::updateWatched()
{
	bool watched = receivers(SIGNAL(valueChanged(QVariant))) != 0;
	if (mWatched == watched)
		return;
	mWatched = watched;
	watchedChanged();
}

void VeQItem::receiverDestroyed(QObject *obj)
{
	disconnect(obj);
}

void VeQItem::disconnectNotify(const char *signal)
{
	Q_UNUSED(signal);
	updateWatched();
}

/**
 * The mWatched member keeps track if there is a receiver interested in value
 * changes at all. In such cases there is no need to refresh the value continuesly
 * since it will be ignored anyway. The watchedChanged can be overridden by a
 * producer.
 *
 * Warning: This function violates the object-oriented principle of modularity.
 * However, it might be useful when you need to perform expensive initialization
 * only if something is connected to a signal.
 */
void VeQItem::watchedChanged()
{
}

VeQItem *VeQItem::itemChild(int n)
{
	if (n >= mChildren.count())
		return 0;

	// check if this is expensive
	return mChildren[mChildren.keys()[n]];
}

VeQItem *VeQItem::itemAddChild(QString id, VeQItem *item)
{
	item->setId(id);
	item->setParent(this);
	mIsLeaf = false;
	emit childAboutToBeAdded(item);
	mChildren[id] = item;
	emit childAdded(item);
	item->afterAdd();

	return item;
}

void VeQItem::afterAdd()
{
}

void VeQItem::itemDeleteChild(VeQItem *child)
{
	emit childAboutToBeRemoved(child);
	mChildren.remove(child->mId);
	emit childRemoved(child);
	child->deleteLater();
}

// deletes the item and removes it from its parent
void VeQItem::itemDelete()
{
	if (itemParent())
		itemParent()->itemDeleteChild(this);
}

QString VeQItem::getRelId(VeQItem *ancestor)
{
	Q_ASSERT(ancestor != 0);
	if (this == ancestor)
		return "/";
	VeQItem *parent = itemParent();
	if (parent == 0)
		return QString();
	QString p = parent->getRelId(ancestor);
	if (p.isEmpty())
		return p;
	if (!p.endsWith('/'))
		p += '/';
	p += id();
	return p;
}

VeQItem *VeQItem::createChild(QString id, QVariant var)
{
	VeQItem *item = createChild(id);
	item->produceValue(var);
	return item;
}

VeQItem *VeQItem::createChild(QString id, bool isLeaf, bool isTrusted)
{
	if (isTrusted)
		Q_ASSERT(producer());
	else if (!producer())
		return nullptr;

	VeQItem *item = producer()->createItem();
	item->mIsLeaf = isLeaf;
	itemAddChild(id, item);
	return item;
}

void VeQItem::produceValue(QVariant variant, State state, bool forceChanged)
{
	// Stop updating the value from the other side as long as it's previewed.
	// Keep the actual values around though, for the case the preview is discarded.
	if (mState == VeQItem::Preview) {
		mValueWhilePreviewing = variant;
		mStateWhilePreviewing = state;
		return;
	}

	if (state == VeQItem::Preview) {
		// Only allow previews when online / in sync etc.
		if (mState != Synchronized)
			return;

		// Text is normally produced client-side, but when previewing, we have to fake it.
		produceText(variant.toString(), VeQItem::Preview);

		mValueWhilePreviewing = mValue;
		mStateWhilePreviewing = mState;
	}

	bool stateIsChanged = forceChanged || mState != state;
	bool valueIsChanged = forceChanged || mValue != variant;

	mState = state;
	mValue = variant;

	if (mValue.isValid())
		mLastValidValue = mValue;

	if (!mSeen && state == VeQItem::Synchronized) {
		mSeen = true;
		emit seenChanged();
	}

	if (stateIsChanged)
		emit stateChanged(state);
	if (valueIsChanged)
		emit valueChanged(variant);
}

void VeQItem::produceText(QString text, VeQItem::State state)
{
	// Stop updating the value from the other side as long as it's previewed.
	// Keep the actual values around though, for the case the preview is discarded.
	if (mTextState == VeQItem::Preview) {
		mTextWhilePreviewing = text;
		mTextStateWhilePreviewing = state;
		return;
	}

	if (state == VeQItem::Preview) {
		mTextWhilePreviewing = mText;
		mTextStateWhilePreviewing = mTextState;
	}

	bool stateIsChanged = mTextState != state;
	bool textIsChanged = mText != text;

	mTextState = state;
	mText = text;

	if (!mText.isNull())
		mLastValidText = mText;

	if (stateIsChanged)
		emit textStateChanged(state);
	if (textIsChanged)
		emit textChanged(text);
}

QString VeQItem::id()
{
	return mId;
}

void VeQItem::setId(QString id)
{
	Q_ASSERT(parent() == 0);

	mId = id;
	setObjectName(id);
}

QString VeQItem::uniqueId()
{
	if (!mUid.isNull())
		return mUid;

	QString ret;
	uniqueId(ret);
	return ret;
}

void VeQItem::uniqueId(QString &uid)
{
	VeQItem *theParent = qobject_cast<VeQItem *>(parent());
	if (theParent) {
		theParent->uniqueId(uid);
		uid += (uid == "" ? "" : "/") + mId;
	} else {
		uid = mId;
	}
}

void VeQItem::resetId(VeQItem *item, void *ctx)
{
	Q_UNUSED(ctx);
	QString str;
	item->uniqueId(str);
	item->mUid = str;
}

VeQItem *VeQItem::itemGet(QString uid)
{
	VeQItem *item = this;

	// tolerate ids starting with a slash
	if (uid.startsWith("/"))
		uid = uid.mid(1);

	if (uid.isEmpty())
		return this;

	QStringList parts = uid.split('/');
	for (QString const &part: parts) {
		item = item->itemChildren().value(part);
		if (item == 0)
			return 0;
	}
	return item;
}

/*
 * If you know what you are doing this function won't fail. Worse case it
 * will throw a out of memory exception. When doing bad things it will
 * assert / crash and you need to stop doing bad things.
 *
 * Unless isTrusted is set to false, e.g. when accepting string from qml.
 * Obviously you need to check the return value then since it might be a
 * nullptr.
 */
VeQItem *VeQItem::itemGetOrCreate(QString uid, bool isLeaf, bool isTrusted)
{
	VeQItem *item = this;

	// tolerate ids starting with a slash
	if (uid.startsWith("/"))
		uid = uid.mid(1);

	if (uid.isEmpty())
		return this;

	QStringList parts = uid.split('/');
	int n = 1;
	for (QString const &part: parts) {
		VeQItem *child = item->itemChildren().value(part);
		if (child == 0)
			child = item->createChild(part, isLeaf && n == parts.count(), isTrusted);
		item = child;
		if (!isTrusted && item == nullptr) {
			qWarning() << "UID" << uid << "contains invalid part:" << part;
			break;
		}
		n++;
	}
	return item;
}

VeQItem *VeQItem::itemGetOrCreateAndProduce(QString uid, QVariant value)
{
	VeQItem *item = itemGetOrCreate(uid);
	item->produceValue(value);
	return item;
}

VeQItem *VeQItem::itemRoot()
{
	VeQItem *ret = this;
	for (;;) {
		VeQItem *parent = ret->itemParent();
		if (!parent)
			return ret;
		ret = parent;
	}
}

VeQItem *VeQItem::itemParent()
{
	return qobject_cast<VeQItem *>(parent());
}

QVariant VeQItem::itemProperty(const char *name)
{
	if (mPropertyState[name] != Synchronized)
		mPropertyState[name] = Requested;
	return property(name);
}

void VeQItem::itemProduceProperty(const char *name, const QVariant &value, VeQItem::State state)
{
	bool changed = property(name) != value;
	mPropertyState[name] = state;
	setProperty(name, value);
	if (changed)
		emit dynamicPropertyChanged(name, value);
}

// returns the index in the parents it child ids.
int VeQItem::index()
{
	VeQItem *theParent = qobject_cast<VeQItem *>(parent());
	if (!theParent)
		return 0;

	return theParent->itemChildren().keys().indexOf(mId);
}

void VeQItem::foreachChildFirst(VeQItemForeach *each)
{
	for (VeQItem *child: mChildren)
		child->foreachChildFirst(each);
	each->handleItem(this);
}

void VeQItem::foreachChildFirst(QObject *obj, const char *member, void *ctx)
{
	VeQItemForeach each(obj, member, ctx);
	foreachChildFirst(&each);
}

void VeQItem::foreachChildFirst(std::function<void(VeQItem *)> const &f)
{
	for (VeQItem *child: mChildren)
		child->foreachChildFirst(f);
	f(this);
}

// Loop over a copy, so the item itself can be removed.
void VeQItem::foreachChildFirstSafe(const std::function<void (VeQItem *)> &f)
{
	Children copy = mChildren;
	for (VeQItem *child: copy)
		child->foreachChildFirstSafe(f);
	f(this);
}

void VeQItem::forAllChildren(std::function<void(VeQItem *)> const &f)
{
	for (VeQItem *child: mChildren)
		child->foreachChildFirst(f);
}

// Loop over a copy, so the item itself can be removed.
void VeQItem::forAllChildrenSafe(const std::function<void (VeQItem *)> &f)
{
	Children copy = mChildren;
	for (VeQItem *child: copy)
		child->foreachChildFirstSafe(f);
}

void VeQItem::foreachParentFirst(VeQItemForeach *each)
{
	each->handleItem(this);
	for (VeQItem *child: mChildren)
		child->foreachParentFirst(each);
}

void VeQItem::foreachParentFirst(QObject *obj, const char *member, void *ctx)
{
	VeQItemForeach each(obj, member, ctx);
	foreachParentFirst(&each);
}

void VeQItem::foreachParentFirst(std::function<void(VeQItem *)> const & f)
{
	f(this);
	foreach (VeQItem *child, mChildren)
		child->foreachParentFirst(f);
}

void VeQItem::setState(VeQItem::State state)
{
	if (mState == state)
		return;
	mState = state;
	emit stateChanged(state);
}

void VeQItem::setTextState(VeQItem::State state)
{
	if (mTextState == state)
		return;
	mTextState = state;
	emit textStateChanged(state);
}

VeQItem *VeQItems::getRoot()
{
	static VeQItem theRoot(0);
	return &theRoot;
}

VeQItemProxy::VeQItemProxy(VeQItem *srcItem) :
	VeQItem(nullptr), // NOTE: children cannot be created!
	mSrcItem(srcItem)
{
	mIsLeaf = true;
	connect(mSrcItem, SIGNAL(valueChanged(QVariant)), SIGNAL(valueChanged(QVariant)));
	connect(mSrcItem, SIGNAL(textChanged(QString)), SIGNAL(textChanged(QString)));
	connect(mSrcItem, SIGNAL(dynamicPropertyChanged(char const *,QVariant)),
			SIGNAL(dynamicPropertyChanged(char const *,QVariant)));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
VeQItemLocal::VeQItemLocal(VeQItemProducer *producer, QObject *parent) :
	VeQItem(producer, parent)
{}

VeQItemLocal::VeQItemLocal(VeQItemProducer *producer) :
	VeQItemLocal(producer, nullptr)
{}
#pragma GCC diagnostic pop
