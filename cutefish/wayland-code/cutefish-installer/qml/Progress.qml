import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Installing")
    Column {
        anchors.centerIn: parent
        spacing: 12
        BusyIndicator { anchors.horizontalCenter: parent.horizontalCenter; running: true }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("No installation job is running in this build")
            color: "white"
        }
    }
}
