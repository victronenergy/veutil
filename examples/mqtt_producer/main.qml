import QtQuick
import QtQuick.Window
import Victron.VeUtil

Window {
    id: root

    title: "Testing MQTT VeQItem Support"
    color: "lightsteelblue"

    width: 600
    height: 600

    Text {
        anchors.centerIn: parent
        text: "Voltage = " + batteryVoltage.value
        font.family: "Helvetica"
        font.pointSize: 24
        color: "black"
    }

    VeQuickItem {
        id: batteryVoltage
        uid: "mqtt/system/0/Dc/Battery/Voltage"
    }
}
