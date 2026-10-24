import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: beatIndicator

    width: 60
    height: 60
    radius: 6
    color: "#00e5ff"
    opacity: 0.15

    signal clicked()

    function beat() {
        fade.stop()
        opacity = 1.0
        fade.start()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            beatTapper.tap() 
        }
    }

    NumberAnimation {
        id: fade
        target: beatIndicator
        property: "opacity"
        to: 0.15
        duration: 200
        easing.type: Easing.OutCubic
    }

    Connections {
        target: beatClock

        function onBeatDetected() {
            beatIndicator.beat()
        }
    }

    Connections {
        target: beatTapper

        function onBeatDetected() {
            beatIndicator.beat()
        }
    }

    Component.onCompleted: beatClock.start()
    Component.onDestruction: beatClock.stop()
}