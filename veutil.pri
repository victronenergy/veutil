SOURCES += \
    $$PWD/src/plt/exit_on_oom_glibc.c \
    $$PWD/src/qt/alternator_error.cpp \
    $$PWD/src/qt/bms_error.cpp \
    $$PWD/src/qt/canbus_interfaces.cpp \
    $$PWD/src/qt/charger_error.cpp \
    $$PWD/src/qt/daemontools_service.cpp \
    $$PWD/src/qt/vebus_error.cpp \
    $$PWD/src/qt/venus_types.cpp \

HEADERS += \
    $$PWD/inc/veutil/base/compiler.h \
    $$PWD/inc/veutil/charger_error.h \
    $$PWD/inc/veutil/platform/plt.h \
    $$PWD/inc/veutil/qt/alternator_error.hpp \
    $$PWD/inc/veutil/qt/bms_error.hpp \
    $$PWD/inc/veutil/qt/canbus_interfaces.hpp \
    $$PWD/inc/veutil/qt/charger_error.hpp \
    $$PWD/inc/veutil/qt/daemontools_service.hpp \
    $$PWD/inc/veutil/qt/firmware_updater_data.hpp \
    $$PWD/inc/veutil/qt/vebus_error.hpp \
    $$PWD/inc/veutil/qt/venus_types.hpp \
    $$PWD/inc/veutil/ve_regs_payload.h \

contains(QT, dbus|qdbus) {
    contains(VE_CONFIG, udev) {
        SOURCES += \
            $$PWD/src/qt/canbus_monitor.cpp \
            $$PWD/src/qt/q_udev.cpp \

        HEADERS += \
            $$PWD/inc/veutil/qt/canbus_monitor.hpp \
            $$PWD/inc/veutil/qt/q_udev.hpp \

        LIBS *= -ludev
    }

    SOURCES += \
        $$PWD/src/qt/ve_dbus_connection.cpp \

    HEADERS += \
        $$PWD/inc/veutil/qt/ve_dbus_connection.hpp \
}

include("common.pri")
include("$$PWD/src/qt/veqitem.pri")
