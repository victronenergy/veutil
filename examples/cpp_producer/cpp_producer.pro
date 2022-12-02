QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include("../../veutil.pri")

SOURCES += \
    main.cpp \
    setting_loading_example_gui.cpp \
    ve_qitems_example.cpp \

HEADERS  += \
    setting_loading_example_gui.h \
    ve_qitems_example.hpp \

FORMS += setting_loading_example_gui.ui
