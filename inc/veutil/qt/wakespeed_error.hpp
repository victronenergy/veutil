#pragma once

#include <QObject>
#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

class WakespeedError : public QObject
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
	QML_SINGLETON
#endif

public:
	Q_INVOKABLE QString description(int error) { return WakespeedError::getDescription(error); }

	WakespeedError() {}

	static const int FailureMask = 0x8000;

	static QString getDescription(int errorNumber);
	static bool isWarning(int errorNumber) { return (errorNumber & FailureMask) == 0; }
};
