import QtQuick
import QtQuick.Controls

Rectangle {
    id: desktop
    color: "#123456"

    // 阶段占位：壁纸与文件图标仍待迁移；输出信息已接入 core 模型。
    Column {
        anchors.centerIn: parent
        spacing: 8
        Repeater {
            model: OutputModel
            delegate: Label {
                anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                text: qsTr("Output %1: %2x%3 scale=%4").arg(model.name).arg(model.width).arg(model.height).arg(model.scale)
                color: "white"
            }
        }
        Label {
            anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
            text: qsTr("Desktop placeholder: wallpaper and file icons arrive with the QML migration")
            color: "#c0c5ce"
            font.pixelSize: 14
        }
    }
}
