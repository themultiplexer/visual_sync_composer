pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    property string name: ""
    property alias value: slider.value
    property var regions: []

    signal edited(int value)

    spacing: 4

    Label {
        text: root.name
        Layout.fillWidth: true
        color: "#FFFFFF"
    }


    Slider {
        id: slider
        onMoved: edited(Math.round(value))
        implicitHeight: 32
        Layout.fillWidth: true

        from: 0
        to: 255
        stepSize: 1

        function regionColor(region) {
            // Prefer an explicitly supplied color, if available.
            if (region.properties && region.properties.color)
                return region.properties.color

            return Qt.hsva(region.start/255.0, 1.0, 1.0, 1.0)
        }

        background: Item {
            x: slider.leftPadding
            y: slider.topPadding + (slider.availableHeight - height) / 2
            width: slider.availableWidth
            height: 8

            // Track behind the colored regions.
            Rectangle {
                anchors.fill: parent
                radius: height / 2
                color: "#000000"
            }

            Repeater {
                model: root.regions

                Rectangle {
                    required property var modelData

                    x: (modelData.start / 256) * parent.width
                    width: Math.max(
                        0,
                        ((modelData.end - modelData.start + 1) / 256)
                        * parent.width
                    )
                    height: parent.height

                    color: slider.regionColor(modelData)

                    // Keep the outer corners rounded.
                    radius: 2
                }
            }
        }

        handle: Rectangle {
            x: slider.leftPadding
            + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + (slider.availableHeight - height) / 2

            width: 16
            height: 16
            radius: width / 2

            color: "#FFFFFF"
            border.color: "#24272D"
            border.width: 2
        }
    }
}