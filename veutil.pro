TEMPLATE = lib

QT *= dbus

equals(QT_MAJOR_VERSION, 4) {
    QT += declarative
}

!lessThan(QT_VERSION, 5) {
    QT += qml quick widgets
}

include("veutil.pri")
