import QtQuick
import QtQuick.Controls

Rectangle {
    id: statusBar
    color: "#1a1d24"
    opacity: 0.96

    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 12
        spacing: 12
        Label { text: qsTr("CutefishOS"); color: "white" }
        Label {
            text: {
                for (var i = 0; i < WindowModel.rowCount(); ++i) {
                    if (WindowModel.index(i, 0).data(WindowModel.ActivatedRole))
                        return WindowModel.index(i, 0).data(WindowModel.TitleRole)
                }
                return qsTr("StatusBar: waiting for window model")
            }
            color: "#c0c5ce"
        }
    }
}
