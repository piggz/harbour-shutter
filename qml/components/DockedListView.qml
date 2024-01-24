import QtQuick 2.0
import "./platform"

DockedPanelPL {
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

        ListViewPL {
            id: listView
            anchors.fill: parent
            clip: true

            delegate: ListItemPL {
                highlighted: value === selectedItem

                LabelPL {
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
