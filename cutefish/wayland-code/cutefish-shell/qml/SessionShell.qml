import QtQuick

Item {
    id: root
    Rectangle {
        anchors.fill: parent
        color: "#101318"
    }
    Desktop { anchors.fill: parent }
    StatusBar {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 36
    }
    Dock {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
    }
    Launcher { anchors.centerIn: parent }
}
