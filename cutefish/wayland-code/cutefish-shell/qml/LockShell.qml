import QtQuick
import QtQuick.Controls

Item {
    id: root
    Rectangle {
        anchors.fill: parent
        color: "#101318"
    }
    Column {
        anchors.centerIn: parent
        spacing: 16
        width: 340
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Locked")
            font.pixelSize: 34
            font.bold: true
            color: "white"
        }
        TextField {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
        }
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Unlock")
            onClicked: lockStatus.text = qsTr("LockShell placeholder: core owns lock state; unlock is implemented in stage-2")
        }
        Label {
            id: lockStatus
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            wrapMode: Text.Wrap
            color: "#c0c5ce"
        }
    }
}
