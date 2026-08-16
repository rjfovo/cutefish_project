import QtQuick
import QtQuick.Controls

Page {
    title: qsTr("Welcome")
    Label {
        anchors.centerIn: parent
        text: qsTr("CutefishOS\n\nA dedicated, simple and stable Wayland desktop.")
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 22
        color: "white"
    }
}
