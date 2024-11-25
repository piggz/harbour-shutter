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
        //anchors.leftMargin: styler.themeHorizontalPageMargin
        anchors.right: parent.right
        //anchors.rightMargin: styler.themeHorizontalPageMargin
        anchors.top: parent.top
        //font.pixelSize: styler.themeFontSizeMedium
    }

    Label {
        id: desc
        anchors.left: parent.left
        //anchors.leftMargin: styler.themeHorizontalPageMargin
        anchors.right: parent.right
        //anchors.rightMargin: styler.themeHorizontalPageMargin
        anchors.top: sw.bottom
        //anchors.topMargin: text ? styler.themePaddingSmall : 0
        //font.pixelSize: styler.themeFontSizeSmall
        height: text ? implicitHeight : 0
        visible: text
        wrapMode: Text.WordWrap
    }
}
