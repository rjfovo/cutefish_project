import QtQuick
import QtQuick.Controls

Item {
    id: root
    Rectangle {
        anchors.fill: parent
        color: "#14161b"
    }
    Column {
        anchors.centerIn: parent
        spacing: 18
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Shutting down…")
            font.pixelSize: 32
            font.bold: true
            color: "white"
        }
        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            running: true
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("The compositor core remains active until the final stage")
            color: "#c0c5ce"
        }
    }
}
