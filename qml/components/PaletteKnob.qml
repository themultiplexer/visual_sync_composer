import QtQuick
import QtQuick.Controls

Item {
    id: root

    property real hue: 0.0
    property real saturation: 1.0
    signal edited(real hue, real saturation)

    implicitWidth: 64
    implicitHeight: 64

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height) - 4
        height: width
        radius: width / 2
        color: Qt.hsva(root.hue, root.saturation, 1.0, 1.0)
        border.color: "white"
        border.width: 2

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * (1.0 - root.saturation) * 0.65
            height: width
            radius: width / 2
            color: "white"
            opacity: 0.8
        }

        Rectangle {
            anchors.centerIn: parent
            width: 3
            height: parent.height * 0.38
            color: "#202226"
            transformOrigin: Item.Bottom
            rotation: root.hue * 360.0
            anchors.verticalCenterOffset: -height / 2
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
        property real lastY: 0

        onPressed: mouse => lastY = mouse.y
        onPositionChanged: mouse => {
            if (!pressed)
                return
            const difference = (lastY - mouse.y) / 150.0
            lastY = mouse.y
            if (mouse.modifiers & Qt.ShiftModifier) {
                root.edited(root.hue,
                            Math.max(0.0, Math.min(1.0, root.saturation + difference)))
            } else {
                let nextHue = root.hue + difference
                nextHue = nextHue - Math.floor(nextHue)
                root.edited(nextHue, root.saturation)
            }
        }
    }

    ToolTip.visible: hover.hovered
    ToolTip.text: qsTr("Drag: hue · Shift+drag: saturation")
    HoverHandler { id: hover }
}
