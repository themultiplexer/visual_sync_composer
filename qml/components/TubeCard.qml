import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: root

    required property int tubeIndex
    required property var tube

    signal changed(int delay, int group)
    signal moveRequested(int offset)
    signal flashFirmwareRequested()
    signal peakRequested()

    property color peakColor: "transparent"

    implicitWidth: 130

    function flash(color) {
        peakColor = color
        peakAnimation.restart()
    }

    background: Rectangle {
        radius: 6
        color: "#26292e"
        border.color: root.peakColor.a > 0 ? root.peakColor : "#4f545c"
        border.width: root.peakColor.a > 0 ? 4 : 1
    }

    SequentialAnimation {
        id: peakAnimation
        ColorAnimation {
            target: root
            property: "peakColor"
            from: root.peakColor
            to: "transparent"
            duration: 450
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.tube.mac
            elide: Text.ElideMiddle
            font.bold: true
        }

        Label { text: qsTr("Delay") }
        SpinBox {
            id: delayBox
            Layout.fillWidth: true
            from: 0
            to: 255
            value: root.tube.delay
            editable: true
            onValueModified: root.changed(value, groupBox.value)
        }

        Label { text: qsTr("Group") }
        SpinBox {
            id: groupBox
            Layout.fillWidth: true
            from: 0
            to: 32
            value: root.tube.group
            editable: true
            onValueModified: root.changed(delayBox.value, value)
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "◀"
                enabled: root.tubeIndex > 0
                onClicked: root.moveRequested(-1)
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Peak")
                onClicked: root.peakRequested()
            }
            Button {
                text: "▶"
                onClicked: root.moveRequested(1)
            }
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("FW mode")
            onClicked: root.flashFirmwareRequested()
        }
    }
}
