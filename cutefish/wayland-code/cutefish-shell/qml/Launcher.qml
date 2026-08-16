import QtQuick
import QtQuick.Controls

Item {
    id: launcher
    width: 480
    height: 360
    visible: false
    Rectangle {
        anchors.fill: parent
        radius: 18
        color: "#242833"
        opacity: 0.96
    }
    Label {
        anchors.centerIn: parent
        text: qsTr("Launcher placeholder")
        color: "white"
    }
}
