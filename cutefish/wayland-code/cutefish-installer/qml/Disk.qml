import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Select disk")
    Column {
        anchors.centerIn: parent
        spacing: 12
        Label {
            text: qsTr("Disk selection placeholder.\nThe live medium is excluded and the disk must be confirmed before partitioning.")
            horizontalAlignment: Text.AlignHCenter
            color: "white"
        }
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Dangerous disk jobs are disabled in this build")
            enabled: false
        }
    }
}
