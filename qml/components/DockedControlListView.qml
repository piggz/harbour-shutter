import QtQuick 2.0
import uk.co.piggz.shutter 1.0
import "./platform"

DockedPanelPL {
    id: panel

    property alias model: listView.model
    property var selectedItem
    modal: true

    dock: dockModes.left

    width: parent.width / 2
    height: parent.height
    z: 99

    clip: true

    function init() {

    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.7

        ListViewPL {
            id: listView
            anchors.fill: parent
            clip: true

            delegate: Item {
                id: item
                height: childrenRect.height
                width: listView.width
                property alias controlEnabled: checkEnable.checked
                property bool _ready: false

                onControlEnabledChanged: {
                    console.log("Control ", name, " enabled ", controlEnabled);
                    settings.setCameraModeValue(name + "_enabled", controlEnabled);
                    if (controlEnabled) {
                        setValue();
                    } else {
                        //setDefaultValue();
                        cameraProxy.removeControlValue(code);
                    }
                }
                function setValue() {
                    if (cameraProxy && _ready && controlEnabled /*&& name !== "ExposureTime"*/) { //TODO ExposureTime seems to cause issues
                        if (type === CameraProxy.ControlTypeFloat || type === CameraProxy.ControlTypeInteger32 || type === CameraProxy.ControlTypeInteger64) {
                            console.log("setting control value", code, type, slider.value)
                            settings.setCameraModeValue(name, slider.value);
                            cameraProxy.setControlValue(code, type, slider.value);
                        } else if (type === CameraProxy.ControlTypeBool) {
                            console.log("setting control value", code, type, switchButton.checked)
                            settings.setCameraModeValue(name, switchButton.checked);
                            cameraProxy.setControlValue(code, type, switchButton.checked);
                        }
                    }
                }

                function setDefaultValue() {
                    if (type === CameraProxy.ControlTypeFloat || type === CameraProxy.ControlTypeInteger32 || type === CameraProxy.ControlTypeInteger64) {
                        slider.value = def;
                    } else if (type === CameraProxy.ControlTypeBool) {
                        switchButton.checked = def;
                    }
                }

                Row {
                    width: listView.width
                    height: styler.themeItemSizeLarge

                    TextSwitchPL {
                        id: checkEnable
                        text: ""
                        width: styler.themeItemSizeLarge
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item {
                        width: listView.width - checkEnable.width - btnReset.width
                        height: styler.themeItemSizeLarge

                        ControlSlider {
                            id: slider
                            title: name
                            visible: type === CameraProxy.ControlTypeFloat || type === CameraProxy.ControlTypeInteger32 || type === CameraProxy.ControlTypeInteger64
                            control: code
                            minimumValue: min
                            maximumValue: max
                            stepSize: type === CameraProxy.ControlTypeFloat ? 0.1 : 1
                            width: parent.width
                            height: parent.height
                            enabled: item.controlEnabled
                            onValueChanged: {
                                setValue();
                            }
                        }

                        TextSwitchPL {
                            id: switchButton
                            text: name
                            visible: type === CameraProxy.ControlTypeBool
                            anchors.verticalCenter: parent.verticalCenter
                            enabled: item.controlEnabled

                            onCheckedChanged: {
                                setValue();
                            }
                        }
                    }

                    IconButtonPL {
                        id: btnReset
                        iconSource: styler.customIconPrefix + "../pics/icon-camera-switch.svg"

                        onClicked: {
                            setDefaultValue();
                        }
                    }
                }
                Component.onCompleted: {
                    console.log("Created delegate for ", name, code, type, min, max, def);

                    if (type === CameraProxy.ControlTypeFloat || type === CameraProxy.ControlTypeInteger32 || type === CameraProxy.ControlTypeInteger64) {
                        console.log("Loading saved float value for", name, settings.getCameraModeValue(name, def));
                        slider.value = settings.getCameraModeValue(name, def);
                    } else if (type === CameraProxy.ControlTypeBool) {
                        console.log("Loading saved bool value", settings.getCameraModeValue(name, def));
                        switchButton.checked = settings.getCameraModeValue(name, def) == "true";
                    }
                    controlEnabled = settings.getCameraModeValue(name + "_enabled", false) == "true";
                    _ready = true;
                }
            }
        }
    }
}
