import QtQuick
import QtQuick.Controls

Rectangle {
    id: dock
    width: Math.min(520, parent ? parent.width * 0.6 : 520)
    height: 64
    radius: 14
    color: "#2a2d35"
    opacity: 0.92

    Row {
        anchors.centerIn: parent
        spacing: 8
        Repeater {
            model: WindowModel
            delegate: Button {
                width: 48
                height: 48
                text: model.title.length > 0 ? model.title.substring(0, 1) : "?"
                highlighted: model.activated
                onClicked: CoreClient.requestActivate(model.appId)
            }
        }
        Label {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            visible: WindowModel.rowCount() === 0
            text: qsTr("Dock placeholder")
            color: "white"
        }
    }
}
