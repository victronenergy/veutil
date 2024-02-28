#pragma once

#include <QObject>
#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

#include <veutil/ve_regs_payload.h>

class BmsError : public QObject
{
	Q_OBJECT

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_ELEMENT
	QML_SINGLETON
#endif

public:
	Q_INVOKABLE QString description(int error) { return BmsError::getDescription(error); }

	BmsError() {}
	static QString getDescription(int errorNumber);
};
