#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration>

class AlternatorError : public QObject
{
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

public:
	Q_INVOKABLE QString description(QString errorId) { return AlternatorError::getDescription(errorId); }

	AlternatorError() {}

	static QString getDescription(QString errorId);
};
