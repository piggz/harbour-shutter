import QtQuick
import QtQuick.Controls

DockedPanel {
    id: panel

    property alias model: listView.model
    property var selectedItem
    modal: true

    dock: dockModes.left

    signal clicked(var value)

    width: parent.width / 2
    height: parent.height
    z: 99

    clip: true

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.7

        ListView {
            id: listView
            anchors.fill: parent
            clip: true

            delegate: ListItem {
                highlighted: value === selectedItem

                Label {
                    id: lbl
                    color: highlighted ? styler.themeHighlightColor : styler.themePrimaryColor
                    text: name
                }
                onClicked: {
                    panel.clicked(value)
                }
            }
        }
    }
}
