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
    Column {
        anchors.centerIn: parent
        spacing: 8
        Label { text: qsTr("Launcher placeholder"); color: "white" }
        Repeater {
            model: OutputModel
            delegate: Label {
                text: qsTr("Display %1").arg(model.name)
                color: "#c0c5ce"
            }
        }
    }
}
