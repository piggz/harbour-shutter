import QtQuick
import QtQuick.Controls

// required properties:
//    contentHeight
//    menu
//
// highlighted can be used, if available, to give a feedback that an item is pressed
//
// signals: clicked
//
Item {
    id: main

    height: item.height
    width: parent.width

    property real contentHeight
    property bool highlighted: false
    property var  menu

    signal clicked

    ItemDelegate {
        id: item

        height: contentHeight
        width: parent.width

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            propagateComposedEvents: true
            onClicked: {
                if (!menu || !menu.enabled) return
                menu.x = mouse.x
                menu.y = mouse.y
                menu.visibleChanged.connect(function (){
                    main.highlighted = menu.visible;
                })
                menu.open();
            }
            onPressed: main.highlighted = pressed
            onReleased: main.highlighted = pressed || (!!menu && menu.visible)
        }

        onClicked: parent.clicked()
    }
}
