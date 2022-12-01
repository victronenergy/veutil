#pragma once

#include <QObject>
#include <QString>

class WakespeedError : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString description(int error) { return WakespeedError::getDescription(error); }

	WakespeedError() {}

	static const int FailureMask = 0x8000;

	static QString getDescription(int errorNumber);
	static bool isWarning(int errorNumber) { return (errorNumber & FailureMask) == 0; }
};
