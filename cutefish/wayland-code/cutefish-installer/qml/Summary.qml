import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Summary")
    Label {
        anchors.centerIn: parent
        text: qsTr("Fixed summary placeholder.\nInstallation jobs are intentionally disabled in stage-0.")
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }
}
