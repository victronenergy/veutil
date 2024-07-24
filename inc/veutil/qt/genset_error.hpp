#pragma once

#include <QObject>
#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

class GensetError : public QObject
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
	QML_SINGLETON
#endif

public:
	Q_INVOKABLE QString description(QString errorId, int nrOfPhases) { return GensetError::getDescription(errorId, nrOfPhases); }

	GensetError() = default;

	static QString getDescription(QString errorId, int nrOfPhases);
};
