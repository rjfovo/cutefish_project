import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Cutefish Shell")
    color: "#1e1f24"

    Loader {
        anchors.fill: parent
        sourceComponent: {
            switch (ShellClient.mode) {
            case 0:
                return bootComponent
            case 1:
                return loginComponent
            case 2:
                return sessionComponent
            case 3:
                return lockComponent
            case 4:
                return shutdownComponent
            default:
                return bootComponent
            }
        }
    }

    Component { id: bootComponent; BootShell {} }
    Component { id: loginComponent; LoginShell {} }
    Component { id: sessionComponent; SessionShell {} }
    Component { id: lockComponent; LockShell {} }
    Component { id: shutdownComponent; ShutdownShell {} }
}
