import QtQuick 2.2
import "./platform"

Item {
    id: button
    property alias iconSource: iconButton.iconSource
    property string iconColor
    property int iconRotation: 0
    property int size: styler.themeItemSizeSmall
    //property alias down: iconButton.down
    signal clicked
    signal pressed

    height: size
    width: size

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: styler.colorScheme === styler.LightOnDark ? "black" : "white"
        opacity: 0.7

        IconButtonPL {
            id: iconButton
            anchors.centerIn: parent
            anchors.fill: parent
            rotation: button.iconRotation
            //icon.anchors.fill: icon.parent
            //icon.anchors.margins: styler.paddingMedium
            //icon.source: button.image
            //icon.fillMode: Image.PreserveAspectFit
            onClicked: {
                console.log("button clicked")
                button.clicked()
            }
            //onPressed: button.pressed()
            iconWidth: (parent.width / 4) * 3
            iconHeight: iconWidth
        }
    }
}
