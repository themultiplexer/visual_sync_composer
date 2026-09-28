import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

Frame {
    id: root

    required property int tubeIndex
    required property var tube

    signal changed(int delay, int group)
    signal moveRequested(int offset)
    signal flashFirmwareRequested()
    signal peakRequested()

    property color pendingPeakColor: "transparent"
    property color peakColor: "transparent"
    property int peakDelay

    function flash(color, delay) {
        pendingPeakColor = color
        peakDelay = delay
        peakAnimation.restart()
    }

    background: Rectangle {
        radius: 6
        color: "#26292e"
        Shape {
            anchors.fill: parent

            ShapePath {
                strokeColor: root.peakColor.a > 0
                             ? root.peakColor
                             : "#4f545c"

                strokeWidth: root.peakColor.a > 0 ? 4 : 1

                strokeStyle: ShapePath.DashLine
                dashPattern: [4, 4]

                fillColor: "transparent"

                PathRectangle {
                    x: 0.5
                    y: 0.5
                    width: root.width - 1
                    height: root.height - 1
                    radius: 6
                }
            }
        }
    }

    SequentialAnimation {
        id: peakAnimation
        PauseAnimation {
            duration: root.peakDelay
        }
        PropertyAction {
            target: root
            property: "peakColor"
            value: root.pendingPeakColor
        }
        ColorAnimation {
            target: root
            property: "peakColor"
            to: "transparent"
            duration: 450
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        Label {
            Layout.fillWidth: true
            text: root.tube.mac
            elide: Text.ElideMiddle
            padding: 0
            color: "#FFFFFF"
            font.bold: true
        }

        RowLayout {
        Label { text: qsTr("Delay"); color: "#FFFFFF"; }
            SpinBox {
                id: delayBox
                from: 0
                to: 255
                value: root.tube.delay
                padding: 0
                editable: true
                implicitWidth: 60
                onValueModified: root.changed(value, groupBox.value)
            }
        }
        RowLayout {
            Label { text: qsTr("Group"); color: "#FFFFFF"; }
            SpinBox {
                id: groupBox
                from: 0
                to: 32
                value: root.tube.group
                padding: 0
                editable: true
                implicitWidth: 40
                onValueModified: root.changed(delayBox.value, value)
            }
        }

        RowLayout {
            Button {
                text: "◀"
                enabled: root.tubeIndex > 0
                onClicked: root.moveRequested(-1)
                implicitWidth: 40
                padding: 0
            }
            Button {
                text: qsTr("Peak")
                padding: 0
                onClicked: root.peakRequested()
                implicitWidth: 40
            }
            Button {
                text: "▶"
                padding: 0
                onClicked: root.moveRequested(1)
                implicitWidth: 40
            }
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("FW mode")
            onClicked: root.flashFirmwareRequested()
        }
    }
}
