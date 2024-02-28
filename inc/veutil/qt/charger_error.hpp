#pragma once

#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

typedef struct
{
	int errorId;
	const char *description;
} VeChargerError;

class ChargerError : public QObject
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
	QML_SINGLETON
#endif

public:

	Q_INVOKABLE QString description(int error) { return ChargerError::getDescription(error); }

	ChargerError();

	static QString getDescription(int error);
	static bool isWarning(int error);

private:
	static const VeChargerError errors[];
};
