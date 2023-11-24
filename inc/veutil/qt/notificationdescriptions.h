#ifndef NOTIFICATIONDESCRIPTIONS_H
#define NOTIFICATIONDESCRIPTIONS_H

#include <QVariant>
#include <QObject>
#include <QHash>

class NotificationDescriptions : public QObject {
	Q_OBJECT

public:
	typedef QHash<QString, QString> AlarmDescription;

	enum Type {
		WARNING,
		ALARM,
		NOTIFICATION
	};

	NotificationDescriptions();
	Q_INVOKABLE QString getDescription(QString serviceName, QString trigger, QVariant value);
	Q_INVOKABLE QString getTypeString(int type) const;

	AlarmDescription mBatteryAlarms;
	AlarmDescription mDcMeterAlarms;
	AlarmDescription mAlternatorAlarms;
	AlarmDescription mVebusAlarms;
	AlarmDescription mMultiRsAlarms;
	AlarmDescription mSolarChargerAlarms;
	AlarmDescription mAcChargerAlarms;
	AlarmDescription mInverterAlarms;
	AlarmDescription mSystemCalcAlarms;
	AlarmDescription mGeneratorStartStopAlarms;
	AlarmDescription mDigitalInputAlarms;
	AlarmDescription mVecanAlarms;
	AlarmDescription mEssAlarms;
	AlarmDescription mTankAlarms;
	AlarmDescription mPlatformAlarms;

	void addBatteryAlarms();
	void addDcMeterAlarms();
	void addAlternatorAlarms();
	void addVebusAlarms();
	void addMultiRsAlarms();
	void addSolarChargerAlarms();
	void addAcChargerAlarms();
	void addInverterAlarms();
	void addSystemCalcAlarms();
	void addGeneratorStartStopAlarms();
	void addDigitalInputAlarms();
	void addVecanAlarms();
	void addEssAlarms();
	void addTankAlarms();
	void addPlatformAlarms();

	QString getAlternatorAlarmDescription(QString trigger, QVariant value);
	QString getVebusAlarmDescription(QString trigger, QVariant value);
	QString getMultiRsAlarmDescription(QString trigger, QVariant value);
	QString getSolarChagerAlarmDescription(QString trigger, QVariant value);
	QString getAcChargerAlarmDescription(QString trigger, QVariant value);
	QString getBatteryAlarmDescription(QString trigger, QVariant value);

	Q_INVOKABLE QString getAlternatorErrorDescription(int error);
	Q_INVOKABLE QString getVebusErrorDescription(int error);
	Q_INVOKABLE QString getChargerErrorDescription(int error);
	Q_INVOKABLE QString getBmsErrorDescription(int error);
	QString getDescriptionFromTrigger(QString trigger, AlarmDescription descriptions);
};

#endif // NOTIFICATIONDESCRIPTIONS_H
