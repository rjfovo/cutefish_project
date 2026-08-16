import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Timezone")
    Label {
        anchors.centerIn: parent
        text: qsTr("Timezone selection placeholder.\nWrites target rootfs only; never changes the live system.")
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }
}
