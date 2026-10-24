pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: fixture
    required property var modelData
    required property int index
    required property bool detailed

    signal fullscreenRequested()

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Label {
                text: modelData.name
                color: "#FFFFFF"
                font.bold: true
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("-")
                onClicked: fixture.fullscreenRequested()
            }
            Button {
                text: qsTr("[ ]")
                onClicked: fixture.fullscreenRequested()
            }
        }

        RowLayout {
            Label {
                text: qsTr("Group")
                font.bold: true
                color: "#FFFFFF"
                Layout.preferredWidth: 60
            }

            SpinBox {
                from: 0
                to: 512
                editable: true
            }

            Label {
                text: qsTr("Address")
                font.bold: true
                color: "#FFFFFF"
                Layout.preferredWidth: 60
            }

            SpinBox {
                from: 0
                to: 512
                editable: true
            }
        }

        Repeater {
            model: modelData.sliders

            delegate: DmxSlider {
                required property var modelData
                required property int index
                Layout.fillWidth: true
                name: modelData.name
                value: modelData.value
                regions: modelData.regions
                regionLabels: fixture.detailed

                onEdited: value => {
                    audioController.setDmxValue(fixture.index, index, value)
                }
            }
        }
    }
}