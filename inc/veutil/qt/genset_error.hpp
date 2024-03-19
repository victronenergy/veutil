#pragma once

#include <QObject>
#include <QString>

class GensetError : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString description(QString errorId, int nrOfPhases) { return GensetError::getDescription(errorId, nrOfPhases); }

	GensetError() = default;

	static QString getDescription(QString errorId, int nrOfPhases);
};
