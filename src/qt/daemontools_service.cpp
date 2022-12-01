#include <QDir>
#include <QFile>
#include <QProcess>

#include <veutil/qt/daemontools_service.hpp>

/*
 * Initially the services were rather static, roughly one service per onboard connection
 * and the standard services. Thereafter came many conditional services depending on
 * settings / available CAN-bus ports etc. These services were already present in the rootfs,
 * but down by default and conditionally upped. By now, many of the services are
 * conditional and since the rootfs became readonly they live in RAM now as well.
 * (although can be done differently as well, but as is, not used servises waste RAM).
 *
 * A new option is therefore added to install a service in the service dir only when
 * needed. This is done by a script, svectl, so it can be used outside of Qt as well.
 * The script can replace certain parameters from a template service before installing.
 * NOTE: by default the script will not wait for the service to be supervised, so calling
 * svc directly after installing will fail. pass -w to wait for it to be supervised.
 * Since conditionally installed services don't need to be down, it is not needed to
 * call cvs -u on it though. There is also no need to down the service, it will be done
 * by the script when removing it.
 *
 * Since this option is added later, some service do the add/remove while others still
 * do the up/down version.
 *
 * installService / removeService don't wait for the script to finish, after returning
 * from these the service might still be there / not be installed yet. A blocking version
 * or signal can be added, but at the moment there is no need for that. They can be called
 * unconditially, the last one wins.
 */

DaemonToolsService::DaemonToolsService(const QString &servicePath, QObject *parent) :
	QObject(parent),
	mServicePath(servicePath),
	mRestart(true)
{
	mStartValues.append(1);
	init();
}

DaemonToolsService::DaemonToolsService(VeQItemSettings *settings, const QString &servicePath,
				 const QList<QString> &dbusObjectPathList, QObject *parent, bool restart, const QStringList &sveCtlArgs) :
	QObject(parent),
	mServicePath(servicePath),
	mDbusObjectPathList(dbusObjectPathList),
	mSveCtlArgs(sveCtlArgs),
	mRestart(restart)

{
	mStartValues.append(1);
	init();
	initSettings(settings);
}

DaemonToolsService::DaemonToolsService(VeQItemSettings *settings, const QString &servicePath,
				 const QList<QString> &dbusObjectPathList, int startValue, QObject *parent, bool restart) :
	QObject(parent),
	mServicePath(servicePath),
	mDbusObjectPathList(dbusObjectPathList),
	mRestart(restart)

{
	mStartValues.append(startValue);
	init();
	initSettings(settings);
}

DaemonToolsService::DaemonToolsService(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath,
									   QObject *parent, const QStringList &sveCtlArgs) :
	QObject(parent),
	mServicePath(servicePath),
	mSveCtlArgs(sveCtlArgs),
	mRestart(true)
{
	mStartValues.append(1);
	mDbusObjectPathList.append(dbusObjectPath);

	init();
	initSettings(settings);
}

DaemonToolsService::DaemonToolsService(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath, int startValue, QObject *parent):
	QObject(parent),
	mServicePath(servicePath),
	mRestart(true)
{
	mStartValues.append(startValue);
	mDbusObjectPathList.append(dbusObjectPath);

	init();
	initSettings(settings);
}

DaemonToolsService::DaemonToolsService(VeQItemSettings *settings, const QString &servicePath,
									   const QString &dbusObjectPath, QList<int> &startValues,
									   QObject *parent, bool restart, QStringList const &sveCtlArgs):
	QObject(parent),
	mServicePath(servicePath),
	mStartValues(startValues),
	mSveCtlArgs(sveCtlArgs),
	mRestart(restart)
{
	mDbusObjectPathList.append(dbusObjectPath);
	init();
	initSettings(settings);
}

void DaemonToolsService::init()
{
	mIsInstalled = mShouldBeInstalled = QDir(mServicePath).exists();
}

void DaemonToolsService::initSettings(VeQItemSettings *settings)
{
	for (QString &path : mDbusObjectPathList) {
		VeQItem *v = settings->root()->itemGetOrCreate(path);
		mDbusItemList.append(v);
	}
	// NOTE! since this is called from the constructor, use a queued connection here, so
	// the object is completely constructed. Otherwise virtual methods will not be call
	// the overridden one!
	for (VeQItem *item: mDbusItemList)
		item->getValueAndChanges(this, SLOT(dbusItemChanged()), true, true);
}

void DaemonToolsService::enable()
{
	if (isUp()) {
		// Multiple objects paths or values control this service. One
		// of them was changed: restart the service.
		if ((mDbusObjectPathList.count() > 1 || mStartValues.count() > 1)
			&& mRestart)
			restart();
		return;
	}

	beforeStarted();
	start();
}

void DaemonToolsService::disable()
{
	if (!isUp())
		return;
	stop();
	afterStopped();
}

void DaemonToolsService::restart()
{
	qDebug() << "[Service] Restart" << mServicePath;

	execSvc(QStringList() << "-t" << mServicePath);
}

void DaemonToolsService::start()
{
	qDebug() << "[Service] up" << mServicePath;

	execSvc(QStringList() << "-u" << mServicePath << mServicePath + "/log");
}

void DaemonToolsService::stop()
{
	qDebug() << "[Service] down" << mServicePath;

	execSvc(QStringList() << "-d" << mServicePath << mServicePath + "/log");
}

void DaemonToolsService::dbusItemChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	QVariant var = item->getValue();

	qDebug() << "[Service] DBus service" << mServicePath << "changed:" << item->id() << "-" << var.toString();

	// If one of the services is enabled, start the service.
	QList<VeQItem*>::iterator i;
	bool doEnable = false;
	for (i = mDbusItemList.begin(); i != mDbusItemList.end(); ++i) {
		QVariant value = (*i)->getValue();

		if (value.isValid()) {
			if (mStartValues.contains(value.toInt()))
				doEnable = true;
		} else {
			return;
		}
	}

	// If arguments for svectrl are given, use that to install / remove the service...
	if (!mSveCtlArgs.empty()) {
		if (doEnable)
			install();
		else
			remove();

		return;
	}

	// Otherwise enable / disable it.
	if (doEnable)
		enable();
	else
		disable();
}

void DaemonToolsService::touch(QString fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadWrite);
	file.close();
}

void DaemonToolsService::execCmd(QString cmd, QStringList arguments)
{
	QProcess proc;
	proc.start(cmd, arguments);
	proc.waitForFinished();
}

void DaemonToolsService::execSvc(QStringList arguments)
{
	if (!QDir(mServicePath).exists())
		return;

	execCmd("svc", arguments);
}

// Not the most polite call since it blocks, but only used when changing CAN-bus profiles
// to make sure the interface is not used before changing bitrate. The service is typically
// down the first time it is checked, so this shouldnt take long.
void DaemonToolsService::waitTillDown()
{
	if (!QDir(mServicePath).exists())
		return;

	for (;;) {
		QProcess proc;
		proc.start("svstat", QStringList() << mServicePath);
		proc.waitForFinished();
		QString output(proc.readAllStandardOutput());
		if (output.startsWith(mServicePath + ": down "))
			break;
	}
}

bool DaemonToolsService::isUp()
{
	QProcess proc;
	proc.start("svstat", QStringList() << mServicePath);
	proc.waitForFinished();
	QString output(proc.readAllStandardOutput());
	return output.startsWith(mServicePath + ": up ") && !output.contains(", want down");
}

void DaemonToolsService::onSveCtlFinished(int exitCode)
{
	if (exitCode == 0) {
		if (mSceCtlAction == Installing) {
			//qDebug() << "[Service] installed" << mServicePath;
			mSceCtlAction = Idle;
			mIsInstalled = true;
			// Since svectl is called async a request to remove the service can have
			// happened, while it was being installed.
			if (!mShouldBeInstalled)
				remove();
		} else if (mSceCtlAction == Removing) {
			//qDebug() << "[Service] removed" << mServicePath;
			mSceCtlAction = Idle;
			mIsInstalled = false;
			if (mShouldBeInstalled)
				install();
		} else {
			qDebug() << "[Service] msvectl: currupt state %d %s" << mSceCtlAction << mServicePath;
			mSceCtlAction = Idle;
			mIsInstalled = mShouldBeInstalled = QDir(mServicePath).exists();
		}
	} else {
		// error, shouldn't occur.
		qCritical("error occured while running svectl");
		QProcess *proc = static_cast<QProcess *>(sender());
		qDebug() << proc->readAllStandardError();

		mSceCtlAction = Idle;
		mIsInstalled = mShouldBeInstalled = QDir(mServicePath).exists();
	}
}

void DaemonToolsService::spawnSveCtl(QStringList const &args)
{
	// Mind it: the QProcess exists longer then this object in case of remove
	// followed by free. So let it free itself. This class its slot will be
	// disconnected in the desctructor and the free will remain connected.
	QProcess *proc = new QProcess();
	connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));
	connect(proc, SIGNAL(finished(int)), SLOT(onSveCtlFinished(int)));
	proc->start("/opt/victronenergy/venus-platform/svectl", args);
}

void DaemonToolsService::install()
{
	// Don't run multiple svectl against the same service, it is undefined what will happen.
	// Just make sure the last one wins.
	mShouldBeInstalled = true;
	if (mSceCtlAction != Idle)
		return;

	if (mIsInstalled) {
		if (mRestart)
			restart();
		return;
	}

	beforeStarted();
	qDebug() << "[Service] installing" << mServicePath;
	mSceCtlAction = Installing;
	spawnSveCtl(mSveCtlArgs);
}

void DaemonToolsService::remove()
{
	mShouldBeInstalled = false;
	if (mSceCtlAction != Idle)
		return;

	qDebug() << "[Service] removing" << mServicePath;
	mSceCtlAction = Removing;
	spawnSveCtl(mSveCtlArgs + QStringList() << "-r");
	afterStopped();
}
