VE_UTIL_INC = "$$PWD/../../inc/veutil"

SOURCES += \
    $$PWD/unit_conversion.cpp \
    $$PWD/ve_qitem.cpp \
    $$PWD/ve_qitem_loader.cpp \
    $$PWD/ve_qitem_table_model.cpp \
    $$PWD/ve_qitem_tree_model.cpp \

HEADERS += \
    $$VE_UTIL_INC/qt/unit_conversion.hpp \
    $$VE_UTIL_INC/qt/ve_qitem.hpp \
    $$VE_UTIL_INC/qt/ve_qitem_loader.hpp \
    $$VE_UTIL_INC/qt/ve_qitem_table_model.hpp \
    $$VE_UTIL_INC/qt/ve_qitem_tree_model.hpp \
    $$VE_UTIL_INC/qt/ve_qitem_utils.hpp \

contains(QT, dbus|qdbus) {
    SOURCES += \
        $$PWD/ve_qitems_dbus.cpp \
        $$PWD/ve_qitem_exported_dbus_service.cpp \
        $$PWD/ve_qitem_exported_dbus_services.cpp \

    HEADERS += \
        $$PWD/ve_qitem_exported_dbus_service.hpp \
        $$VE_UTIL_INC/qt/ve_qitems_dbus.hpp \
        $$VE_UTIL_INC/qt/ve_qitem_exported_dbus_services.hpp \
}

# configure option, allow MQTT-backed VeQItems
CONFIG(ve-qitems-mqtt) {
    SOURCES += $$PWD/ve_qitems_mqtt.cpp
    HEADERS += $$VE_UTIL_INC/qt/ve_qitems_mqtt.hpp
}

!lessThan(QT_VERSION, 5) | contains(QT, gui) {
    SOURCES += \
        $$PWD/ve_qitem_sort_table_model.cpp \

    HEADERS += \
        $$VE_UTIL_INC/qt/ve_qitem_sort_table_model.hpp \
        $$VE_UTIL_INC/qt/ve_sort_filter_proxy_model.hpp \
}

contains(QT, gui|widgets) {
    INCLUDEPATH += $$PWD/gui

    SOURCES += \
        $$PWD/gui/ve_qitem_table_widget.cpp \
        $$PWD/gui/ve_qitem_tree_widget.cpp \

    HEADERS += \
        $$PWD/gui/ve_qitem_table_widget.hpp \
        $$PWD/gui/ve_qitem_tree_widget.hpp \

    FORMS += \
        $$PWD/gui/ve_qitem_table_widget.ui \
        $$PWD/gui/ve_qitem_tree_widget.ui \
}

contains(QT, declarative|qml) {
    SOURCES += \
        $$PWD/ve_qitem_child_model.cpp \
        $$PWD/ve_quick_item.cpp \

    HEADERS += \
        $$VE_UTIL_INC/qt/ve_qitem_child_model.hpp \
        $$VE_UTIL_INC/qt/ve_quick_item.hpp \
}

include("../../common.pri")
