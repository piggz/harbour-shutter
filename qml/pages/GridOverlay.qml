import QtQuick 2.0

import "../components/"
import "../components/platform"

Item {
    id: root

    property double aspect: 16.0 / 9.0
    visible: settings.gridMode != "none"
    anchors.centerIn: parent
    width: parent.width// * aspect
    height: parent.height

    function horizontalLines(id) {
        if (id === "thirds")
            return [0.33, 0.66]
        if (id === "ambience")
            return [0.21, 0.79]
        return []
    }

    function verticalLines(id) {
        if (id === "thirds")
            return [0.33, 0.66]
        if (id === "ambience")
            return [0.2333, 0.7666]
        return []
    }

    Repeater {
        model: horizontalLines(settings.gridMode)

        delegate: Item {
            width: parent.width
            height: 2

            Rectangle {
                id: left
                width: parent.width / 2
                height: 2
                y: root.height * modelData

                color: "#88ffffff"
            }

            Rectangle {
                id: right
                width: parent.width / 2
                height: 2
                y: root.height * modelData
                x: root.width / 2

                color: "#88ffffff"
            }
        }
    }

    Repeater {
        model: verticalLines(settings.gridMode)

        delegate: Item {
            width: 2
            height: parent.height

            Rectangle {
                id: up
                width: 2
                height: parent.height / 2
                x: root.width * modelData

                color: "#88ffffff"
            }

            Rectangle {
                id: bottom
                width: 2
                height: parent.height / 2
                x: root.width * modelData
                y: up.height

                color: "#88ffffff"
            }
        }
    }
}
