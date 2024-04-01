import QtQuick 2.0
import QtMultimedia 5.6
import "pages"
import uk.co.piggz.shutter 1.0

import "./components/"
import "./components/platform"

ApplicationWindowPL {
    id: app
    property bool loadingComplete: false;
    property bool forceUpdate: false;
    property int cameraId: 0
    property int cameraCount
    property variant enabledCameras: [] //Calculated on startup and when disabledCameras changes
    property string disabledCameras: ""
    property string captureMode
    property string gridMode: "none"

    function strToSize(siz) {
        var w = parseInt(siz.substring(0, siz.indexOf("x")))
        var h = parseInt(siz.substring(siz.indexOf("x") + 1))
        return Qt.size(w, h)
    }

    function sizeToStr(siz) {
        return siz.width + "x" + siz.height
    }

    Settings {
        id: settings
        
        function setGlobalValue(s, v) {
            if (!loadingComplete) {
                return;
            }
            set("global", s, v);
            forceUpdate = !forceUpdate;
        }

        function getCameraModeValue(s, d) {
            return get(cameraId + "_" + captureMode, s, d);
        }

        function setCameraModeValue(s, v) {
            set(cameraId + "_" + captureMode, s, v);
            forceUpdate = !forceUpdate;
        }

        //Return either the current mode resolution or default resolution for that mode
        function resolution(mode) {
            if (captureMode === mode
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
            enabledCameras = []
            for (var i = 0; i < cameraCount; ++i) {
                if (disabledCameras.indexOf("[" + i + "]") == -1) {
                    enabledCameras.push(i)
                }
            }
            console.log("Disabled Cameras:", disabledCameras);
            console.log("Enabled Cameras :", enabledCameras);

            setGlobalValue("disabledCameras", disabledCameras);
            app.forceUpdate = !app.forceUpdate;
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

    function loadGlobalSettings() {
	settings.set("global", "captureMode", captureMode);
	settings.set("global", "cameraId", cameraId);
	settings.set("global", "disabledCameras", disabledCameras);
	settings.set("global", "gridMode", gridMode);
    }

    function saveGlobalSettings() {
        captureMode = settings.get("global", "captureMode", "image");
        cameraId = settings.get("global", "cameraId", 0);
        disabledCameras = settings.get("global", "disabledCameras", "");
        gridMode = settings.get("global", "gridMode", "none");
    }

    Component.onCompleted: {
        cameraUI.startup();
        loadingComplete = true;
        console.log("Setting up default settings");
        loadGlobalSettings();
        saveGlobalSettings();

        cameraCount = modelCamera.rowCount;
    }

    onRunningChanged: {
        if (!app.active) {
            cameraProxy.stop();
        } else {
            if (pageStack.depth === 1){
                cameraUI.startViewfinder();
            }
        }
    }
}
