#pragma once

#include <libudev.h>

#include <QDebug>
#include <QObject>
#include <QList>
#include <QSocketNotifier>

bool isCanBusInterface(char const *interface);

/**
 * A bare implementation for libudev in Qt. Enough so it can be used in the
 * qt mainloop, enumerate devices and have a signal for udev changes.
 *
 * It simply makes the c types available instead of wrapping everything in
 * c++ classes.
 *
 * For completeness, this is not portable code, but rather Linux specific.
 */

// Just a helper which will free the devices automatically.
// The devices should be inserted while holding a reference.
class QUdevList : public QList<udev_device *>
{
public:
	~QUdevList() {
		while (!isEmpty()) {
			auto dev = takeLast();
			udev_device_unref(dev);
		}
	}
};

// The instance of the udev api. Apparently there can be more than one, but
// since normally there is only one needed, it is also available as a singleton.
// In a multi-threaded case more than one is likely needed though.
class QUdev : public QObject
{
	Q_OBJECT

public:
	explicit QUdev(QObject *parent = nullptr) : QObject(parent) { mUdev = udev_new(); }
	~QUdev() { udev_unref(mUdev); }

	static QUdev *instance() {
		static QUdev *mInstance = new QUdev();
		return mInstance;
	}

	struct udev *udev() const { return mUdev; }
	QUdevList enumerate(char const *subsystem) const;
	QUdevList scan(struct udev_enumerate *enumerate) const;
	static void dumpDevice(struct udev_device *dev);

private:
	struct udev *mUdev = nullptr;
};

// Translated from a string, so numerical comparisons can be used.
enum class QUdevAction {
	None,
	Add,
	Remove,
	Move,
	Change,
	Online,
	Offline,
	Unknown,
};

// Receive udev events (that is after udev rules are applied). Note: if no filter rule
// is set all udev events are received, which is likely not intended. More then one
// udev_monitor can be connected to a single udev instance.
class QUdevMonitor : public QObject {
	Q_OBJECT

public:
	QUdevMonitor(QUdev *qudev);
	~QUdevMonitor();

	bool enableReceiving();

	struct udev_monitor *monitor() const { return mMonitor; }

	// Since this is the most common, it is added as a c++ function.
	// More advanced filters can be created with the c functions.
	// The monitor member simply makes the c struct available.
	int filterAddMatchSubsytemDevtype(const char *subsystem, const char *devtype = nullptr) {
		return udev_monitor_filter_add_match_subsystem_devtype(monitor(), subsystem, devtype);
	}

	static QUdevAction getUdevDeviceAction(struct udev_device *dev);

signals:
	void udevEvent(QUdevAction action, struct udev_device *dev);

private slots:
	void handleUdevReceive();

private:
	struct udev_monitor *mMonitor;
	QSocketNotifier *mNotifier = nullptr;
};

// Example of enumeration and udev events.
class QUdevExample : public QObject {
	Q_OBJECT

public:
	QUdevExample(QObject *parent = nullptr) : QObject(parent)
	{
		mMonitor = new QUdevMonitor(QUdev::instance());
	}

	void enumerateTtys() const
	{
		QUdevList list = QUdev::instance()->enumerate("tty");
		for (const auto &dev: list) {
			qDebug() << "=====================================";
			QUdev::dumpDevice(dev);
		}
	}

	void enableReceiving()
	{
		connect(mMonitor, SIGNAL(udevEvent(QUdevAction, struct udev_device *)), SLOT(onUdevEvent(QUdevAction, struct udev_device *)));
		mMonitor->filterAddMatchSubsytemDevtype("net");
		mMonitor->filterAddMatchSubsytemDevtype("tty");
		mMonitor->enableReceiving();
	}

public slots:
	void onUdevEvent(QUdevAction action, struct udev_device *dev) const
	{
		qDebug() << "=======" << udev_device_get_action(dev) << (int) action << "=============";
		QUdev::dumpDevice(dev);
	}

private:
	QUdevMonitor *mMonitor;
};
