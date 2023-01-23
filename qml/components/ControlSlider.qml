import QtQuick 2.0
import "./platform"

SliderPL {
    id: sldControl

    property string title
    property int control

    label: title
    width: parent.width
    visible: cameraProxy.controlExists(control);
    minimumValue: cameraProxy.controlMin(control);
    maximumValue: cameraProxy.controlMax(control);

    Text {
        text: sldControl.value.toFixed(2)
        anchors.centerIn: parent
        color: styler.themePrimaryColor
    }

    onValueChanged: {
        cameraProxy.setControlValue(control, value);
    }
}
