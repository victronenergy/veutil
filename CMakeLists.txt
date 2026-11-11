# Victron.VeUtil: a static Qt 6 QML module with VeQuickItem, the VeQItem
# producers (MQTT, and D-Bus where available) and the other QML types of veutil.
#
# The qmake files (veutil.pro, veutil.pri, common.pri) are unchanged and still
# build the library for projects that use qmake. This file is only for CMake
# projects that use veutil's QML types, such as gui-v2.
#
# As a subproject:
#     set(VEUTIL_DBUS OFF)                 # optional, see the options below
#     add_subdirectory(path/to/veutil)
#     target_link_libraries(myapp PRIVATE VictronVeUtil VictronVeUtilplugin)
# QML then uses "import Victron.VeUtil". The target carries its include path
# and the definitions its headers depend on, so the app needs nothing else.
#
# Standalone (to check that it builds):
#     qt-cmake -S path/to/veutil -B build && cmake --build build
#
# Variables a parent project may set before add_subdirectory():
#   QT_QML_OUTPUT_DIRECTORY  where Victron/VeUtil/qmldir is written, so that
#                            tools such as qmllint find it next to the app's
#                            other modules. Default: this binary directory.
#   VEUTIL_QML_INSTALL_DIR   where cmake --install puts Victron/VeUtil/qmldir
#                            and the qmltypes. Default: Qt's QML install dir.
#
# After add_subdirectory(), VictronVeUtil_TRANSLATION_SOURCES holds the
# absolute paths of the sources that contain translatable strings.

cmake_minimum_required(VERSION 3.24)

project(veutil LANGUAGES CXX)

# D-Bus is not available in WebAssembly, and a browser can only reach an MQTT
# broker over a WebSocket. A parent project may set either variable before
# add_subdirectory() to override the default.
if(EMSCRIPTEN)
    set(_veutil_dbus_default OFF)
    set(_veutil_websockets_default ON)
else()
    set(_veutil_dbus_default ON)
    set(_veutil_websockets_default OFF)
endif()
option(VEUTIL_DBUS "Build the D-Bus VeQItem producer (VeQItemDbusProducer)" ${_veutil_dbus_default})
# This changes the members of VeQItemMqttProducer, so it is a PUBLIC definition.
option(VEUTIL_MQTT_WEBSOCKETS "Let VeQItemMqttProducer connect over a WebSocket" ${_veutil_websockets_default})

# GLOBAL, because the Qt targets that VictronVeUtil links are linked into the
# parent project's app too, and an imported target found in this directory is
# otherwise not visible there. (Qt warns "the non-existent target 'Qt6::Quick'
# was encountered" when it finalizes such an app.)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml Quick Mqtt GLOBAL)
if(VEUTIL_DBUS)
    find_package(Qt6 6.8 REQUIRED COMPONENTS DBus Xml GLOBAL)
endif()
if(VEUTIL_MQTT_WEBSOCKETS)
    find_package(Qt6 6.8 REQUIRED COMPONENTS WebSockets GLOBAL)
endif()

# Put the module under the default import path (:/qt/qml/Victron/VeUtil).
if(QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 NEW)
endif()

set(VictronVeUtil_CORE_SOURCES
    inc/veutil/qt/alternator_error.hpp
    inc/veutil/qt/bms_error.hpp
    inc/veutil/qt/charger_error.hpp
    inc/veutil/qt/firmware_updater_data.hpp
    inc/veutil/qt/genset_error.hpp
    inc/veutil/qt/unit_conversion.hpp
    inc/veutil/qt/vebus_error.hpp
    inc/veutil/qt/ve_qitem.hpp
    inc/veutil/qt/ve_qitem_child_model.hpp
    inc/veutil/qt/ve_qitem_loader.hpp
    inc/veutil/qt/ve_qitem_sort_table_model.hpp
    inc/veutil/qt/ve_qitem_table_model.hpp
    inc/veutil/qt/ve_qitem_tree_model.hpp
    inc/veutil/qt/ve_quick_item.hpp

    src/qt/alternator_error.cpp
    src/qt/bms_error.cpp
    src/qt/charger_error.cpp
    src/qt/genset_error.cpp
    src/qt/unit_conversion.cpp
    src/qt/vebus_error.cpp
    src/qt/ve_qitem.cpp
    src/qt/ve_qitem_child_model.cpp
    src/qt/ve_qitem_loader.cpp
    src/qt/ve_qitem_sort_table_model.cpp
    src/qt/ve_qitem_table_model.cpp
    src/qt/ve_qitem_tree_model.cpp
    src/qt/ve_quick_item.cpp
)

set(VictronVeUtil_MQTT_SOURCES
    inc/veutil/qt/ve_qitems_mqtt.hpp

    src/qt/ve_qitems_mqtt.cpp
)

set(VictronVeUtil_DBUS_SOURCES
    inc/veutil/qt/ve_dbus_connection.hpp
    inc/veutil/qt/ve_qitems_dbus.hpp

    src/qt/ve_dbus_connection.cpp
    src/qt/ve_qitems_dbus.cpp
)

set(VictronVeUtil_SOURCES ${VictronVeUtil_CORE_SOURCES} ${VictronVeUtil_MQTT_SOURCES})
if(VEUTIL_DBUS)
    list(APPEND VictronVeUtil_SOURCES ${VictronVeUtil_DBUS_SOURCES})
endif()

if(QT_QML_OUTPUT_DIRECTORY)
    set(_veutil_qml_dir "${QT_QML_OUTPUT_DIRECTORY}/Victron/VeUtil")
else()
    set(_veutil_qml_dir "${CMAKE_CURRENT_BINARY_DIR}/Victron/VeUtil")
endif()

qt_add_library(VictronVeUtil STATIC)
# The QML type registration needs moc's output.
set_target_properties(VictronVeUtil PROPERTIES AUTOMOC ON)
qt_add_qml_module(VictronVeUtil
    URI Victron.VeUtil
    VERSION 1.0
    OUTPUT_DIRECTORY "${_veutil_qml_dir}"
    SOURCES ${VictronVeUtil_SOURCES}
    # VeQItemSortDelegate is a QQuickItem.
    DEPENDENCIES QtQuick
)

# Only veutil's own headers are on the include path. The second directory is
# for the generated QML type registration, which includes the headers by name.
target_include_directories(VictronVeUtil PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/veutil/qt")

# The public headers include the Qt Quick, Qt MQTT and, when enabled, Qt D-Bus
# and Qt WebSockets headers, so whoever links this module needs them too.
target_link_libraries(VictronVeUtil PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Mqtt
)

if(VEUTIL_DBUS)
    # ve_qitems_dbus.cpp reads the D-Bus introspection XML.
    target_link_libraries(VictronVeUtil PUBLIC Qt6::DBus PRIVATE Qt6::Xml)
endif()

if(VEUTIL_MQTT_WEBSOCKETS)
    # ve_qitems_mqtt.hpp declares different members when this is defined, so
    # every file that includes it, here or in an app that links this module,
    # must see the same definition.
    target_compile_definitions(VictronVeUtil PUBLIC MQTT_WEBSOCKETS_ENABLED)
    target_link_libraries(VictronVeUtil PUBLIC Qt6::WebSockets)
endif()

# The qmldir and qmltypes are installed, so that tools such as qmllint can
# resolve the types of this module and of the modules that import it.
if(NOT DEFINED VEUTIL_QML_INSTALL_DIR)
    if(QT6_INSTALL_QML)
        set(VEUTIL_QML_INSTALL_DIR "${QT6_INSTALL_QML}")
    else()
        set(VEUTIL_QML_INSTALL_DIR "qml")
    endif()
endif()
qt_query_qml_module(VictronVeUtil QMLDIR _veutil_qmldir TYPEINFO _veutil_typeinfo)
install(FILES "${_veutil_qmldir}" "${_veutil_typeinfo}"
    DESTINATION "${VEUTIL_QML_INSTALL_DIR}/Victron/VeUtil")

# The sources with translatable strings, for the parent project's lupdate.
list(TRANSFORM VictronVeUtil_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/"
    OUTPUT_VARIABLE VictronVeUtil_TRANSLATION_SOURCES)
if(NOT PROJECT_IS_TOP_LEVEL)
    set(VictronVeUtil_TRANSLATION_SOURCES "${VictronVeUtil_TRANSLATION_SOURCES}" PARENT_SCOPE)
endif()
