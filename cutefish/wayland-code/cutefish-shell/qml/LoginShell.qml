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
        spacing: 16
        width: 360
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("CutefishOS")
            font.pixelSize: 38
            font.bold: true
            color: "white"
        }
        TextField {
            id: userName
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            placeholderText: qsTr("User name")
        }
        TextField {
            id: password
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
        }
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Log in")
            onClicked: {
                // Stage-2 will route credentials through the PAM helper.
                statusLabel.text = qsTr("LoginShell placeholder: authentication is implemented in stage-2")
            }
        }
        Label {
            id: statusLabel
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            wrapMode: Text.Wrap
            color: "#c0c5ce"
        }
    }
}
