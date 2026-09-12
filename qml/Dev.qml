import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import "components"

Item {
    anchors.fill: parent

    property bool allowClose: false
    property int editedPresetKind: 0 // 0 = effect, 1 = tube
    property int editedPresetIndex: -1
    property int visualizationMode: 0

    Component.onCompleted: audioController.start()

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: Math.max(1920 - 24, 1060)
            spacing: 10

            Frame {
                Layout.fillWidth: true
                Layout.preferredHeight: 215

                Flickable {
                    anchors.fill: parent
                    contentWidth: tubeRow.implicitWidth
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    Row {
                        id: tubeRow
                        spacing: 8

                        Repeater {
                            model: audioController.tubes

                            delegate: TubeCard {
                                id: tubeCard
                                required property int index
                                required property var modelData

                                height: 185
                                tubeIndex: index
                                tube: modelData

                                onChanged: function(delay, group) {
                                    audioController.updateTube(index, delay, group)
                                }
                                onMoveRequested: function(offset) {
                                    audioController.moveTube(index, index + offset)
                                }
                                onFlashFirmwareRequested:
                                    audioController.putTubeIntoFirmwareUpdateMode(index)
                                onPeakRequested:
                                    audioController.triggerPeak(0, index, true)

                                Connections {
                                    target: audioController
                                    function onTubePeakTriggered(tubeIndex, color, group) {
                                        if (tubeIndex === tubeCard.index)
                                            tubeCard.flash(color)
                                    }
                                    function onAllTubesPeakTriggered(color, group) {
                                        tubeCard.flash(color)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Frame {
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent

                    RowLayout {
                        Layout.fillWidth: true

                        Label { text: qsTr("Group selection"); font.bold: true }
                        ComboBox {
                            model: [qsTr("Count up"), qsTr("Region"), qsTr("Random")]
                            currentIndex: audioController.groupSelectionMode
                            onActivated: audioController.groupSelectionMode = currentIndex
                        }

                        Item { Layout.fillWidth: true }

                        Label { text: qsTr("Color selection"); font.bold: true }
                        ComboBox {
                            model: [qsTr("Count up"), qsTr("Region"), qsTr("Random")]
                            currentIndex: audioController.colorSelectionMode
                            onActivated: audioController.colorSelectionMode = currentIndex
                        }

                        Item { Layout.fillWidth: true }

                        Label { text: qsTr("Color source"); font.bold: true }
                        ComboBox {
                            id: paletteMode
                            model: [
                                qsTr("Maximum frequency"), qsTr("Manual"),
                                qsTr("Random hue"), qsTr("Random hue & saturation"),
                                qsTr("Red / white"), qsTr("Greenish"), qsTr("Custom")
                            ]
                            currentIndex: 4
                            onActivated: {
                                if (currentIndex <= 3)
                                    audioController.colorControlMode = currentIndex
                                else
                                    audioController.selectBuiltInPalette(currentIndex - 4)
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label { text: qsTr("Analysis:"); font.bold: true }
                        Label { text: qsTr("Group %1").arg(audioController.beatGroup) }
                        Label { text: qsTr("%1 bpm").arg(audioController.bpm) }
                        Label {
                            text: qsTr("%1 ms").arg(audioController.meanBeatIntervalMs.toFixed(1))
                        }
                        CheckBox {
                            text: qsTr("Audio filter")
                            checked: audioController.audioFilterEnabled
                            onClicked: audioController.audioFilterEnabled = checked
                        }

                        Item { Layout.fillWidth: true }

                        Label { text: qsTr("FFT visualization"); font.bold: true }
                        ComboBox {
                            model: [qsTr("Exponential mean"), qsTr("Mean"), qsTr("Variance")]
                            currentIndex: visualizationMode
                            onActivated: visualizationMode = currentIndex
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 210

                ColumnLayout {
                    Layout.preferredWidth: 80
                    Label { text: qsTr("Decay"); Layout.alignment: Qt.AlignHCenter }
                    Slider {
                        Layout.fillHeight: true
                        orientation: Qt.Vertical
                        from: 0.001
                        to: 0.1
                        value: audioController.spectrumDecay
                        onMoved: audioController.spectrumDecay = value
                    }
                }

                SpectrumView {
                    id: spectrum
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    leftSpectrum: audioController.leftSpectrum
                    rightSpectrum: audioController.rightSpectrum
                    threshold: audioController.sensitivity
                    decay: audioController.spectrumDecay
                    visualizationMode: visualizationMode
                    onFilterRangeEdited: function(lower, upper) {
                        audioController.setFilterRange(lower, upper)
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 80
                    Label { text: qsTr("Volume"); Layout.alignment: Qt.AlignHCenter }
                    Slider {
                        Layout.fillHeight: true
                        orientation: Qt.Vertical
                        from: 0.01
                        to: 1.0
                        value: audioController.inputVolume
                        onMoved: audioController.inputVolume = value
                    }
                }
            }

            Frame {
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent

                    RowLayout {
                        Layout.fillWidth: true

                        ComboBox {
                            Layout.preferredWidth: 500
                            model: audioController.effectNames
                            currentIndex: audioController.ledMode
                            onActivated: audioController.ledMode = currentIndex
                        }

                        Label { text: qsTr("Modifiers:"); font.bold: true }
                        Repeater {
                            model: [
                                qsTr("Fadeout after peak"), qsTr("No color delay"),
                                qsTr("Reversed"), qsTr("Stickiness"),
                                qsTr("Pseudo random"), qsTr("Sync on peak"),
                                "7", "8"
                            ]
                            delegate: CheckBox {
                                required property int index
                                required property string modelData
                                text: modelData
                                checked: Boolean(audioController.modifiers[index])
                                onClicked: audioController.setModifier(index, checked)
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label { text: qsTr("Auto mode:"); font.bold: true }
                        CheckBox { id: automaticColor; text: qsTr("Color") }
                        CheckBox { id: automaticEffects; text: qsTr("Effect") }
                        CheckBox { id: automaticComposition; text: qsTr("Composition") }

                        Item { Layout.fillWidth: true }

                        Label { text: qsTr("Lock:"); font.bold: true }
                        CheckBox { id: compositionLock; text: qsTr("Composition") }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 12

                Frame {
                    Layout.preferredWidth: 360
                    Layout.alignment: Qt.AlignTop

                    ColumnLayout {
                        anchors.fill: parent
                        LabeledSlider {
                            text: qsTr("Sensitivity")
                            from: 0; to: 100
                            value: audioController.sensitivity * 100
                            onEdited: value => audioController.sensitivity = value / 100
                        }
                        LabeledSlider {
                            text: qsTr("Brightness")
                            from: 0; to: 255
                            value: audioController.brightness
                            onEdited: value => audioController.brightness = Math.round(value)
                        }
                        LabeledSlider {
                            id: saturationSlider
                            text: qsTr("Saturation")
                            from: 0; to: 255
                            value: audioController.peakSaturation * 255
                            onEdited: value => audioController.peakSaturation = value / 255
                        }
                        LabeledSlider {
                            text: qsTr("Speed")
                            from: 1; to: 255
                            value: audioController.speed
                            onEdited: value => audioController.speed = Math.round(value)
                        }
                        LabeledSlider {
                            text: qsTr("Parameter 1")
                            from: 1; to: 255
                            value: audioController.parameter1
                            onEdited: value => audioController.parameter1 = Math.round(value)
                        }
                        LabeledSlider {
                            text: qsTr("Parameter 2")
                            from: 1; to: 255
                            value: audioController.parameter2
                            onEdited: value => audioController.parameter2 = Math.round(value)
                        }
                        LabeledSlider {
                            text: qsTr("Parameter 3")
                            from: 1; to: 255
                            value: audioController.parameter3
                            onEdited: value => audioController.parameter3 = Math.round(value)
                        }
                        LabeledSlider {
                            text: qsTr("Parameter 4")
                            from: 1; to: 255
                            value: 128
                        }
                    }
                }

                Frame {
                    Layout.alignment: Qt.AlignTop

                    ColumnLayout {
                        anchors.fill: parent

                        TabBar {
                            id: bankTabs
                            currentIndex: audioController.activeBank
                            onCurrentIndexChanged: {
                                if (currentIndex !== audioController.activeBank)
                                    audioController.activeBank = currentIndex
                            }
                            Repeater {
                                model: 5
                                TabButton { required property int index; text: qsTr("Bank %1").arg(index) }
                            }
                        }

                        PresetGrid {
                            presets: audioController.effectPresets
                            firstIndex: audioController.activeBank * 16
                            activeIndex: audioController.activeEffectPreset
                            onActivated: function(index) {
                                automaticEffects.checked = false
                                audioController.selectEffectPreset(index)
                            }
                            onRenameRequested: index => openRenameDialog(0, index)
                            onMoveRequested: index => openMoveDialog(0, index)
                            onColorRequested: index => openColorDialog(0, index)
                        }
                    }
                }

                Frame {
                    Layout.alignment: Qt.AlignTop
                    ColumnLayout {
                        anchors.fill: parent
                        Label { text: qsTr("Fixture group"); font.bold: true }
                        PresetGrid {
                            presets: audioController.fixturePresets
                            activeIndex: audioController.activeFixtureGroup
                            editable: false
                            onActivated: index => audioController.selectFixtureGroup(index)
                            onRenameRequested: function(index) {}
                            onMoveRequested: function(index) {}
                            onColorRequested: function(index) {}
                        }
                    }
                }

                Frame {
                    Layout.alignment: Qt.AlignTop

                    ColumnLayout {
                        anchors.fill: parent

                        RowLayout {
                            Repeater {
                                model: Math.min(4, audioController.palette.length)
                                PaletteKnob {
                                    required property int index
                                    readonly property var entry: audioController.palette[index]
                                    hue: entry.hue
                                    saturation: entry.saturation
                                    onEdited: function(hue, saturation) {
                                        audioController.setPaletteEntry(index, hue, saturation)
                                    }
                                }
                            }
                        }

                        Label { text: qsTr("Tube coordination"); font.bold: true }
                        PresetGrid {
                            presets: audioController.tubePresets
                            activeIndex: audioController.activeTubePreset
                            onActivated: function(index) {
                                automaticComposition.checked = false
                                audioController.selectTubePreset(index)
                            }
                            onRenameRequested: index => openRenameDialog(1, index)
                            onMoveRequested: index => openMoveDialog(1, index)
                            onColorRequested: index => openColorDialog(1, index)
                        }
                    }
                }

                Button {
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 100
                    text: qsTr("PEAK")
                    font.bold: true
                    onPressed: audioController.triggerPeak()
                }
            }
        }
    }

    Timer {
        interval: 100
        running: true
        repeat: true
        onTriggered: audioController.processAutomaticModes(
                         automaticEffects.checked,
                         automaticComposition.checked)
    }

    Window {
        id: fullscreenSpectrum
        title: qsTr("Spectrum")
        color: "black"

        SpectrumView {
            anchors.fill: parent
            leftSpectrum: audioController.leftSpectrum
            rightSpectrum: audioController.rightSpectrum
            threshold: audioController.sensitivity
            decay: audioController.spectrumDecay
            visualizationMode: visualizationMode
            filterLower: spectrum.filterLower
            filterUpper: spectrum.filterUpper
            onFilterRangeEdited: function(lower, upper) {
                spectrum.filterLower = lower
                spectrum.filterUpper = upper
                audioController.setFilterRange(lower, upper)
            }
        }

    /*
        Shortcut {
            sequence: StandardKey.Cancel
            onActivated: fullscreenSpectrum.close()
        }
        */
    }

    FileDialog {
        id: firmwareDialog
        title: qsTr("Open tube firmware file")
        nameFilters: [qsTr("Binary files (*.bin)")]
        onAccepted: audioController.flashFirmware(selectedFile)
    }

    Dialog {
        id: renameDialog
        title: editedPresetKind === 0
               ? qsTr("Save effect preset") : qsTr("Save tube preset")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        onOpened: nameField.forceActiveFocus()
        onAccepted: {
            if (editedPresetKind === 0)
                audioController.saveEffectPreset(editedPresetIndex, nameField.text)
            else
                audioController.saveTubePreset(editedPresetIndex, nameField.text)
        }
        TextField {
            id: nameField
            width: 320
            placeholderText: qsTr("Preset name")
            selectByMouse: true
        }
    }

    Dialog {
        id: moveDialog
        title: qsTr("Move preset")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            if (editedPresetKind === 0)
                audioController.moveEffectPreset(editedPresetIndex, destination.value)
            else
                audioController.moveTubePreset(editedPresetIndex, destination.value)
        }
        RowLayout {
            Label { text: qsTr("Destination index:") }
            SpinBox {
                id: destination
                from: 0
                to: editedPresetKind === 0
                    ? audioController.effectPresets.length - 1
                    : audioController.tubePresets.length - 1
                editable: true
            }
        }
    }

    ColorDialog {
        id: presetColorDialog
        title: qsTr("Preset color")
        onAccepted: {
            if (editedPresetKind === 0)
                audioController.setEffectPresetColor(editedPresetIndex, selectedColor)
            else
                audioController.setTubePresetColor(editedPresetIndex, selectedColor)
        }
    }

    Dialog {
        id: errorDialog
        title: qsTr("Error")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok
        property alias message: errorLabel.text
        Label { id: errorLabel; wrapMode: Text.Wrap; width: 380 }
    }

    Connections {
        target: audioController
        function onErrorOccurred(message) {
            errorDialog.message = message
            errorDialog.open()
        }
    }

    function openRenameDialog(kind, index) {
        editedPresetKind = kind
        editedPresetIndex = index
        const presets = kind === 0
                      ? audioController.effectPresets
                      : audioController.tubePresets
        nameField.text = presets[index].name || ""
        renameDialog.open()
    }

    function openMoveDialog(kind, index) {
        editedPresetKind = kind
        editedPresetIndex = index
        destination.value = index
        moveDialog.open()
    }

    function openColorDialog(kind, index) {
        editedPresetKind = kind
        editedPresetIndex = index
        const presets = kind === 0
                      ? audioController.effectPresets
                      : audioController.tubePresets
        presetColorDialog.selectedColor = presets[index].color
        presetColorDialog.open()
    }
}