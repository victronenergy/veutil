#include <QFile>
#include <QProcess>

#include <veutil/qt/canbus_interfaces.hpp>

CanBusProfile::CanBusProfile(int bitrate, QObject *parent) :
	QObject(parent),
	mBitRate(bitrate)
{
}

void CanBusProfile::enableProfile() const
{
	for (CanBusService *service: mServices) {
		service->setProfileEnabled(true);
		service->checkStart();
	}
}

void CanBusProfile::disableProfile() const
{
	for (CanBusService *service: mServices) {
		service->setProfileEnabled(false);
		service->remove();
	}
}

CanBusProfiles::CanBusProfiles(VeQItemSettings *settings, VeQItem *service, QString interface,
							   CanBusProfile::CanProfile defaultProfile, CanBusConfig config,
							   QObject *parent) :
	QObject(parent),
	mActiveProfile(0),
	mInterface(interface),
	mConfig(config)
{
	CanBusService *vecan = new CanBusService("vecan-dbus", interface, this);
	CanBusService *canBusBms = new CanBusService("can-bus-bms", interface, this);
	CanBusService *valence = new CanBusService("dbus-valence", interface, this);
	CanBusService *motordrive = new CanBusService("dbus-motordrive", interface, this);
	CanBusService *rvc = new CanBusService("dbus-rv-c", interface, this);
	CanBusService *hvCanBus = new CanBusService("hv-can-bus", interface, this);

	// Disabled profile, without any service
	CanBusProfile *profile = new CanBusProfile(0, this);
	mProfiles.append(profile);

	profile = new CanBusProfile(250000, this);
	profile->addService(vecan);
	mProfiles.append(profile);

	profile = new CanBusProfile(250000, this);
	profile->addService(vecan);
	profile->addService(canBusBms);
	mProfiles.append(profile);

	profile = new CanBusProfile(500000, this);
	profile->addService(canBusBms);
	mProfiles.append(profile);

	profile = new CanBusProfile(250000, this);
	profile->addService(vecan);
	profile->addService(valence);
	profile->addService(motordrive);
	mProfiles.append(profile);

	// Debug, bring up the interface (250kbit), but not the services
	profile = new CanBusProfile(250000, this);
	mProfiles.append(profile);

	profile = new CanBusProfile(250000, this);
	profile->addService(rvc);
	mProfiles.append(profile);

	profile = new CanBusProfile(500000, this);
	profile->addService(hvCanBus);
	mProfiles.append(profile);

	// Debug, bring up the interface (500kbit), but not the services
	profile = new CanBusProfile(500000, this);
	mProfiles.append(profile);

	VeQItem *item = settings->add(QString("Canbus/%1/Profile").arg(interface),
								  defaultProfile, 0, mProfiles.count() - 1);
	item->getValueAndChanges(this, SLOT(dbusItemChanged()));

	mInterfaceItem = service->itemGetOrCreate("CanBus/Interface/" + interface);
	mInterfaceItem->itemAddChild("Statistics", new VeQItemCanStats(interface));

	connect(&mTimer, SIGNAL(timeout()), SLOT(onPollTimer()));
	onPollTimer();
	mTimer.start(60000);
}

CanBusProfiles::~CanBusProfiles()
{
	mInterfaceItem->itemDelete();
}

void CanBusProfiles::changeCanBusBitRate(int bitrate)
{
	QProcess proc;

	qDebug() << "[CanBus] Bitrate change for" << mInterface << ":" << QString::number(bitrate);

	QStringList args = QStringList() << mInterface << QString::number(bitrate);
	proc.start("can-set-rate", args);
	proc.waitForFinished();
}

void CanBusProfiles::dbusItemChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	QVariant canProfile = item->getValue();

	if (canProfile.isValid() && canProfile.canConvert<uint>()) {
		int index = canProfile.toUInt();

		if (index < 0 || index >= mProfiles.size())
			return;

		if (mActiveProfile)
			mActiveProfile->disableProfile();

		mActiveProfile = mProfiles[index];
		changeCanBusBitRate(mActiveProfile->bitrate());
		mActiveProfile->enableProfile();
	}
}

void CanBusProfiles::checkSpiStats()
{

	QFile statFile("/sys/class/net/" + mInterface + "/spi_stats");
	if (!statFile.exists() || !statFile.open(QFile::ReadOnly))
		return;
	QTextStream in(&statFile);
	quint64 retries = 0;
	quint64 transfers = 0;
	double retryRatio;
	bool ok;

	for (;;) {
		QString line = in.readLine();
		if (line.isNull())
			break;

		qDebug() << line;
		QStringList parts = line.split(":", Qt::SkipEmptyParts);
		if (parts.length() < 2)
			goto out;

		if (parts[0] == "transfers") {
			transfers = parts[1].toLongLong(&ok);
			if (!ok)
				goto out;
		} else if (parts[0] == "retries") {
			retries = parts[1].toLongLong(&ok);
			if (!ok)
				goto out;
		}
	}

	retryRatio = transfers ? (double) retries * 100 / transfers : 0;
	mInterfaceItem->itemGetOrCreateAndProduce("SpiRetryPercentage", retryRatio);
	return;

out:
	qCritical() << "malformed line in spi_stats";
}

void CanBusProfiles::checkFailed()
{
	QFile failedFile("/sys/class/net/" + mInterface + "/failed");
	if (!failedFile.exists() || !failedFile.open(QFile::ReadOnly))
		return;
	bool failed = failedFile.readLine() == "1";
	mInterfaceItem->itemGetOrCreateAndProduce("Failed", failed);
}

void CanBusProfiles::onPollTimer()
{
	checkSpiStats();
	checkFailed();
}

QVariantMap CanBusProfiles::canInfo() const
{
	QVariantMap ret;

	ret["interface"] = mInterface;
	ret["name"] = getUiName();
	ret["config"] = mConfig;

	return ret;
}

void CanBusService::checkStart()
{
	bool doEnable;

	if (mProfileEnabled) {
		doEnable = true;
		for (QList<VeQItem*>::iterator i = mDbusItemList.begin(); i != mDbusItemList.end(); ++i) {
			QVariant value = (*i)->getValue();

			if (!value.isValid())
				return;

			if (!mStartValues.contains(value.toInt())) {
				doEnable = false;
				break;
			}
		}
	} else {
		doEnable = false;
	}

	if (doEnable) {
		install();
	} else {
		remove();
	}
}

void CanBusService::dbusItemChanged(VeQItem *item, QVariant var)
{
	qDebug() << "[CanService] DBus service" << mServicePath << "changed:" << item->id() << "-" << var.toString();

	checkStart();
}

QVariant VeQItemCanStats::getValue()
{
	QProcess process;

	process.start("ip", QStringList() << "-json" << "-details" << "-statistics" << "link" << "show" << mInterface);
	process.waitForFinished();

	return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}
