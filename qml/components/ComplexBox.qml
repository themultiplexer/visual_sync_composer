import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    required property var items
    signal clicked(int index, int value)

    ComboBox {
        id: comboboxId
        width: parent.width / 2

        model: root.items
        delegate: Item {
            width: parent.width
            height: 30
            Row {
                spacing: 5
                anchors.fill: parent
                anchors.margins: 5
                CheckBox {
                    id: checkboxId
                    height: parent.height
                    width: height
                    onPressed: checked = !checked
                    onCheckedChanged: {
                        root.clicked(0, 1)
                    }
                }
                Label {
                    text: model.name
                    width: parent.width - checkboxId.width
                    height: parent.height
                    color: "black"
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: Qt.AlignLeft
                }
            }
        }
    }
}