#pragma once

#include <QObject>
#include <QString>

class AlternatorError : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE QString description(QString errorId) { return AlternatorError::getDescription(errorId); }

	AlternatorError() {}

	static QString getDescription(QString errorId);
};
