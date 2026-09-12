import QtQuick

Item {
    id: root

    property var leftSpectrum: []
    property var rightSpectrum: []
    property real threshold: 0.8
    property real decay: 0.05
    property int visualizationMode: 0
    property real filterLower: 0.0
    property real filterUpper: 1.0

    signal filterRangeEdited(real lower, real upper)

    implicitHeight: 180

    onLeftSpectrumChanged: spectrumCanvas.requestPaint()
    onRightSpectrumChanged: spectrumCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        radius: 5
        color: "#111318"
        border.color: "#41464e"
    }

    Canvas {
        id: spectrumCanvas
        anchors.fill: parent
        anchors.margins: 2

        function drawSpectrum(context, values, strokeColor, mirror) {
            if (!values || values.length < 2)
                return
            context.beginPath()
            context.strokeStyle = strokeColor
            context.lineWidth = 1.25
            for (let i = 0; i < values.length; ++i) {
                const x = i * width / (values.length - 1)
                const amplitude = Math.min(1.0, Math.max(0.0, Number(values[i])))
                const y = mirror
                          ? height * 0.5 + amplitude * height * 0.48
                          : height * 0.5 - amplitude * height * 0.48
                if (i === 0)
                    context.moveTo(x, y)
                else
                    context.lineTo(x, y)
            }
            context.stroke()
        }

        onPaint: {
            const context = getContext("2d")
            context.reset()
            context.clearRect(0, 0, width, height)

            context.strokeStyle = "#333840"
            context.lineWidth = 1
            context.beginPath()
            context.moveTo(0, height / 2)
            context.lineTo(width, height / 2)
            context.stroke()

            drawSpectrum(context, root.leftSpectrum, "#65d1ff", false)
            drawSpectrum(context, root.rightSpectrum, "#ff6fae", true)

            context.fillStyle = "rgba(255, 211, 92, 0.12)"
            context.fillRect(root.filterLower * width, 0,
                             (root.filterUpper - root.filterLower) * width, height)
        }
    }

    Rectangle {
        id: lowerHandle
        x: root.filterLower * (root.width - width)
        width: 10
        height: parent.height
        color: "#ffd35c"
        opacity: 0.8

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.SizeHorCursor
            drag.target: lowerHandle
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: upperHandle.x - lowerHandle.width
            onPositionChanged: {
                if (!drag.active)
                    return
                root.filterLower = lowerHandle.x / (root.width - lowerHandle.width)
                root.filterRangeEdited(root.filterLower, root.filterUpper)
                spectrumCanvas.requestPaint()
            }
        }
    }

    Rectangle {
        id: upperHandle
        x: root.filterUpper * (root.width - width)
        width: 10
        height: parent.height
        color: "#ffd35c"
        opacity: 0.8

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.SizeHorCursor
            drag.target: upperHandle
            drag.axis: Drag.XAxis
            drag.minimumX: lowerHandle.x + upperHandle.width
            drag.maximumX: root.width - upperHandle.width
            onPositionChanged: {
                if (!drag.active)
                    return
                root.filterUpper = upperHandle.x / (root.width - upperHandle.width)
                root.filterRangeEdited(root.filterLower, root.filterUpper)
                spectrumCanvas.requestPaint()
            }
        }
    }
}
