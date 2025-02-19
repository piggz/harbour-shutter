import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtSensors

import "../components"
import uk.co.piggz.shutter 1.0

Item {
    id: page

    property alias camera: camera
    property bool _completed: false
    property bool _focusAndSnap: false
    property bool _loadParameters: true
    property bool _recordingVideo: false
    property bool _manualModeSelected: false
    readonly property real zoomStepSize: 0.05
    readonly property real zoomStepButton: 5.0
    property bool _nativePortrait: Screen.primaryOrientation == Qt.PortraitOrientation ? true : false
    property int controlsRotation: Screen.primaryOrientation == Qt.PortraitOrientation ? 0 : 90
    property int iconRotation: 0;

    // Use easy device orientation values
    // 0=unknown, 1=portrait, 2=portrait inverted, 3=landscape, 4=landscape inverted
    property int _orientation: OrientationReading.TopUp

    Keys.onPressed: {
        console.log(event.key);
        if (event.isAutoRepeat) {
            return
        }
        if (event.key === Qt.Key_VolumeUp) {
            cameraUI.volUp();
        } else if (event.key === Qt.Key_VolumeDown) {
            cameraUI.volDown();
        } else if (event.key === Qt.Key_Camera) {
            console.log("Camera key");
        }
    }

    /*
    Rectangle {
        anchors.fill: parent
        z: -10
        color: "black"
    }
*/
    OrientationSensor {
        id: orientationSensor
        active: !settings.useSizeAsOrientation

        onReadingChanged: {
            console.log("Orientation changed: ", reading.orientation);
            updateRotation(reading.orientation);
        }
    }

    /*
    ViewFinder3D {
        id: viewFinder
        anchors.centerIn: parent
        height:5
        width: 5
        x: 0
        z:-5

        //transform: [
        //    Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: viewFinder.width / 2; origin.y: viewFinder.height / 2; },
        //    Translate { id: txOut; x: -viewFinder.width / 2; y: -viewFinder.height / 2 },
        //    Scale { id: scale; },
        //    Translate { id: txIn; x: viewFinder.width / 2; y: viewFinder.height / 2 }
        //]

        Rectangle {
            id: rectFlash
            anchors.fill: parent
            opacity: 0

            NumberAnimation on opacity {
                id: animFlash
                from: 1.0
                to: 0.0
                duration: 200
            }
        }
    }*/

    /*
    ViewFinder2D {
        id: viewFinder;
        //anchors.centerIn: parent
        width: parent.width / 2
        height: parent.height / 2
        z:-5

        Rectangle {
            id: rectFlash
            anchors.fill: parent
            opacity: 0

            NumberAnimation on opacity {
                id: animFlash
                from: 1.0
                to: 0.0
                duration: 200
            }
        }


    }
*/
    ViewFinderGL {
        id: viewFinderGL
        anchors.fill: parent
        z: -100

        Rectangle {
            id: rectFlash
            anchors.fill: parent
            opacity: 0

            NumberAnimation on opacity {
                id: animFlash
                from: 1.0
                to: 0.0
                duration: 200
            }
        }
    }


    /*
    PositionSource {
        id: positionSource

        active: settings.global.locationMetadata

        onActiveChanged: {
            // PositionSource is activated a moment after initialization
            // regardless "active" property assignment. It looks like Qt bug.
            // Code below workaround it.
            console.log("positionSource.active: " + positionSource.active)
            if (positionSource.active != settings.global.locationMetadata) {
                if (settings.global.locationMetadata) {
                    start();
                } else {
                    stop();
                }
            }
        }

        updateInterval: 1000 // ms
    }
    */

    // Orientation sensors for primary (back camera) & secondary (front camera)
    readonly property var _rotationValues: {
        "primary": [270, 270, 90, 180, 0, 270, 270],
        "secondary"//Uses orientation sensor value 0-6
        : [90, 90, 270, 180, 0, 90, 90],
        "ui": [0, 90, 270, 0, 0, 0, 0, 0, 180], //Uses enum value 1,2,4,8
        "uil": [0, 00, 0, 0, 0, 0, 0, 0, 0], //Uses enum value 1,2,4,8
        "icon": [0, 270, 90, 0, 0, 0, 0, 0, 0]
    }

    readonly property int viewfinderOrientation: {
        var rotation = 90
        switch (_orientation) {
        case OrientationReading.RightUp:
            rotation = 0
            break
        case OrientationReading.TopDown:
            rotation = 270
            break
        case OrientationReading.LeftUp:
            rotation = 180
            break
        }

        return (720 + rotation) % 360
    }

    RotationAnimation on controlsRotation {
        running: _orientation === OrientationReading.TopUp
        to: 270
        duration: 200
        direction: RotationAnimation.Shortest
    }
    RotationAnimation on controlsRotation {
        running: _orientation === OrientationReading.TopDown
        to: 90
        duration: 200
        direction: RotationAnimation.Shortest
    }
    RotationAnimation on controlsRotation {
        running: _orientation === OrientationReading.LeftUp
        to: 180
        duration: 200
        direction: RotationAnimation.Shortest
    }
    RotationAnimation on controlsRotation {
        running: _orientation === OrientationReading.RightUp
        to: 0
        duration: 200
        direction: RotationAnimation.Shortest
    }

    focus: true
    Item {
        id: camera
        property real digitalZoom: 1
        property real maximumDigitalZoom: 1

        function start(){}
        function stop(){}
    }

    Item {
        id: controlsOuter
        anchors.fill: parent

        SettingsOverlay {
            id: settingsOverlay
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            rotation:  controlsContainer.rotation
            iconRotation: page.iconRotation
            onRotationChanged: {
                console.log("Control rotation:", page._orientation, page.controlsRotation, settingsOverlay.rotation, width, height, page.width, page.height);
                console.log(OrientationReading.TopUp, OrientationReading.TopDown, OrientationReading.LeftUp, OrientationReading.RightUp);
            }

            width: rotation == 0 ? parent.width : parent.height
            height: rotation == 0 ? parent.height : parent.width

        }

        Item {
            id: controlsContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            width: rotation == 0 ? parent.width : parent.height
            height: rotation == 0 ? parent.height : parent.width

            GridOverlay {
                aspect: ratio(settings.getCameraModeValue("resolution", Qt.size(1280, 720)))

                function ratio(resolution) {
                    return resolution.width / resolution.height
                }
            }

            Slider {
                id: zoomSlider
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: parent.width * 0.75
                from: 1
                to: camera.maximumDigitalZoom
                value: camera.digitalZoom
                stepSize: zoomStepSize
                visible: camera.maximumDigitalZoom > 1
                rotation: {
                    // Zoom slider should be slide up to zoom in
                    if (_orientation === OrientationReading.TopUp)
                        return -180
                    else if (_orientation === OrientationReading.TopDown)
                        return 0
                    else if (_orientation === OrientationReading.LeftUp)
                        return 180
                    else if (_orientation === OrientationReading.RightUp)
                        return 0
                }

                onValueChanged: {
                    if (value != camera.digitalZoom) {
                        //TODO camera.digitalZoom = value

                    }
                }
            }

            Image {
                id: photoPreview
                rotation: page.controlsRotation
                onStatusChanged: {
                    if (photoPreview.status === Image.Ready) {
                        console.log('photoPreview ready')
                    }
                }
            }

            RoundButton {
                id: btnCapture

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: styler.themePaddingLarge

                size: styler.themeItemSizeLarge
                rotation: page.controlsRotation

                iconSource: shutterIcon()
                onClicked: doShutter()
            }

            Column {
                id: grdOnscreenControls
                spacing: styler.themePaddingMedium
                rotation: _nativePortrait ? page.controlsRotation : 0
                height: childrenRect.height

                anchors.horizontalCenter: {
                    if (((_orientation === OrientationReading.TopUp)
                         || (_orientation === OrientationReading.TopDown)) && _nativePortrait)
                        return parent.right
                    else
                        return parent.horizontalCenter
                }

                anchors.verticalCenter: {
                    if (((_orientation === OrientationReading.TopUp)
                         || (_orientation === OrientationReading.TopDown)) && _nativePortrait)
                        return parent.verticalCenter
                    else
                        return parent.top
                }

                anchors.verticalCenterOffset: {
                    if (((_orientation === OrientationReading.TopUp)
                         || (_orientation === OrientationReading.TopDown)) && _nativePortrait)
                        return 0
                    else
                        return styler.themeItemSizeLarge
                }


                anchors.horizontalCenterOffset: {
                    if (((_orientation === OrientationReading.TopUp)
                         || (_orientation === OrientationReading.TopDown)) && _nativePortrait)
                        return -(btnCapture.width + height)
                    else
                        return 0
                }

                Label {
                    color: "white"
                    text: qsTr("  Camera: ") + modelCamera.get(settings.cameraId) + "(" + settings.cameraId +")" + " - " + settings.sizeToStr(settings.getCameraModeValue("resolution", Qt.size(1280, 720))) + "  "
                    background: Rectangle {
                        color: "black"
                        radius: 4
                    }
                }
            }

            RoundButton {
                id: btnGallery

                visible: galleryModel.count > 0
                enabled: visible

                anchors.top: btnCameraSwitch.bottom
                anchors.bottomMargin: styler.paddingMedium
                anchors.right: parent.right
                anchors.rightMargin: styler.themePaddingMedium
                iconRotation: page.controlsRotation

                size: styler.themeItemSizeSmall

                iconSource: styler.customIconPrefix +"../pics/icon-m-image.svg"

                onClicked: {
                    cameraProxy.stop()
                    pageStack.push(Qt.resolvedUrl("GalleryUI.qml"), {
                                       "fileList": galleryModel
                                   })
                }
            }

            RoundButton {
                id: btnCameraSwitch
                iconSource: styler.customIconPrefix + "../pics/icon-camera-switch.svg"
                visible: (forceUpdate || !forceUpdate) ? settings.enabledCameras.length > 1 : false
                iconRotation: page.controlsRotation
                property string prevCamId
                anchors {
                    top: parent.top
                    topMargin: styler.themePaddingMedium
                    right: parent.right
                    rightMargin: styler.themePaddingMedium
                }
                onClicked: {
                    switchToNextCamera()
                }
            }

            IconSwitch {
                id: btnModeSwitch
                visible: false //Disable until video recording is implemented
                anchors.bottom: parent.bottom
                anchors.bottomMargin: styler.themePaddingMedium
                anchors.right: parent.right
                anchors.rightMargin: (rotation === 90
                                      || rotation === 270) ? styler.themePaddingLarge
                                                             * 2 : styler.themePaddingMedium
                rotation: _nativePortrait ? page.controlsRotation : 0
                width: styler.themeItemSizeSmall

                icon1Source: styler.customIconPrefix + "../pics/icon-m-camera.svg"
                icon2Source: styler.customIconPrefix + "../pics/icon-m-video.svg"
                button1Name: "image"
                button2Name: "video"

                onClicked: {
                    console.log("selected:", name)
                    camera.stop()
                    settingsOverlay.setMode(name)
                    if (name === button1Name) {
                        camera.captureMode = Camera.CaptureStillImage
                    } else {
                        camera.captureMode = Camera.CaptureVideo
                    }
                    camera.start()
                }
            }

        }
        //End controlsContainer
    }

    MouseArea {
        id: mouseFocusArea
        anchors.fill: parent
        z: -1 //Send to back
        onClicked: {

            if (settingsOverlay.panelOpen) {
                settingsOverlay.hideAllPanels()
                return
            }

            // If in auto or macro focus mode, focus on the specified point
            if (camera.focus.focusMode === Camera.FocusAuto
                    || camera.focus.focusMode === Camera.FocusMacro
                    || camera.focus.focusMode === Camera.FocusContinuous) {
                var focusPoint
                switch ((360 - viewfinderOrientation) % 360) {
                case 90:
                    focusPoint = Qt.point(mouse.y, width - mouse.x)
                    break
                case 180:
                    focusPoint = Qt.point(width - mouse.x, height - mouse.y)
                    break
                case 270:
                    focusPoint = Qt.point(height - mouse.y, mouse.x)
                    break
                default:
                    focusPoint = Qt.point(mouse.x, mouse.y)
                    break
                }

                // Normalize the focus point.
                focusPoint.x = focusPoint.x / Math.max(page.width, page.height)
                focusPoint.y = focusPoint.y / Math.min(page.width, page.height)

                camera.focus.focusPointMode = Camera.FocusPointCustom
                camera.focus.setCustomFocusPoint(focusPoint)
                camera.unlock()
            }
            //TODO camera.searchAndLock()
            if (!_manualModeSelected) focusPointTimer.restart()
        }
    }

    /*
    Rectangle {
        id: focusCircle
        height: (camera.lockStatus === Camera.Locked) ? styler.themeItemSizeSmall : styler.themeItemSizeMedium
        width: height
        radius: width / 2
        border.width: 4
        border.color: focusColor()
        color: "transparent"
        visible: camera.focus.focusPointMode === Camera.FocusPointCustom

        x: {
            var ret = 0
            switch ((360 - viewfinderOrientation) % 360) {
            case 90:
                ret = page.width - camera.focus.customFocusPoint.y * page.width
                break
            case 180:
                ret = page.width - camera.focus.customFocusPoint.x * page.width
                break
            case 270:
                ret = camera.focus.customFocusPoint.y * page.width
                break
            default:
                ret = camera.focus.customFocusPoint.x * page.width
                break
            }
        }

        y: {
            var ret = 0
            switch ((360 - viewfinderOrientation) % 360) {
            case 90:
                ret = camera.focus.customFocusPoint.x * page.height
                break
            case 180:
                ret = page.height - camera.focus.customFocusPoint.y * page.height
                break
            case 270:
                ret = page.height - camera.focus.customFocusPoint.x * page.height
                break
            default:
                ret = camera.focus.customFocusPoint.y * page.height
                break
            }
        }

        transform: Translate {
            x: -focusCircle.width / 2
            y: -focusCircle.height / 2
        }
    }
*/

    function startup() {
        console.log("Orientations:", OrientationReading.TopUp, OrientationReading.TopDown, OrientationReading.LeftUp, OrientationReading.RightUp)
        console.log("Orientation: ", _orientation, controlsRotation, _nativePortrait);

        updateRotation(orientationSensor.reading ? orientationSensor.reading.orientation : 0);

        cameraProxy.setViewFinder(viewFinderGL);
        cameraProxy.setFaceDetectionEnabled(settings.faceDetection);

        for( var i = 0; i < modelCamera.rowCount; i++ ) {
            console.log("Camera: ", modelCamera.get(i) );
        }

        app.forceUpdate = !app.forceUpdate;
        _completed = true
    }

    Connections {
        target: pageStack

        onDepthChanged: {
            if (pageStack.depth === 1) {
                console.log("Calling camera.start() due to pageStack change")
                startViewfinder();
            }
        }
    }

    Connections {
        target: cameraProxy

        onStillCaptureFinished: {
            console.log("Still capture finished, starting viewfinder timer");
            cameraProxy.stop();
            startViewfinder();

            console.log("Camera: image saved", path)
            galleryModel.append({
                                    "filePath": "file://" + path,
                                    "isVideo": false
                                })

        }
    }

    Timer {
        id: tmrStartViewfinder
        interval: 500
        onTriggered: {
            cameraProxy.startViewFinder();
        }
    }

    ListModel {
        id: galleryModel
    }

    ListModel {
        id: viewfinderResolutionModel
    }

    Timer {
        id: tmrDelayedStart
        repeat: false
        running: true
        interval: 200
        onTriggered: {
            console.log("camera delayed start", settings.cameraId)
            _loadParameters = true

            settings.calculateEnabledCameras()

            console.log(settings.enabledCameras, settings.enabledCameras.length);

            cameraProxy.setCameraIndex(modelCamera.get(settings.cameraId));

            var f = settings.getCameraModeValue("format", modelFormats.defaultFormat());
            settings.setCameraModeValue("format", f);
            cameraProxy.setStillFormat(f);

            var r = settings.getCameraModeValue("resolution", modelResolution.defaultResolution(settings.captureMode));
            settings.setCameraModeValue("resolution", r);

            console.log(f, r);
            cameraProxy.setResolution(r);

            applySettings();

            startViewfinder();
        }
    }

    Timer {
        id: focusPointTimer
        interval: 7000
        onTriggered: {
            //Set the focus point back to centre
            camera.focus.setFocusPointMode(Camera.FocusPointAuto)
            // and unlock camera so AF is working again
            camera.unlock()
            if (camera.focus.focusMode === Camera.FocusAuto) camera.searchAndLock()
        }
    }

    function startViewfinder() {
        tmrStartViewfinder.start();
    }

    function volUp() {
        if (settings.global.swapZoomControl) {
            zoomOut()
        } else {
            zoomIn()
        }
    }

    function volDown() {
        if (settings.global.swapZoomControl) {
            zoomIn()
        } else {
            zoomOut()
        }
    }

    function cameraStatusStr() {
        switch(camera.cameraStatus){
        case Camera.ActiveStatus:
            return "Active"
        case Camera.StartingStatus:
            return "Starting"
        case Camera.StoppingStatus:
            return "Stopping"
        case Camera.StandbyStatus:
            return "Standby"
        case Camera.LoadedStatus:
            return "Loaded"
        case Camera.LoadingStatus:
            return "Loading"
        case Camera.UnloadingStatus:
            return "Unloading"
        case Camera.UnloadedStatus:
            return "Unloaded"
        case Camera.UnavailableStatus:
            return "Unavailable"
        default:
            return "unknown (" + camera.cameraStatus + ")"
        }
    }

    function focusStr(focus) {
        // TODO: It's possible to combine multiple Camera::FocusMode values, for example FocusMacro + FocusContinuous.
        switch (focus) {
        case CameraFocus.FocusManual:
            return "Manual"
        case CameraFocus.FocusHyperfocal:
            return "Hyperfocal"
        case CameraFocus.FocusInfinity:
            return "Infinity"
        case CameraFocus.FocusAuto:
            return "Auto"
        case CameraFocus.FocusContinuous:
            return "Continuous"
        case CameraFocus.FocusMacro:
            return "Macro"
        default:
            return "unknown (" + focus + ")"
        }
    }

    function applySettings() {
        console.log("Applying settings in", settings.captureMode,
                    "mode for", modelCamera.get(settings.cameraId))

    }

    function setFocusMode(focus) {
        var requestedFocus = focus === Camera.FocusManual ? Camera.FocusAuto : focus
        if (!camera.focus.isFocusModeSupported(requestedFocus)) {
            console.log("focus mode " + focusStr(requestedFocus) +
                        " is not supported, keeping " + focusStr(camera.focus.focusMode))
            return
        }
        console.log("setting focus mode " +
                    focusStr(camera.focus.focusMode) + " -> " + focusStr(focus))

        if (focus === Camera.FocusManual) {
            _manualModeSelected = true
        } else {
            _manualModeSelected = false
        }
        if (camera.focus.focusMode !== requestedFocus) {
            camera.stop()
            camera.focus.setFocusMode(requestedFocus)
            camera.start()
        }
        camera.unlock() // Do not forget to unlock camera when changing focus mode
        settings.mode.focus = focus

        //Set the focus point back to centre
        camera.focus.setFocusPointMode(Camera.FocusPointAuto)

        // Do not lock focus when continuous focus is declared // TODO: We need to allow combination of continous with Auto + Macro
        if (focus !== Camera.FocusContinuous && focus !== Camera.FocusManual) {
            camera.searchAndLock()
        }
    }

    function getNearestViewFinderResolution() {

        /// Tries to find the most correct ViewFinder resolution
        /// for the selected camera settings
        ///
        /// In order of preference:
        ///  * viewFinderResolution for the nearest aspect ratio as set in jolla-camera's dconf settings
        ///  * viewFinderResolution as set in jolla-camera's dconf settings
        ///  * Best match from camera.supportedViewfinderResolutions() that fit to screen and have the same aspect ratio
        ///  * device resolution

        var currentRatioSize = modelResolution.sizeToRatio(
                    settings.resolution(settings.global.captureMode))
        var currentRatio = currentRatioSize.height
                > 0 ? currentRatioSize.width / currentRatioSize.height : 0
        if (currentRatio > 0) {
            if (currentRatio <= 4.0 / 3
                    && settings.jollaCamera.viewfinderResolution_4_3) {
                return settings.strToSize(
                            settings.jollaCamera.viewfinderResolution_4_3)
            } else if (settings.jollaCamera.viewfinderResolution_16_9) {
                return settings.strToSize(
                            settings.jollaCamera.viewfinderResolution_16_9)
            }
        }

        if (settings.jollaCamera.viewfinderResolution) {
            return settings.strToSize(settings.jollaCamera.viewfinderResolution)
        }

        var supportedResolutions = camera.supportedViewfinderResolutions()
        if (supportedResolutions.length > 0) {
            var bestMatch = 0
            for (var i = 0; i < supportedResolutions.length; i++) {
                var w = supportedResolutions[i].width;
                var h = supportedResolutions[i].height;
                if (w > Screen.height || h > Screen.width) {
                    continue
                }
                if (currentRatio > 0) {
                    var ratio = w / h
                    var bestMatchRatio = supportedResolutions[bestMatch].width / supportedResolutions[bestMatch].height
                    if (Math.abs(ratio - currentRatio) < Math.abs(bestMatchRatio - currentRatio)) {
                        bestMatch = i; // better match to aspect ratio
                    } else if (Math.abs(ratio - currentRatio) == Math.abs(bestMatchRatio - currentRatio) &&
                               w > supportedResolutions[bestMatch].width && h > supportedResolutions[bestMatch].height) {
                        bestMatch = i; // same aspect ratio, better resolution
                    }
                } else {
                    if (w > supportedResolutions[bestMatch].width && h > supportedResolutions[bestMatch].height) {
                        bestMatch = i; // just select best resolution
                    }
                }
            }
            console.log("Choosing view finder resolution: " + supportedResolutions[bestMatch].width + "x" + supportedResolutions[bestMatch].height)
            return Qt.size(supportedResolutions[bestMatch].width, supportedResolutions[bestMatch].height)
        }

        return Qt.size(Screen.height, Screen.width)
    }

    function doShutter() {
        animFlash.start();

        var filename = fsOperations.writableLocation(
                    "image",
                    settings.get("global", "storagePath", "")) + "/IMG_" + Qt.formatDateTime(
                    new Date(), "yyyyMMdd_hhmmss") + "." + fileExtension();

        cameraProxy.stillCapture(filename);
    }

    function fileExtension() {
        var f = settings.getCameraModeValue("format", modelFormats.defaultFormat())
        if (f == "MJPEG") {
            f = "JPG";
        }
        return f.toLowerCase();
    }

    function zoomIn() {
        if (camera.digitalZoom < camera.maximumDigitalZoom) {
            camera.digitalZoom += zoomStepButton;
        }
    }

    function zoomOut() {
        if (camera.digitalZoom > 1) {
            camera.digitalZoom -= zoomStepButton;
        }
    }

    function focusColor() {
        if (camera.lockStatus === Camera.Unlocked) {
            return "white"
        } else if (camera.lockStatus === Camera.Searching) {
            return "#e3e3e3" //light grey
        } else {
            return "lightgreen"
        }
    }

    function shutterIcon() {
        //TODO
        return  styler.customIconPrefix + "../pics/icon-camera-shutter.png";
        if (camera.captureMode === Camera.CaptureStillImage) {
            return "../pics/icon-camera-shutter.png"
        } else {
            if (camera.videoRecorder.recorderStatus === CameraRecorder.RecordingStatus) {
                return "../pics/icon-camera-video-shutter-off.png"
            } else {
                return "../pics/icon-camera-video-shutter-on.png"
            }
        }
    }

    function msToTime(millis) {
        return new Date(millis).toISOString().substr(11, 8)
    }

    function switchCamera(camId) {
        console.log("Switching camera to", camId)
        cameraProxy.stop()

        if (camId !== "") settings.cameraId = camId;
        else if (parseInt(settings.cameraId) + 1 == settings.cameraCount) settings.cameraId = "0";
        else settings.cameraId = parseInt(settings.global.cameraId) + 1;

        console.log("switched to camera", settings.cameraId);
        settings.setGlobalValue("cameraId", settings.cameraId);
        tmrDelayedStart.start()
    }

    function checkIfCamExists(camId) {
        console.log("Check if cam exists: ", camId, settings.enabledCameras.length)
        var found = false;
        for(var i = 0; i < settings.enabledCameras.length; i++) {
            if(settings.enabledCameras[i] === camId)
                found = true;
        }
        return found
    }

    function switchToNextCamera() {
        console.log("Switching no next camera from", settings.cameraId, settings.enabledCameras)
        if (settings.enabledCameras.length == 0) {
            switchCamera(0)
        }else if (settings.enabledCameras.length == 1) {
            switchCamera(settings.enabledCameras[0])
        } else {
            var idx = settings.enabledCameras.indexOf(settings.cameraId);
            if (idx >= 0) {
                idx++;
                if (idx >= settings.enabledCameras.length) {
                    idx = 0
                }
                switchCamera(settings.enabledCameras[idx])
            } else {
                switchCamera(settings.enabledCameras[0])
            }
        }
    }

    function updateRotation(orientation) {
        if (!settings.useSizeAsOrientation) {
            return;
        }

        console.log("Orientation:", orientation, _orientation, controlsContainer.rotation, _rotationValues["ui"][page._orientation], _rotationValues["ui"][orientation], controlsRotation);

        if ((orientation >= OrientationReading.TopUp
             && orientation <= OrientationReading.RightUp)) {
            _orientation = orientation
        }

        if (_nativePortrait) {
            controlsContainer.rotation = _rotationValues["ui"][_orientation]
            page.iconRotation = _rotationValues["icon"][_orientation]
        } else {
            controlsContainer.rotation = _rotationValues["uil"][_orientation]
        }

        viewFinder.rotation = viewfinderOrientation;

        console.log("...", controlsContainer.rotation, page.iconRotation);
    }
}
