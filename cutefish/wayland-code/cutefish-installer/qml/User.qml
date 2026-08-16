import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("User")
    Column {
        anchors.centerIn: parent
        spacing: 8
        TextField { placeholderText: qsTr("User name"); width: 320 }
        TextField { placeholderText: qsTr("Password"); echoMode: TextInput.Password; width: 320 }
        Switch { text: qsTr("Automatic login") }
    }
}
