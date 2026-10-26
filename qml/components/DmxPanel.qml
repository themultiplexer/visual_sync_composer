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
                onClicked: audioController.removeFixture(fixture.index)
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
            }

            SpinBox {
                Layout.preferredWidth: 50
                from: 0
                to: 512

                value: modelData.group

                onValueModified: {
                    audioController.setDmxGroup(fixture.index, value)
                }

                editable: true
            }

            Label {
                text: qsTr("Address")
                font.bold: true
                color: "#FFFFFF"
            }

            SpinBox {
                Layout.preferredWidth: 50
                from: 0
                to: 512

                value: modelData.address

                onValueModified: {
                    audioController.setDmxAddress(fixture.index, value)
                }

                editable: true
            }

            Label {
                text: qsTr("Delay")
                font.bold: true
                color: "#FFFFFF"
            }

            SpinBox {
                Layout.preferredWidth: 50
                from: 0
                to: 512

                value: modelData.delay

                onValueModified: {
                    audioController.setDmxDelay(fixture.index, value)
                }

                editable: true
            }

            Label {
                text: qsTr("Program")
                font.bold: true
                color: "#FFFFFF"
            }

            SpinBox {
                Layout.preferredWidth: 50
                from: 0
                to: 512

                value: modelData.program

                onValueModified: {
                    audioController.setDmxProgram(fixture.index, value)
                }

                editable: true
            }
        }
        ColumnLayout {
            spacing: 0
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
}