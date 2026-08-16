import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Complete")
    Label {
        anchors.centerIn: parent
        text: qsTr("Installation complete.\nReboot placeholder (reboot is disabled in stage-0).")
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }
}
