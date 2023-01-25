import QtQuick 2.0
import "./platform"

SliderPL {
    id: sldControl

    property string title
    property int control
    property bool forceUpdate: app.forceUpdate

    label: title
    width: parent.width
    visible: (forceUpdate || !forceUpdate) ? cameraProxy.controlExists(control) : false
    minimumValue: (forceUpdate || !forceUpdate) ? cameraProxy.controlMin(control) : 0
    maximumValue: (forceUpdate || !forceUpdate) ? cameraProxy.controlMax(control) : 0

    Text {
        text: sldControl.value.toFixed(2)
        anchors.centerIn: parent
        color: styler.themePrimaryColor
    }

    onValueChanged: {
        cameraProxy.setControlValue(control, value);
    }
}
