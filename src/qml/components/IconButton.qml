
import QtQuick 2.11
import QtQuick.Controls 2.4
// for IconImage, see https://bugreports.qt.io/browse/QTBUG-66829
import QtQuick.Controls.impl 2.4

Item {
    id: item
    height: (iconName ? iconimage.height : image.height)*(1 + padding)
    width: (iconName ? iconimage.width : image.width)*(1 + padding)

    property bool   iconColorize: true
    property int    iconHeight: 0
    property alias  iconName: iconimage.name
    property real   iconOpacity: 1.0
    property real   iconRotation
    property alias  iconSource: image.source
    property int    iconWidth: 0
    property real   padding: 0.5
    property alias  pressed: mouse.pressed

    signal clicked

    IconImage {
        id: iconimage
        anchors.centerIn: parent
        color: iconColorize ? styler.themeHighlightColor : "transparent"
        opacity: iconOpacity
        rotation: iconRotation
        smooth: false
        sourceSize.height: iconHeight
        sourceSize.width: iconWidth
        visible: name
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: image
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        opacity: iconOpacity
        rotation: iconRotation
        sourceSize.height: iconHeight
        sourceSize.width: iconWidth
        visible: source && !iconName
    }

    Rectangle {
        anchors.fill: parent
        color: mouse.pressed ? styler.themePrimaryColor : "transparent"
        opacity: 0.2
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        onClicked: item.clicked()
    }
}
