import QtQuick
import QtQuick.Controls

Item {
    id: item
    height: sw.height + desc.height + desc.anchors.topMargin
    width: parent.width

    property alias checked: sw.checked
    property alias description: desc.text
    property alias text: sw.text

    property real leftMargin // ignoring this property

    Switch {
        id: sw
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
    }

    Label {
        id: desc
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: sw.bottom
        //font.pixelSize: styler.themeFontSizeSmall
        height: text ? implicitHeight : 0
        visible: text
        wrapMode: Text.WordWrap
    }
}
