#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration>

#include <veutil/ve_regs_payload.h>

class BmsError : public QObject
{
	Q_OBJECT
	QML_ELEMENT
	QML_SINGLETON

public:
	Q_INVOKABLE QString description(int error) { return BmsError::getDescription(error); }

	BmsError() {}
	static QString getDescription(int errorNumber);
};
