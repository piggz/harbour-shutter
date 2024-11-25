import QtQuick
import QtQuick.Controls

DockedPanel {
    id: panel

    property alias model: listView.model
    property var selectedItem
    signal clicked(var value)

    modal: true
    dock: dockModes.left
    width: parent.width / 2
    height: parent.height
    z: 99

    clip: true

    ListView {
        id: listView
        anchors.fill: parent
        clip: true

        delegate: ItemDelegate {
            highlighted: value === selectedItem
            width: ListView.view.width
            text: name

            onClicked: {
                panel.clicked(value)
            }
        }
    }

}
