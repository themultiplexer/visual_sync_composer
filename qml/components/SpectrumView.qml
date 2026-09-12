import QtQuick
import visual_sync_composer 1.0

Item {
    id: root

    OGLTest {
        id: widget
        anchors.fill: parent
    }

    property var leftSpectrum: []
    property var rightSpectrum: []
    property real threshold: 0.8
    property real decay: 0.05
    property int visualizationMode: 0
    property real filterLower: 0.0
    property real filterUpper: 1.0

    signal filterRangeEdited(real lower, real upper)
    signal sigtest()

    implicitHeight: 180

    onLeftSpectrumChanged: widget.requestPaint()
    onRightSpectrumChanged: widget.requestPaint()
   
}
