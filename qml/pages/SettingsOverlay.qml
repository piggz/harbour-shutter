import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtMultimedia 5.4
import uk.co.piggz.pinhole 1.0

import "../components/"
import "../components/platform"

Item {
    property int iconRotation: 0
    property bool panelOpen: panelFlash.expanded
                             || panelWhiteBalance.expanded
                             || panelFocus.expanded || panelIso.expanded
                             || panelResolution.expanded
                             || panelStorage.expanded || panelGeneral.expanded

    property alias modelFormat: modelFormats

    Item {
        id: buttonPanel
        opacity: (panelOpen ? 0 : 1)
        enabled: !panelOpen

        height: parent.height
        width: 2 * styler.itemSizeSmall + 4 * styler.paddingMedium
        anchors.left: parent.left
        anchors.top: parent.top

        //Behavior on opacity {
        //    FadeAnimation {
        //    }
        //}

        GridLayout {
            id: colButtons
            flow: GridLayout.TopToBottom
            rowSpacing: styler.themePaddingSmall
            columnSpacing: styler.themePaddingSmall
            rows: Math.floor(
                      height / (btnGeneral.height + rowSpacing)) //using the button height and not styler size incase we change the RoundButton size

            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                margins: styler.themePaddingSmall
            }

            RoundButton {
                id: btnFormat
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-pixelformat.png"
                visible: modelFormats.rowCount > 0

                onClicked: {
                    panelFormats.show()
                }
            }

            RoundButton {
                id: btnResolution
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-resolution.png"
                visible: modelResolution.rowCount > 0

                onClicked: {
                    panelResolution.show()
                }
            }

            RoundButton {
                id: btnFocus
                iconSource: focusIcon()
                iconRotation: iconRotation
                visible: modelFocus.rowCount > 0

                onClicked: {
                    panelFocus.show()
                }
            }

            RoundButton {
                id: btnWhiteBalance
                iconSource: whiteBalanceIcon()
                iconRotation: iconRotation
                visible: modelWhiteBalance.rowCount > 0

                onClicked: {
                    panelWhiteBalance.show()
                }
            }
            RoundButton {
                id: btnFlash
                iconSource: flashIcon()
                iconRotation: iconRotation
                visible: modelFlash.rowCount > 0

                onClicked: {
                    panelFlash.show()
                }
            }

            RoundButton {
                id: btnIso
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: isoIcon()
                visible: modelIso.rowCount > 0

                onClicked: {
                    panelIso.show()
                }
            }

            RoundButton {
                id: btnStorage
                objectName: "btnStorage"
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-sd-card.svg"
                visible: modelStorage.rowCount > 0

                onClicked: {
                    modelStorage.scan("/media/sdcard")
                    panelStorage.show()
                }
            }

            RoundButton {
                id: btnControls
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-controls.svg"

                onClicked: {
                    panelControls.show()
                }
            }

            RoundButton {
                id: btnGeneral
                iconColor: styler.themePrimaryColor
                iconRotation: iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-developer-mode.svg"

                onClicked: {
                    panelGeneral.show()
                }
            }
        }
    }

    DockedListView {
        id: panelFormats
        model: modelFormats
        selectedItem: (forceUpdate || !forceUpdate) ? settings.getCameraModeValue("format", modelFormats.defaultFormat()) : modelFormats.defaultFormat()
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            settings.setCameraModeValue("format", value);
            cameraProxy.setStillFormat(value);
            hide()
        }
    }

    DockedListView {
        id: panelResolution
        model: sortedModelResolution
        selectedItem: (forceUpdate || !forceUpdate) ?  settings.getCameraModeValue("resolution", modelResolution.defaultResolution(settings.captureMode)) : modelResolution.defaultResolution(settings.captureModel)
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            settings.setCameraModeValue("resolution", value);
            cameraProxy.setResolution(value);
            hide();
        }
    }

    DockedListView {
        id: panelFlash
        model: modelFlash
        selectedItem: settings.getCameraModeValue("flash", 0)
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            camera.flash.setFlashMode(value)
            settings.mode.flash = value
            hide()
        }
    }

    DockedListView {
        id: panelWhiteBalance
        model: modelWhiteBalance
        selectedItem: settings.getCameraModeValue("whiteBalance", 0)
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            camera.imageProcessing.setWhiteBalanceMode(value)
            settings.mode.whiteBalance = value
            hide()
        }
    }

    DockedListView {
        id: panelFocus
        model: modelFocus
        selectedItem: settings.getCameraModeValue("focus", 0)
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            setFocusMode(value)
            hide()
        }
    }

    DockedListView {
        id: panelIso
        model: modelIso
        selectedItem: settings.getCameraModeValue("iso", 0)
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            if (value === 0) {
                camera.exposure.setAutoIsoSensitivity()
            } else {
                camera.exposure.setManualIsoSensitivity(value)
            }
            settings.mode.iso = value
            hide()
        }
    }

    DockedListView {
        id: panelStorage
        model: modelStorage
        selectedItem: settings.get("global", "storagePath", "")
        rotation: iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            settings.global.storagePath = value
            hide()
        }

        Component.onCompleted: {
            restoreStorage()
        }
    }

    DockedPanelPL {
        id: panelControls
        modal: true
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2
        height: parent.height
        z: 99
        dock: dockModes.left
        clip: true

        onVisibleChanged: {
            if (loadingComplete) {
                if (visible) {
                    console.log("loading...")
                    sldBrightness.value = settings.getCameraModeValue("brightness", 0);
                    sldContrast.value = settings.getCameraModeValue("contrast", 0);
                    sldSaturation.value = settings.getCameraModeValue("saturation", 0);
                    sldAnalogueGain.value = settings.getCameraModeValue("analogueGain", 0);

                } else {
                    console.log("saving...")
                    settings.setCameraModeValue("brightness", sldBrightness.value);
                    settings.setCameraModeValue("contrast", sldContrast.value);
                    settings.setCameraModeValue("saturation", sldSaturation.value);
                    settings.setCameraModeValue("analogueGain", sldAnalogueGain.value);

                }
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.7

            Flickable {
                anchors.fill: parent
                anchors.margins: styler.themePaddingMedium
                contentHeight: colControls.height

                Column {
                    id: colControls
                    width: parent.width
                    height: childrenRect.height
                    spacing: styler.themePaddingMedium

                    ControlSlider {
                        id: sldBrightness
                        title: qsTr("Brightness")
                        control: CameraProxy.Brightness;
                    }

                    ControlSlider {
                        id: sldContrast
                        title: qsTr("Contrast")
                        control: CameraProxy.Contrast;
                    }

                    ControlSlider {
                        id: sldSaturation
                        title: qsTr("Saturation")
                        control: CameraProxy.Saturation;
                    }

                    ControlSlider {
                        id: sldAnalogueGain
                        title: qsTr("Gain")
                        control: CameraProxy.AnalogueGain;
                    }

                    /*
                    ControlSlider {
                        id: sldExposure
                        title: qsTr("Exposure")
                        control: CameraProxy.ExposureTime;
                    }*/

                }
            }
        }
    }

    DockedPanelPL {
        id: panelGeneral
        modal: true
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2
        height: parent.height
        z: 99
        dock: dockModes.left
        clip: true
        //TODO rotation: iconRotation

        onVisibleChanged: {
            if (loadingComplete) {
                if (visible) {
                    console.log("loading...")
                    sldAudioBitrate.value = settings.get("global", "audioBitrate", 128000);
                    sldVideoBitrate.value = settings.get("global", "videoBitrate", 1280000);
                } else {
                    console.log("saving...")
                    settings.setGlobalValue("audioBitrate", sldAudioBitrate.value);
                    settings.setGlobalValue("videoBitrate", sldVideoBitrate.value);
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.7

            Flickable {
                anchors.fill: parent
                anchors.margins: styler.themePaddingMedium
                contentHeight: mainColumn.height

                Column {
                    id: mainColumn
                    width: parent.width
                    height: childrenRect.height
                    spacing: styler.themePaddingMedium
                    TextSwitchPL {
                        id: zoomSwitch
                        text: qsTr("Swap zoom controls")
                        checked: settings.get("global", "swapZoomControl", false)
                        onCheckedChanged: {
                            settings.set("global", "swapZoomControl", checked);
                        }
                    }

                    ComboBoxPL {
                        id: gridSwitch
                        label: qsTr("Grid:")
                        model: grids
                        property var grids: [
                            qsTr("None"),
                            qsTr("Thirds"),
                            qsTr("Ambience")
                        ]
                        property var values: ["none", "thirds", "ambience"]

                        function findIndex(id) {
                            for (var i = 0; i < values.length; i++) {
                                if (values[i] === id) {
                                    return i
                                }
                            }
                            return 0
                        }

                        currentIndex: findIndex(settings.gridMode)
                        onValueChanged: {
                            var index = gridSwitch.currentIndex;
                            settings.setGlobalValue("gridMode", gridSwitch.values[index]);
                        }
                    }

                    SliderPL {
                        id: sldVideoBitrate
                        label: qsTr("Video Bitrate")
                        width: parent.width
                        minimumValue: 6400000
                        maximumValue: 32000000
                        stepSize: 800000
                        Text {
                            text: sldVideoBitrate.value
                            anchors.centerIn: parent
                        }

                    }
                    SliderPL {
                        id: sldAudioBitrate
                        label: qsTr("Audio Bitrate")
                        width: parent.width
                        minimumValue: 64000
                        maximumValue: 320000
                        stepSize: 8-000
                        Text {
                            text: sldAudioBitrate.value
                            anchors.centerIn: parent
                        }

                    }
                    TextSwitchPL{
                        id: locationMetadataSwitch
                        width: parent.width

                        checked: settings.get("global", "locationMetadata", false)
                        text: qsTr("Store GPS location to metadata")

                        onCheckedChanged: {
                            settings.setGlobalValue("locationMetadata", checked);
                        }
                    }

                    TextSwitchPL{
                        id: showManualControls
                        width: parent.width

                        checked: settings.get("global", "showManualControls", false)
                        text: qsTr("Display manual controls")

                        onCheckedChanged: {
                            settings.setGlobalValue("showManualControls", checked);
                        }
                    }

                    LabelPL {
                        text: qsTr("Disabled Cameras")
                    }

                    Row {
                        width: parent.width
                        spacing: 5
                        Repeater {
                            model: modelCamera
                            Rectangle {
                                width: styler.themeItemSizeSmall
                                height: width
                                color: (settings.disabledCameras.indexOf("[" + index + "]") >=0) ? "red" : "green"
                                LabelPL  {
                                    anchors.centerIn: parent
                                    text: index
                                }
                                MouseArea {
                                    anchors.fill: parent

                                    onClicked: {
                                        console.log("Clicked ", index)
                                        if (settings.disabledCameras.indexOf("[" + index + "]") >=0) {
                                            settings.disabledCameras = settings.disabledCameras.replace("[" + index + "]", "")
                                        } else {
                                            settings.disabledCameras += ("[" + index + "]")
                                        }

                                        console.log(settings.disabledCameras)
                                        settings.calculateEnabledCameras()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FormatModel {
        id: modelFormats
    }

    EffectsModel {
        id: modelEffects
    }

    ExposureModel {
        id: modelExposure
    }

    IsoModel {
        id: modelIso
    }

    WhiteBalanceModel {
        id: modelWhiteBalance
    }

    FocusModel {
        id: modelFocus
    }

    FlashModel {
        id: modelFlash
    }

    function setCameraProxy(cam) {
        modelFormats.setCameraProxy(cameraProxy);
        modelResolution.setCameraProxy(cameraProxy);
/*
        modelExposure.setCamera(cam)
        modelEffects.setCamera(cam)
        modelIso.setCamera(cam)
        modelWhiteBalance.setCamera(cam)
        modelFocus.setCamera(cam)
        modelFlash.setCamera(cam)
        modelResolution.setImageCapture(cam.imageCapture)
        modelResolution.setVideoRecorder(cam.videoRecorder)
        modelResolution.setMode(settings.global.captureMode)
 */
    }

    function flashIcon() {
        var flashIcon = ""
        switch (settings.getCameraModeValue("flash", 0)) {
        case Camera.FlashAuto:
            flashIcon = "../pics/icon-camera-flash-automatic"
            break
        case Camera.FlashOn:
            flashIcon = "../pics/icon-camera-flash-on"
            break
        case Camera.FlashOff:
            flashIcon = "../pics/icon-camera-flash-off"
            break
        case Camera.FlashRedEyeReduction:
            flashIcon = "../pics/icon-camera-flash-redeye"
            break
        default:
            flashIcon = "../pics/icon-camera-flash-on"
            break
        }
        return flashIcon
    }

    function focusIcon() {
        var focusIcon = ""
        switch (settings.getCameraModeValue("focus", 0)) {
        case Camera.FocusAuto:
            focusIcon = "../pics/icon-camera-focus-auto"
            break
        case Camera.FocusManual:
            focusIcon = "../pics/icon-camera-focus-manual.png"
            break
        case Camera.FocusMacro:
            focusIcon = "../pics/icon-camera-focus-macro"
            break
        case Camera.FocusHyperfocal:
            focusIcon = "../pics/icon-camera-focus-hyperfocal.png"
            break
        case Camera.FocusContinuous:
            focusIcon = "../pics/icon-camera-focus-continuous.png"
            break
        case Camera.FocusInfinity:
            focusIcon = "../pics/icon-camera-focus-infinity"
            break
        default:
            focusIcon = "../pics/icon-camera-focus"
            break
        }
        return styler.customIconPrefix + focusIcon
    }

    function whiteBalanceIcon() {
        var wbIcon = ""
        switch (settings.getCameraModeValue("whiteBalance", 0)) {
        case CameraImageProcessing.WhiteBalanceAuto:
            wbIcon = "../pics/icon-camera-wb-automatic"
            break
        case CameraImageProcessing.WhiteBalanceSunlight:
            wbIcon = "../pics/icon-camera-wb-sunny"
            break
        case CameraImageProcessing.WhiteBalanceCloudy:
            wbIcon = "../pics/icon-camera-wb-cloudy"
            break
        case CameraImageProcessing.WhiteBalanceShade:
            wbIcon = "../pics/icon-camera-wb-shade"
            break
        case CameraImageProcessing.WhiteBalanceTungsten:
            wbIcon = "../pics/icon-camera-wb-tungsten"
            break
        case CameraImageProcessing.WhiteBalanceFluorescent:
            wbIcon = "../pics/icon-camera-wb-fluorecent"
            break
        case CameraImageProcessing.WhiteBalanceSunset:
            wbIcon = "../pics/icon-camera-wb-sunset"
            break
        case CameraImageProcessing.WhiteBalanceFlash:
            wbIcon = "../pics/icon-camera-wb-default" //TODO need icon
            break
        default:
            wbIcon = "../pics/icon-camera-wb-default"
            break
        }
        return styler.customIconPrefix + wbIcon
    }

    function isoIcon() {
        var iso = ""
        if (settings.getCameraModeValue("iso", 0) === 0) {
            iso = "../pics/icon-m-iso-auto.png"
        } else if (settings.getCameraModeValue("iso", 0) === 1) {
            iso = "../pics/icon-m-iso-hjr.png"
        } else {
            iso = "../pics/icon-m-iso-" + settings.getCameraModeValue("iso") + ".png"
        }
        return styler.customIconPrefix + iso
    }

    function effectIcon() {
        var effectIcon = ""

        switch (settings.getCameraModeValue("effect", CameraImageProcessing.ColorFilterNone)) {
        case CameraImageProcessing.ColorFilterNone:
            effectIcon = "none"
            break
        case CameraImageProcessing.ColorFilterAqua:
            effectIcon = "aqua"
            break
        case CameraImageProcessing.ColorFilterBlackboard:
            effectIcon = "blackboard"
            break
        case CameraImageProcessing.ColorFilterGrayscale:
            effectIcon = "grayscale"
            break
        case CameraImageProcessing.ColorFilterNegative:
            effectIcon = "negative"
            break
        case CameraImageProcessing.ColorFilterPosterize:
            effectIcon = "posterize"
            break
        case CameraImageProcessing.ColorFilterSepia:
            effectIcon = "sepia"
            break
        case CameraImageProcessing.ColorFilterSolarize:
            effectIcon = "solarize"
            break
        case CameraImageProcessing.ColorFilterWhiteboard:
            effectIcon = "whiteboard"
            break
        case CameraImageProcessing.ColorFilterEmboss:
            effectIcon = "emboss"
            break
        case CameraImageProcessing.ColorFilterSketch:
            effectIcon = "sketch"
            break
        case CameraImageProcessing.ColorFilterNeon:
            effectIcon = "neon"
            break
        default:
            effectIcon = "default"
            break
        }
        return styler.customIconPrefix + "../pics/icon-m-effect-" + effectIcon + ".svg"
    }

    function sceneModeIcon(scene) {
        return styler.customIconPrefix + "../pics/icon-m-scene_mode_" + modelExposure.iconName(
                    settings.getCameraModeValue("exposure", 0)) + ".svg"
    }

    function setMode(mode) {
        modelResolution.setMode(mode)
        settings.set("global", "captureMode", mode);
    }

    function hideAllPanels() {
        panelFlash.hide()
        panelFocus.hide()
        panelGeneral.hide()
        panelIso.hide()
        panelResolution.hide()
        panelControls.hide()
        panelStorage.hide()
    }

    function restoreStorage() {
        // Restore selection to saved setting, fallback to internal otherwise
        for (var i = 0; i < modelStorage.rowCount; i++) {
            var name = modelStorage.getName(i)
            var path = modelStorage.getPath(i)
            if (path === settings.get("global", "storagePath", "")) {
                settings.set("global", "storagePath", path);
                console.log("Selecting", name, "->", path);
                return i;
            }
        }
        console.log("Defaulting to internal storage")
        settings.set("global", "storagePath", modelStorage.getPath(0));
    }

    Connections {
        target: modelStorage

        onModelReset: {
            restoreStorage()
        }
    }
}
