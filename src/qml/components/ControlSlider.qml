import QtQuick
import QtQuick.Controls

TextSlider {
    id: sldControl

    property string title
    property int control
    property real minimim: 0
    property real maximum: 0

    //label: title
    from: (forceUpdate || !forceUpdate) && cameraProxy ? minimim : 0
    to: (forceUpdate || !forceUpdate) && cameraProxy ? maximum : 0

    Label {
        text: sldControl.value.toFixed(2)
        anchors.centerIn: parent
    }
}
