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
        property string cameraIndex
        property string captureMode
        property variant enabledCameras: [] //Calculated on startup and when disabledCameras changes

        function getCameraValue(s, d) {
            return get(cameraIndex, s, d);
        }
        function setCameraValue(s, v) {
            set(cameraIndex, s, v);
        }
        function getCameraModeValue(s, d) {
            return get(cameraIndex + "/" + captureMode, s, d);
        }
        function setCameraModeValue(s, v) {
            set(cameraIndex + "/" + captureMode, s, v);
        }
        function strToSize(siz) {
            var w = parseInt(siz.substring(0, siz.indexOf("x")))
            var h = parseInt(siz.substring(siz.indexOf("x") + 1))
            return Qt.size(w, h)
        }

        function sizeToStr(siz) {
            return siz.width + "x" + siz.height
        }
        //Return either the current mode resolution or default resolution for that mode
        function resolution(mode) {
            if (settings.captureMode === mode
                    && settings.mode.resolution !== "") {
                var res = strToSize(settings.mode.resolution)
                if (modelResolution.isValidResolution(res, mode)) {
                    return res
                }
            }
            return modelResolution.defaultResolution(mode)
        }

        function calculateEnabledCameras()
        {
            settings.enabledCameras = []
            for (var i = 0; i < globalSettings.cameraCount; ++i) {
                if (settings.global.disabledCameras.indexOf("[" + QtMultimedia.availableCameras[i].deviceId + "]") == -1) {
                    settings.enabledCameras.push(QtMultimedia.availableCameras[i].deviceId)
                }
            }
        }
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
