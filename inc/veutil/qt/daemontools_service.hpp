#pragma once

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QStringList>

#include <veutil/qt/ve_qitem.hpp>

class DaemonToolsService : public QObject
{
	Q_OBJECT

public:
	DaemonToolsService(const QString &servicePath, QObject *parent = 0);

	DaemonToolsService(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath,
									   QObject *parent = 0, QStringList const &sveCtlArgs = QStringList());

	DaemonToolsService(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath, int startValue, QObject *parent = 0);

	DaemonToolsService(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath,
									   QList<int> &startValues, QObject *parent = 0, bool restart = true,
									   QStringList const &sveCtlArgs = QStringList());

	// Listens to multiple ObjectPaths. If one of them is True, the service will be started. By default,
	// everytime a value becomes True or False, the service will be restarted. Unless none of them is True
	// any-more, then the service will be stopped.
	DaemonToolsService(VeQItemSettings *settings, const QString &servicePath,
									   const QList<QString> &dbusObjectPathList, QObject *parent = 0, bool restart = true,
									   QStringList const &sveCtlArgs = QStringList());

	//Same as above with specific start value
	DaemonToolsService(VeQItemSettings *settings, const QString &servicePath,
					const QList<QString> &dbusObjectPathList, int startValue, QObject *parent = 0, bool restart = true);

	void enable();
	void disable();
	void restart();

	// enable down-ed daemons but keep the down files
	void start();
	void stop();

	void waitTillDown();
	bool isUp();

	void install();
	void remove();

	void setSveCtlArgs(const QStringList &args) { mSveCtlArgs = args; }

protected:
	enum SceCtlAction {
			Idle,
			Installing,
			Removing
	};

	virtual void beforeStarted() {}
	virtual void afterStopped() {}
	void execCmd(QString cmd, QStringList arguments);
	void execSvc(QStringList arguments);
	void touch(QString fileName);
	void init();
	void initSettings(VeQItemSettings *settings);
	void spawnSveCtl(const QStringList &args);

	const QString mServicePath;
	QList<VeQItem*> mDbusItemList;
	QList<int> mStartValues;
	QList<QString> mDbusObjectPathList;

	// Arguments needed to install a service, e.g. -s type -D DEV=n
	QStringList mSveCtlArgs;

protected slots:
	virtual void dbusItemChanged();
	void onSveCtlFinished(int exitCode);

private:
	SceCtlAction mSceCtlAction = Idle;
	bool mIsInstalled;
	bool mShouldBeInstalled;
	bool mRestart;
};

// The console port is shared with ve.direct 1, so has some additional start / stop dependencies
class DeamonToolsConsole : public DaemonToolsService
{
	Q_OBJECT

public:
	DeamonToolsConsole(VeQItemSettings *settings, const QString &servicePath, const QString &dbusObjectPath,
									   QObject *parent = 0, QStringList const &sveCtlArgs = QStringList()) :
			DaemonToolsService(settings, servicePath, dbusObjectPath, parent, sveCtlArgs)
	{
	}

protected:
	void beforeStarted() override {
			qDebug() << "[console] Stopping services on ttyconsole";

			// prevent serial starter from using the console
			execCmd("/opt/victronenergy/serial-starter/stop-tty.sh",
					QStringList() << "ttyconsole");
	}

	void afterStopped() override {
			qDebug() << "[console] Starting services on ttyconsole";

			execCmd("/opt/victronenergy/serial-starter/start-tty.sh",
					QStringList() << "ttyconsole");
	}
};

