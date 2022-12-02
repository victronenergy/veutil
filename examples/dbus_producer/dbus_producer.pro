QT = core gui dbus xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include("../../veutil.pri")

SOURCES += \
    main.cpp \
    mainwindow.cpp \

HEADERS  += \
    mainwindow.h \

FORMS += \
    mainwindow.ui
