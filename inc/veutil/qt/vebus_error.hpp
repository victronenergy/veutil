#pragma once

#include <QObject>
#include <QString>

class VebusError : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE static QString getDescription(int errorNumber);
};
