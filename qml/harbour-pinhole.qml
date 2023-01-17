import QtQuick 2.0
import QtMultimedia 5.6
import "pages"
import uk.co.piggz.pinhole 1.0

import "./components/"
import "./components/platform"

ApplicationWindowPL {
    id: app
    property bool loadingComplete: false;
    Settings {
        id: settings
    }

    StylerPL {
        id: styler
    }
    TruncationModes { id: truncModes }
    DockModes { id: dockModes }

    Rectangle {
        anchors.fill: parent
        z: -10
        color: "black"
    }

    initialPage: CameraUI {
            id: cameraUI
    }

    Component.onCompleted: {
        loadingComplete = true;
    }

    /*
    onApplicationActiveChanged: {
        if (Qt.application.state == Qt.ApplicationActive) {
            cameraUI.camera.start();
        } else {
            cameraUI.camera.stop();
        }
    }
*/
}
