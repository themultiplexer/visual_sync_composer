import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    id: root

    required property var presets
    property int firstIndex: 0
    property int activeIndex: -1
    property bool editable: true

    signal activated(int index)
    signal renameRequested(int index)
    signal moveRequested(int index)
    signal colorRequested(int index)

    columns: 4
    rows: 4
    rowSpacing: 6
    columnSpacing: 6

    Repeater {
        model: 16

        delegate: Rectangle {
            id: tile

            required property int index
            readonly property int presetIndex: root.firstIndex + index
            readonly property var preset: presetIndex < root.presets.length
                                          ? root.presets[presetIndex]
                                          : null

            Layout.preferredWidth: 76
            Layout.preferredHeight: 68
            radius: 5
            color: preset && preset.color ? preset.color : "#3a3d43"
            border.width: root.activeIndex === presetIndex ? 3 : 1
            border.color: root.activeIndex === presetIndex ? "white" : "#747982"

            Label {
                anchors.fill: parent
                anchors.margins: 5
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                text: tile.preset && tile.preset.name
                      ? tile.preset.name
                      : qsTr("Preset %1").arg(tile.presetIndex + 1)
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: root.activated(tile.presetIndex)
                onLongPressed: root.renameRequested(tile.presetIndex)
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                enabled: root.editable
                onTapped: contextMenu.popup()
                onLongPressed: contextMenu.popup()
            }

            Menu {
                id: contextMenu
                MenuItem {
                    text: qsTr("Move…")
                    onTriggered: root.moveRequested(tile.presetIndex)
                }
                MenuItem {
                    text: qsTr("Change color…")
                    onTriggered: root.colorRequested(tile.presetIndex)
                }
                MenuItem {
                    text: qsTr("Rename/save…")
                    onTriggered: root.renameRequested(tile.presetIndex)
                }
            }
        }
    }
}
