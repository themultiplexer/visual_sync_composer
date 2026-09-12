import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root

    property alias text: label.text
    property alias from: slider.from
    property alias to: slider.to
    property alias stepSize: slider.stepSize
    property alias value: slider.value
    property alias live: slider.live
    property int decimals: 0
    property bool integral: true

    signal edited(real value)

    spacing: 8

    Label {
        id: label
        Layout.preferredWidth: 90
        color: "#FFFFFF"
        font.bold: true
    }

    Slider {
        id: slider
        Layout.fillWidth: true
        onMoved: root.edited(value)
    }

    Label {
        Layout.preferredWidth: 44
        horizontalAlignment: Text.AlignRight
        text: root.integral
              ? Math.round(slider.value).toString()
              : slider.value.toFixed(root.decimals)
        font.features: { "tnum": 1 }
    }
}
