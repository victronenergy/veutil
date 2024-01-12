#pragma once

#include <QObject>
#include <QtQmlIntegration>

typedef struct
{
	int errorId;
	const char *description;
} VeChargerError;

class ChargerError : public QObject
{
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

public:

	Q_INVOKABLE QString description(int error) { return ChargerError::getDescription(error); }

	ChargerError();

	static QString getDescription(int error);
	static bool isWarning(int error);

private:
	static const VeChargerError errors[];
};
