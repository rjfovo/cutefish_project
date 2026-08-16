import QtQuick
import QtQuick.Controls

Rectangle {
    id: desktop
    color: "#123456"
    Label {
        anchors.centerIn: parent
        text: qsTr("Desktop placeholder: wallpaper and file icons arrive with the QML migration")
        color: "white"
        font.pixelSize: 18
    }
}
