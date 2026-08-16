import QtQuick
import QtQuick.Controls

Rectangle {
    id: statusBar
    color: "#1a1d24"
    opacity: 0.96
    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 12
        spacing: 12
        Label { text: qsTr("CutefishOS"); color: "white" }
        Label { text: qsTr("StatusBar placeholder"); color: "#c0c5ce" }
    }
}
