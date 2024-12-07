import QtQuick 2.0
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtMultimedia
import "pages"
import "components"
import uk.co.piggz.shutter 1.0

Kirigami.ApplicationWindow {
    id: app
    property bool loadingComplete: false;
    property bool forceUpdate: false;

    pageStack.initialPage: CameraUI{
        id: cameraUI
    }

    Component.onCompleted: {
        cameraUI.startup();
        loadingComplete = true;
    }

    Settings {
        id: settings
        property int cameraId: 0
        property string captureMode
        property int cameraCount
        property variant enabledCameras: [] //Calculated on startup and when disabledCameras changes
        property string disabledCameras: ""
        property string gridMode: "none"
        
        function getCameraValue(s, d) {
            return get(cameraId, s, d);
        }
        function setCameraValue(s, v) {
            if (!loadingComplete) {
                return;
            }
            set(cameraId, s, v);
            forceUpdate = !forceUpdate;
        }

        function setGlobalValue(s, v) {
            if (!loadingComplete) {
                return;
            }
            settings[s] = v;
            set("global", s, v);
            forceUpdate = !forceUpdate;
        }

        function getGlobalValue(s, d) {
            settings[s] = get("global", s, d);
            return settings[s];
        }

        function getCameraModeValue(s, d) {
            return get(cameraId + "_" + captureMode, s, d);
        }

        function setCameraModeValue(s, v) {
            set(cameraId + "_" + captureMode, s, v);
            forceUpdate = !forceUpdate;
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

            setGlobalValue("disabledCameras", disabledCameras);
            app.forceUpdate = !app.forceUpdate;
        }

        function loadGlobalSettings() {
            captureMode = getGlobalValue("captureMode", "image");
            cameraId = getGlobalValue("cameraId", 0);
            disabledCameras = getGlobalValue("disabledCameras", "");
            gridMode = getGlobalValue("gridMode", "none");
        }

        function saveGlobalSettings() {
            setGlobalValue("captureMode", captureMode);
            setGlobalValue("cameraId", cameraId);
            setGlobalValue("disabledCameras", disabledCameras);
            setGlobalValue("gridMode", gridMode);
        }

        Component.onCompleted: {
            console.log("Setting up default settings");
            loadGlobalSettings();
            saveGlobalSettings();

            cameraCount = modelCamera.rowCount;
        }
    }

    TruncationModes { id: truncModes }
    DockModes { id: dockModes }

    onActiveChanged: {
        if (!app.active) {
            cameraProxy.stop();
        } else {
            if (pageStack.depth === 1){
                cameraUI.startViewfinder();
            }
        }
    }

    QtObject {
        id: styler
        // font sizes and family
        property string themeFontFamily: Qt.application.font.family
        property string themeFontFamilyHeading: Qt.application.font.family
        property int  themeFontSizeHuge: Math.round(themeFontSizeMedium*3.0)
        property int  themeFontSizeExtraLarge: Math.round(themeFontSizeMedium*2.0)
        property int  themeFontSizeLarge: Math.round(themeFontSizeMedium*1.5)
        property int  themeFontSizeMedium: Math.round(Qt.application.font.pixelSize*2.0)
        property int  themeFontSizeSmall: Math.round(themeFontSizeMedium*0.9)
        property int  themeFontSizeExtraSmall: Math.round(themeFontSizeMedium*0.7)
        property real themeFontSizeOnMap: themeFontSizeSmall

        // colors
        // block background (navigation, poi panel, bubble)
        property color blockBg: palette.window
        // variant of navigation icons
        property string navigationIconsVariant: darkTheme ? "white" : "black"
        // descriptive items
        property color themeHighlightColor: palette.windowText
        // navigation items (to be clicked)
        property color themePrimaryColor: palette.text
        // navigation items, secondary
        property color themeSecondaryColor: inactivePalette.text
        // descriptive items, secondary
        property color themeSecondaryHighlightColor: inactivePalette.text

        // button sizes
        property real themeButtonWidthLarge: 256
        property real themeButtonWidthMedium: 180

        // icon sizes
        property real themeIconSizeLarge: 2.5*themeFontSizeLarge
        property real themeIconSizeMedium: 2*themeFontSizeLarge
        property real themeIconSizeSmall: 1.5*themeFontSizeLarge
        // used icons
        property string iconAbout: "help-about-symbolic"
        property string iconBack: "go-previous-symbolic"
        property string iconClear: "edit-clear-all-symbolic"
        property string iconClose: "window-close-symbolic"
        property string iconDelete: "edit-delete-symbolic"
        property string iconDot: "find-location-symbolic"
        property string iconDown: "go-down-symbolic"
        property string iconEdit: "document-edit-symbolic"
        property string iconEditClear: "edit-clear-symbolic"
        property string iconFavorite: "bookmark-new-symbolic"
        property string iconFavoriteSelected: "user-bookmarks-symbolic"
        property string iconForward: "go-next-symbolic"
        property string iconManeuvers: "maneuvers-symbolic"
        property string iconMaps: "map-layers-symbolic"
        property string iconMenu: "open-menu-symbolic"
        property string iconNavigate: "route-symbolic"
        property string iconNavigateTo: "route-to-symbolic"
        property string iconNavigateFrom: "route-from-symbolic"
        property string iconNearby: "nearby-search-symbolic"
        property string iconPause: "media-playback-pause-symbolic"
        property string iconPhone: "call-start-symbolic"
        property string iconPreferences: "preferences-system-symbolic"
        property string iconProfileMixed: "profile-mixed-symbolic"
        property string iconProfileOffline: "profile-offline-symbolic"
        property string iconProfileOnline: "profile-online-symbolic"
        property string iconRefresh: "view-refresh-symbolic"
        property string iconSave: "document-save-symbolic"
        property string iconSearch: "edit-find-symbolic"
        property string iconShare: "emblem-shared-symbolic"
        property string iconShortlisted: "shortlist-add-symbolic"
        property string iconShortlistedSelected: "shortlist-selected-symbolic"
        property string iconStart: "media-playback-start-symbolic"
        property string iconStop: "media-playback-stop-symbolic"
        property string iconWebLink: "web-browser-symbolic"

        property string customIconPrefix: ""
        property string customIconSuffix: ""

        // item sizes
        property real themeItemSizeLarge: themeFontSizeLarge * 3
        property real themeItemSizeSmall: themeFontSizeMedium * 3
        property real themeItemSizeExtraSmall: themeFontSizeSmall * 3

        // paddings and page margins
        property real themeHorizontalPageMargin: 1.25*themeFontSizeExtraLarge
        property real themePaddingLarge: 0.75*themeFontSizeExtraLarge
        property real themePaddingMedium: 0.5*themeFontSizeLarge
        property real themePaddingSmall: 0.25*themeFontSizeSmall

        property real themePixelRatio: Screen.devicePixelRatio

        property bool darkTheme: (blockBg.r + blockBg.g + blockBg.b) <
                                 (themePrimaryColor.r + themePrimaryColor.g +
                                  themePrimaryColor.b)

        property list<QtObject> children: [
            SystemPalette {
                id: palette
                colorGroup: SystemPalette.Active
            },

            SystemPalette {
                id: disabledPalette
                colorGroup: SystemPalette.Disabled
            },

            SystemPalette {
                id: inactivePalette
                colorGroup: SystemPalette.Inactive
            }
        ]
    }
}
