import QtQuick
import QtQuick.Controls

Rectangle {
    id: dock
    width: Math.min(520, parent ? parent.width * 0.6 : 520)
    height: 64
    radius: 14
    color: "#2a2d35"
    opacity: 0.92
    Label {
        anchors.centerIn: parent
        text: qsTr("Dock placeholder")
        color: "white"
    }
}
