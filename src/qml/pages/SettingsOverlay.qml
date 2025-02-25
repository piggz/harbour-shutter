import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtMultimedia
import uk.co.piggz.shutter 1.0

import "../components/"

Item {
    id: settingsOverlay
    property int iconRotation: 0
    property bool panelOpen: panelFlash.expanded
                             || panelFocus.expanded
                             || panelResolution.expanded
                             || panelStorage.expanded || panelGeneral.expanded


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
                iconRotation: settingsOverlay.iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-pixelformat.png"
                visible: modelFormats ? modelFormats.rowCount > 0 : false

                onClicked: {
                    panelFormats.show()
                }
            }

            RoundButton {
                id: btnResolution
                iconColor: styler.themePrimaryColor
                iconRotation: settingsOverlay.iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-resolution.png"
                visible: modelResolution ? modelResolution.rowCount > 0 : false

                onClicked: {
                    panelResolution.show()
                }
            }

            RoundButton {
                id: btnFocus
                iconSource: focusIcon()
                iconRotation: settingsOverlay.iconRotation
                visible: modelFocus.rowCount > 0

                onClicked: {
                    panelFocus.show()
                }
            }

            RoundButton {
                id: btnFlash
                iconSource: flashIcon()
                iconRotation: settingsOverlay.iconRotation
                visible: modelFlash.rowCount > 0

                onClicked: {
                    panelFlash.show()
                }
            }

            RoundButton {
                id: btnStorage
                objectName: "btnStorage"
                iconColor: styler.themePrimaryColor
                iconRotation: settingsOverlay.iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-sd-card.svg"
                visible: modelStorage ? modelStorage.rowCount > 0 : false

                onClicked: {
                    modelStorage.scan("/media/sdcard")
                    panelStorage.show()
                }
            }

            RoundButton {
                id: btnControls
                iconColor: styler.themePrimaryColor
                iconRotation: settingsOverlay.iconRotation
                iconSource: styler.customIconPrefix + "../pics/icon-m-controls.svg"

                onClicked: {
                    panelControls.show()
                }
            }

            RoundButton {
                id: btnGeneral
                iconColor: styler.themePrimaryColor
                iconRotation: settingsOverlay.iconRotation
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
        selectedItem: (forceUpdate || !forceUpdate) && modelFormats ? settings.getCameraModeValue("format", modelFormats.defaultFormat()) : ""
        rotation: settingsOverlay.iconRotation
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
        selectedItem: (forceUpdate || !forceUpdate) && modelResolution ?  settings.getCameraModeValue("resolution", modelResolution.defaultResolution(settings.captureMode)) : ""
        rotation: settingsOverlay.iconRotation
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
        rotation: settingsOverlay.iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            camera.flash.setFlashMode(value)
            settings.mode.flash = value
            hide()
        }
    }

    DockedListView {
        id: panelFocus
        model: modelFocus
        selectedItem: settings.getCameraModeValue("focus", 0)
        rotation: settingsOverlay.iconRotation
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2

        onClicked: {
            setFocusMode(value)
            hide()
        }
    }

    DockedListView {
        id: panelStorage
        model: modelStorage
        selectedItem: settings.get("global", "storagePath", "")
        rotation: settingsOverlay.iconRotation
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

    DockedControlListView {
        id: panelControls
        model: modelControls
        width: (rotation === 90
                || rotation === 270) ? parent.height : parent.width / 2
        rotation: settingsOverlay.iconRotation
        height: parent.height
        z: 99
        dock: dockModes.left
        clip: true
    }

    DockedPanel {
        id: panelGeneral
        modal: true
        width: (iconRotation === 90
                || iconRotation === 270) ? parent.height : parent.width / 2
        height: parent.height
        z: 99
        dock: dockModes.left
        clip: true
        rotation: settingsOverlay.iconRotation

        onVisibleChanged: {
            if (loadingComplete) {
                if (visible) {
                    console.log("SettingsOverlay - panelGeneral - Loading settings.")
                    sldAudioBitrate.value = settings.get("global", "audioBitrate", 128000);
                    sldVideoBitrate.value = settings.get("global", "videoBitrate", 1280000);
                } else {
                    console.log("SettingsOverlay - panelGeneral - Saving settings.")
                    settings.setGlobalValue("audioBitrate", sldAudioBitrate.value);
                    settings.setGlobalValue("videoBitrate", sldVideoBitrate.value);
                }
            }
        }

        Flickable {
            anchors.fill: parent
            anchors.margins: styler.themePaddingMedium
            contentHeight: mainColumn.height

            Column {
                id: mainColumn
                width: parent.width
                height: childrenRect.height
                spacing: styler.themePaddingMedium
                TextSwitch {
                    id: zoomSwitch
                    text: qsTr("Swap zoom controls")
                    checked: settings.get("global", "swapZoomControl", false)
                    onCheckedChanged: {
                        settings.set("global", "swapZoomControl", checked);
                    }
                }

                ComboBox {
                    id: gridSwitch
                    //label: qsTr("Grid:")
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
                    onCurrentValueChanged: {
                        var index = gridSwitch.currentIndex;
                        settings.setGlobalValue("gridMode", gridSwitch.values[index]);
                    }
                }

                TextSlider {
                    id: sldVideoBitrate
                    label: qsTr("Video Bitrate")
                    from: 6400000
                    to: 32000000
                    stepSize: 800000
                }

                TextSlider {
                    id: sldAudioBitrate
                    label: qsTr("Audio Bitrate")
                    from: 64000
                    to: 320000
                    stepSize: 8-000
                }

                TextSwitch{
                    id: locationMetadataSwitch
                    width: parent.width

                    text: qsTr("Store GPS location to metadata")

                    Component.onCompleted: {
                        checked = settings.getGlobalValue("locationMetadata", false)
                    }

                    onCheckedChanged: {
                        settings.setGlobalValue("locationMetadata", checked);
                    }
                }

                TextSwitch {
                    id: faceDetectionSwitch
                    width: parent.width

                    text: qsTr("Enable/Disable face detection")

                    Component.onCompleted: {
                        checked = settings.getGlobalValue("faceDetection", false)
                    }

                    onCheckedChanged: {
                        console.log("The face detection button has been clicked! - ", checked)
                        settings.setGlobalValue("faceDetection", checked);
                        cameraProxy.setFaceDetectionEnabled(checked);
                    }
                }

                TextSwitch {
                    id: sizeOrientationSwitch
                    width: parent.width

                    text: qsTr("Use screen size as orientation")

                    Component.onCompleted: {
                        checked = settings.getGlobalValue("useSizeAsOrientation", false)
                    }

                    onCheckedChanged: {
                        console.log("Size as orientation! - ", checked)
                        settings.setGlobalValue("useSizeAsOrientation", checked);
                    }
                }

                Label {
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
                            Label  {
                                anchors.centerIn: parent
                                text: index
                            }
                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    console.log("Clicked the button for Camera ", index)
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

    FocusModel {
        id: modelFocus
    }

    FlashModel {
        id: modelFlash
    }

    function flashIcon() {
        var flashIcon = ""
        switch (settings.getCameraModeValue("flash", 0)) {
        case Camera.FlashAuto:
            flashIcon = "../pics/icon-camera-flash-automatic.png"
            break
        case Camera.FlashOn:
            flashIcon = "../pics/icon-camera-flash-on.png"
            break
        case Camera.FlashOff:
            flashIcon = "../pics/icon-camera-flash-off.png"
            break
        case Camera.FlashRedEyeReduction:
            flashIcon = "../pics/icon-camera-flash-redeye.png"
            break
        default:
            flashIcon = "../pics/icon-camera-flash-on.png"
            break
        }
        return styler.customIconPrefix + flashIcon
    }

    function focusIcon() {
        var focusIcon = ""
        switch (settings.getCameraModeValue("focus", 0)) {
        case Camera.FocusAuto:
            focusIcon = "../pics/icon-camera-focus-auto.png"
            break
        case Camera.FocusManual:
            focusIcon = "../pics/icon-camera-focus-manual.png"
            break
        case Camera.FocusMacro:
            focusIcon = "../pics/icon-camera-focus-macro.png"
            break
        case Camera.FocusHyperfocal:
            focusIcon = "../pics/icon-camera-focus-hyperfocal.png"
            break
        case Camera.FocusContinuous:
            focusIcon = "../pics/icon-camera-focus-continuous.png"
            break
        case Camera.FocusInfinity:
            focusIcon = "../pics/icon-camera-focus-infinity.png"
            break
        default:
            focusIcon = "../pics/icon-camera-focus.png"
            break
        }
        return styler.customIconPrefix + focusIcon
    }

    function setMode(mode) {
        modelResolution.setMode(mode)
        settings.set("global", "captureMode", mode);
    }

    function hideAllPanels() {
        panelControls.hide()
        panelFlash.hide()
        panelFocus.hide()
        panelGeneral.hide()
        panelResolution.hide()
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
