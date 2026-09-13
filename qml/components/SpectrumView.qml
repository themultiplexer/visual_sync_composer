import QtQuick
import visual_sync_composer

OGLTest {
    id: oglwidget

    property var leftSpectrum: []
    property var rightSpectrum: []
    property var regions: []
    property real threshold: 0.8
    property real decay: 0.05
    property int visualizationMode: 0
    property real filterLower: 0.0
    property real filterUpper: 1.0

    signal filterRangeEdited(real lower, real upper)
    signal sigtest()

    onLeftSpectrumChanged: {
        oglwidget.setSpectrum(leftSpectrum)
        oglwidget.update()
    }
    onRightSpectrumChanged:  {
        oglwidget.setSpectrum(leftSpectrum)
        oglwidget.update()
    }
    onRegionsChanged:  {
        oglwidget.setRegions(regions)
        oglwidget.update()
    }

    Component.onCompleted: {
        console.log("OGLTest completed:", oglwidget)
    }

    Component.onDestruction: {
        console.log("OGLTest destroyed:", oglwidget)
    }
}