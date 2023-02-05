import QtQuick 2.0
import "./platform"

SliderPL {
    id: sldControl

    property string title
    property int control
    property bool forceUpdate: app.forceUpdate

    label: title
    width: parent.width
    visible: (forceUpdate || !forceUpdate) && cameraProxy ? cameraProxy.controlExists(control) : false
    minimumValue: (forceUpdate || !forceUpdate) && cameraProxy ? cameraProxy.controlMin(control) : 0
    maximumValue: (forceUpdate || !forceUpdate) && cameraProxy ? cameraProxy.controlMax(control) : 0

    Text {
        text: sldControl.value.toFixed(2)
        anchors.centerIn: parent
        color: styler.themePrimaryColor
    }

    onValueChanged: {
        if (cameraProxy) {
            cameraProxy.setControlValue(control, value);
        }
    }
}
