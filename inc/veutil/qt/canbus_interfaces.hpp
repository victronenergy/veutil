#pragma once

#include <QObject>
#include <veutil/qt/daemontools_service.hpp>
#include <veutil/qt/ve_qitem_utils.hpp>

class VeQItemCanStats : public VeQItemExportedLeaf
{
public:
	explicit VeQItemCanStats(QString const &interface) :
		VeQItemExportedLeaf(),
		mInterface(interface)
	{}

	QVariant getValue() override;

private:
	QString mInterface;
};


class CanBusService : public DaemonToolsService
{
	Q_OBJECT

public:
	CanBusService(QString const &kind, QString const &interface, QObject *parent = 0) :
		DaemonToolsService("/service/" + kind + "." + interface, parent),
		mProfileEnabled(false)
	{
		setCanArgs(kind, interface);
	}

	CanBusService(VeQItemSettings *settings, QString const &kind, QString const &interface,
				  const QList<QString> &dbusObjectPathList, QObject *parent = 0) :
		DaemonToolsService("/service/" + kind + "." + interface, parent),
		mProfileEnabled(false)
	{
		mDbusObjectPathList.append(dbusObjectPathList);
		initSettings(settings);
		setCanArgs(kind, interface);
	}

	void setCanArgs(QString const &kind, QString const &interface)
	{
		setSveCtlArgs(QStringList() <<
						"-s" << kind <<
						"-D" << "DEV=" + interface);
	}

	void setProfileEnabled(bool enabled) { mProfileEnabled = enabled; }
	void checkStart();

protected slots:
	virtual void dbusItemChanged(VeQItem *item, QVariant var);

private:
	bool mProfileEnabled;
};

class CanBusProfile : public QObject
{
	Q_OBJECT
	Q_ENUMS(CanProfile)

public:
	enum CanProfile {
		CanProfileDisabled,
		CanProfileVecan,
		CanProfileVecanAndCanBms,
		CanProfileCanBms500,
		CanProfileOceanvolt,
		CanProfileNone250,
		CanProfileRvC,
		CanProfileHighVoltage,
		CanProfileNone500
	};

	CanBusProfile(int bitrate, QObject *parent = 0);
	void addService(CanBusService *service) { mServices.append(service); }

	void enableProfile() const;
	void disableProfile() const;

	int bitrate() const { return mBitRate; }

private:
	QList<CanBusService *> mServices;
	const int mBitRate;
};

class CanBusProfiles : public QObject
{
	Q_OBJECT
	Q_ENUMS(CanBusConfig)

public:
	// Venus config, the CAN-bus might be fixed to a certain protocol and
	// not be configurable by the user.
	enum CanBusConfig {
		// "any" means, any of the for Venus tested protocols, before suppoting the
		// hv-can-bus (CAN-fd). That is classic CAN up to 500 kbit/s.
		CanAnyClassicBus,
		CanForcedCanBusBms,
		CanForcedVeCan,
		// High Voltage CAN can send CAN fd frames as well. Hence CAN interfaces must
		// explicitly indicate it is supported, not only that CAN-fd messages are
		// supported, but also to assure the device can keep up with the higher
		// througput. The HV protocol uses upto 1Mbit/s at the moment.
		CanClassicAndHv,
	};

	CanBusProfiles(VeQItemSettings *settings, VeQItem *service, QString interface,
					   CanBusProfile::CanProfile defaultProfile, CanBusConfig config,
					   QObject *parent = 0);
	~CanBusProfiles();

	void setUiName(const QString &name) { mUiName = name; }
	QString getUiName() const { return mUiName.isEmpty() ? mInterface : mUiName; }

	CanBusConfig getConfig() { return mConfig; }

	QVariantMap canInfo() const;

	// Return the active profile or the disable one, so it can't return 0.
	CanBusProfile *activeProfile() const { return mActiveProfile ? mActiveProfile : mProfiles[0]; }

private slots:
	void dbusItemChanged();

private:
	void changeCanBusBitRate(int speed);

	CanBusProfile *mActiveProfile;
	QVector<CanBusProfile *> mProfiles;
	QString mInterface;
	CanBusConfig mConfig;
	QString mUiName;
	VeQItem *mInterfaceItem;
};
