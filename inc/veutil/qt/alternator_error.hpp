#pragma once

#include <QObject>
#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif

class AlternatorError : public QObject
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
       QML_ELEMENT
       QML_SINGLETON
#endif

public:
	Q_INVOKABLE QString description(QString errorId) { return AlternatorError::getDescription(errorId); }

	AlternatorError() {}

	static QString getDescription(QString errorId);
};
