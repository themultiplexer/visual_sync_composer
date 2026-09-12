import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import "components"

ApplicationWindow {
    id: window

    width: 1600
    height: 980
    minimumWidth: 1100
    minimumHeight: 720
    visible: true
    title: qsTr("Visual Sync Controller")

    Dev {}


    onClosing: function(close) {
        if (!allowClose) {
            close.accepted = false
            quitDialog.open()
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action {
                text: qsTr("Open firmware…")
                onTriggered: firmwareDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Exit")
                onTriggered: quitDialog.open()
            }
        }
        Menu {
            title: qsTr("View")
            Action {
                text: qsTr("Fullscreen spectrum")
                onTriggered: fullscreenSpectrum.showFullScreen()
            }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            Label {
                text: qsTr("Status:")
                font.bold: true
            }
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: audioController.online ? "#55d17a" : "#e45b5b"
            }
            Label {
                text: audioController.online ? qsTr("Online") : qsTr("Offline")
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("All → FW mode")
                onClicked: audioController.putAllTubesIntoFirmwareUpdateMode()
            }
            Button {
                text: qsTr("Flash firmware")
                onClicked: firmwareDialog.open()
            }
            Button {
                text: qsTr("Send hello")
                onClicked: audioController.sendHello()
            }
            Button {
                text: qsTr("DMX tool")
                onClicked: uiBridge.openDmxTool()
            }
        }
    }

}
