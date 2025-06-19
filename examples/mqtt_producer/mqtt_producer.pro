TARGET = mqtt_producer
TEMPLATE = app
QT += core network gui widgets qml quick mqtt

SOURCES += main.cpp
RESOURCES += qml.qrc

# uncomment the following for websockets support
DEFINES += MQTT_WEBSOCKETS_ENABLED
QT += websockets

CONFIG += ve-qitems-mqtt
VEUTIL = ../..
INCLUDEPATH += $${VEUTIL}/inc
include($${VEUTIL}/src/qt/veqitem.pri)
include($${VEUTIL}/common.pri)
