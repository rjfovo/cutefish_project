import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import cutefish.bluez 1.0
import Cutefish.Screen 1.0

Window {
    width: 400
    height: 300
    visible: true
    title: "QML Module Test"

    Column {
        anchors.centerIn: parent
        spacing: 10

        Text {
            text: "cutefish.bluez module: " + (typeof DevicesModel !== "undefined" ? "✓ Available" : "✗ Not Available")
            font.pixelSize: 16
        }

        Text {
            text: "Cutefish.Screen module: " + (typeof Screen !== "undefined" ? "✓ Available" : "✗ Not Available")
            font.pixelSize: 16
        }

        Button {
            text: "Test QML Engine"
            onClicked: {
                console.log("Testing QML modules...")
                if (typeof DevicesModel !== "undefined") {
                    console.log("cutefish.bluez module is available")
                } else {
                    console.log("ERROR: cutefish.bluez module not found")
                }
                if (typeof Screen !== "undefined") {
                    console.log("Cutefish.Screen module is available")
                } else {
                    console.log("ERROR: Cutefish.Screen module not found")
                }
            }
        }
    }
}