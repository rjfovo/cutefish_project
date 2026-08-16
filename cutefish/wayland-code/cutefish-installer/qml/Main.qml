import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1024
    height: 640
    visible: true
    title: qsTr("CutefishOS Installer")
    color: "#14161b"

    StackLayout {
        anchors.fill: parent
        currentIndex: InstallerBackend.currentStep
        Welcome { }
        Disk { }
        PartitionAdvanced { }
        Timezone { }
        User { }
        Summary { }
        Progress { }
        Finish { }
    }

    footer: ToolBar {
        Row {
            anchors.right: parent.right
            anchors.rightMargin: 12
            spacing: 8
            Button {
                text: qsTr("Back")
                enabled: InstallerBackend.currentStep > 0
                onClicked: InstallerBackend.back()
            }
            Button {
                text: qsTr("Next")
                visible: InstallerBackend.currentStep < 5
                onClicked: InstallerBackend.next()
            }
            Button {
                text: qsTr("Install")
                visible: InstallerBackend.currentStep === 5
                enabled: !InstallerBackend.dangerousJobsAllowed
                onClicked: InstallerBackend.beginInstall()
            }
            Button {
                text: qsTr("Finish")
                visible: InstallerBackend.currentStep === 7
                onClicked: Qt.quit()
            }
        }
    }

    Label {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
        text: InstallerBackend.lastError
        color: "#ffb4a9"
        visible: text.length > 0
        z: 10
    }
}
