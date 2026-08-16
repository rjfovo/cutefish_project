import QtQuick
import QtQuick.Controls

Item {
    id: root
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1e1f24" }
            GradientStop { position: 1.0; color: "#0b0c10" }
        }
    }
    Column {
        anchors.centerIn: parent
        spacing: 20
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("CutefishOS")
            font.pixelSize: 42
            font.bold: true
            color: "white"
        }
        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            running: true
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Starting system…")
            color: "#c0c5ce"
        }
    }
}
