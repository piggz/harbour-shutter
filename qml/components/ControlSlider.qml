import QtQuick 2.0
import "./platform"

SliderPL {
    id: sldControl

    property string title
    property int control
    property real minimim: 0
    property real maximum: 0

    label: title
    minimumValue: (forceUpdate || !forceUpdate) && cameraProxy ? minimim : 0
    maximumValue: (forceUpdate || !forceUpdate) && cameraProxy ? maximum : 0

    Text {
        text: sldControl.value.toFixed(2)
        anchors.centerIn: parent
        color: styler.themePrimaryColor
    }
}
