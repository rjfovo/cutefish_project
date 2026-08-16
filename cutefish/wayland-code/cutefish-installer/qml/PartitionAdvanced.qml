import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Partition")
    Label {
        anchors.centerIn: parent
        text: qsTr("Default: automatic GPT with ESP + root.\nAdvanced mode: choose existing partitions and mount points only.")
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }
}
