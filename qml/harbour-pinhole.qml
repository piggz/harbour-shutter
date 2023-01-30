import QtQuick 2.0
import QtMultimedia 5.6
import "pages"
import uk.co.piggz.pinhole 1.0

import "./components/"
import "./components/platform"

ApplicationWindowPL {
    id: app
    property bool loadingComplete: false;
    property bool forceUpdate: false;

    Settings {
        id: settings
        property string cameraName
        property int cameraId: 0
        property string captureMode
        property int cameraCount
        property variant enabledCameras: [] //Calculated on startup and when disabledCameras changes
        property string disabledCameras: ""
        property int rotationCorrection: 0
        
        function getCameraValue(s, d) {
            return get(cameraId, s, d);
        }
        function setCameraValue(s, v) {
            set(cameraId, s, v);
        }
        function getCameraModeValue(s, d) {
            return get(cameraId + "_" + captureMode, s, d);
        }
        function setCameraModeValue(s, v) {
            set(cameraId + "_" + captureMode, s, v);
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
            for (var i = 0; i < settings.cameraCount; ++i) {
                if (settings.disabledCameras.indexOf("[" + i + "]") == -1) {
                    settings.enabledCameras.push(i)
                }
            }
            console.log("Disabled Cameras:", settings.disabledCameras);
            console.log("Enabled Cameras :", settings.enabledCameras);

            settings.set("global", "disabledCameras", disabledCameras);
            app.forceUpdate = !app.forceUpdate;
        }

        Component.onCompleted: {
            console.log("Setting up default settings");
            captureMode = get("global", "captureMode", "image");
            cameraName = get("global", "cameraName", "");
            cameraId = get("global", "cameraId", 0);
            disabledCameras = get("global", "disabledCameras", "");

            rotationCorrection = get("global", "rotationCorrection", 0);

            set("global", "cameraId", cameraId);
            set("global", "cameraName", cameraName);
            set("global", "captureMode", captureMode);
            set("global", "rotationCorrection", rotationCorrection);
            settings.set("global", "disabledCameras", disabledCameras);

            cameraCount = modelCamera.rowCount;
        }
    }

    StylerPL {
        id: styler
        themePrimaryColor: "white"
    }

    TruncationModes { id: truncModes }
    DockModes { id: dockModes }

    initialPage: CameraUI {
            id: cameraUI
    }

    Component.onCompleted: {
        cameraUI.startup();
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
