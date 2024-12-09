import QtQuick
import QtQuick.Controls

Item {
    id: item
    height: sl.height + lbl.height + sl.anchors.topMargin
    width: parent.width

    property alias value: sl.value
    property alias label: lbl.text
    property alias from: sl.from
    property alias to: sl.to
    property alias stepSize: sl.stepSize
    property real leftMargin // ignoring this property

    Label {
        id: lbl
        anchors.left: parent.left
        anchors.right: parent.right
        height: text ? implicitHeight : 0
        visible: text
        wrapMode: Text.WordWrap
    }

    Slider {
        id: sl
        width: parent.width
        height: styler.themeItemSizeMedium
        anchors.top: lbl.bottom

        Text {
            text: sl.value
            anchors.centerIn: parent
        }
    }
}
