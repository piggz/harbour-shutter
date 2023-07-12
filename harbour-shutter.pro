# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-shutter

CONFIG += link_pkgconfig
CONFIG += sailfishapp
CONFIG += no_keywords
CONFIG += c++1z

QT += quick multimedia
PKGCONFIG += libcamera

INCLUDEPATH += /usr/local/include/libcamera

SOURCES += \
    src/cameramodel.cpp \
    src/cameraproxy.cpp \
    src/controlmodel.cpp \
    src/effectsmodel.cpp \
    src/exifmodel.cpp \
    src/exposuremodel.cpp \
    src/format_converter.cpp \
    src/formatmodel.cpp \
    src/harbour-shutter.cpp \
    src/image.cpp \
    src/isomodel.cpp \
    src/encoder_jpeg.cpp \
    src/metadatamodel.cpp \
    src/resolutionmodel.cpp \
    src/settings.cpp \
    src/viewfinder2d.cpp \
    src/viewfinderitem.cpp \
    src/viewfinderrenderer.cpp \
    src/wbmodel.cpp \
    src/focusmodel.cpp \
    src/flashmodel.cpp \
    src/fsoperations.cpp \
    src/resourcehandler.cpp \
    src/storagemodel.cpp

DISTFILES += \
    README.md \
    qml/components/ControlSlider.qml \
    qml/components/DockModes.qml \
    qml/components/DockedControlListView.qml \
    qml/components/DockedListView.qml \
    qml/components/platform.kirigami/SlideshowPL.qml \
    qml/components/platform.qtcontrols/DockedPanelPL.qml \
    qml/components/platform.silica/DockedPanelPL.qml \
    qml/components/platform.silica/SlideshowPL.qml \
    qml/harbour-shutter.qml \
    qml/pages/SettingsX.qml \
    qml/pics/icon-camera-switch.svg \
    qml/pics/icon-m-camera.svg \
    qml/pics/icon-m-developer-mode.svg \
    qml/pics/icon-m-image.svg \
    qml/pics/icon-m-pixelformat.png \
    qml/pics/icon-m-sd-card.svg \
    qml/pics/icon-m-tele-lense-active.png \
    qml/pics/icon-m-tele-lense.svg \
    qml/pics/icon-m-uwide-lense-active.png \
    qml/pics/icon-m-uwide-lense.svg \
    qml/pics/icon-m-video.svg \
    qml/pics/icon-m-wide-lense-active.png \
    qml/pics/icon-m-wide-lense.svg \
    qml/components/AboutMedia.qml \
    qml/pages/AboutImage.qml \
    qml/pages/AboutVideo.qml \
    rpm/harbour-shutter.spec \
    translations/*.ts \
    harbour-shutter.desktop \
    qml/components/IconSwitch.qml \
    qml/components/RoundButton.qml \
    qml/cover/CoverPage.qml \
    qml/pages/CameraUI.qml \
    qml/pages/GalleryUI.qml \
    qml/pages/SettingsOverlay.qml \
    qml/components/platform.silica/ApplicationWindowPL.qml \
    qml/components/platform.silica/BusyIndicatorPL.qml \
    qml/components/platform.silica/BusyIndicatorSmallPL.qml \
    qml/components/platform.silica/ButtonPL.qml \
    qml/components/platform.silica/ClipboardPL.qml \
    qml/components/platform.silica/ComboBoxPL.qml \
    qml/components/platform.silica/ContextMenuItemPL.qml \
    qml/components/platform.silica/ContextMenuPL.qml \
    qml/components/platform.silica/Cover.qml \
    qml/components/platform.silica/DatePickerDialogPL.qml \
    qml/components/platform.silica/DialogAutoPL.qml \
    qml/components/platform.silica/DialogListPL.qml \
    qml/components/platform.silica/DialogPL.qml \
    qml/components/platform.silica/DockedPanelPL.qml \
    qml/components/platform.silica/ExpandingSectionGroupPL.qml \
    qml/components/platform.silica/ExpandingSectionPL.qml \
    qml/components/platform.silica/FileSelectorPL.qml \
    qml/components/platform.silica/FormLayoutPL.qml \
    qml/components/platform.silica/IconButtonPL.qml \
    qml/components/platform.silica/IconPL.qml \
    qml/components/platform.silica/LabelPL.qml \
    qml/components/platform.silica/ListItemPL.qml \
    qml/components/platform.silica/MenuDrawerItemPL.qml \
    qml/components/platform.silica/MenuDrawerPL.qml \
    qml/components/platform.silica/MenuDrawerSubmenuItemPL.qml \
    qml/components/platform.silica/MenuDrawerSubmenuPL.qml \
    qml/components/platform.silica/PageEmptyPL.qml \
    qml/components/platform.silica/PageListPL.qml \
    qml/components/platform.silica/PageMenuItemPL.qml \
    qml/components/platform.silica/PageMenuPL.qml \
    qml/components/platform.silica/PagePL.qml \
    qml/components/platform.silica/RemorsePopupPL.qml \
    qml/components/platform.silica/SearchFieldPL.qml \
    qml/components/platform.silica/SectionHeaderPL.qml \
    qml/components/platform.silica/SliderPL.qml \
    qml/components/platform.silica/StackPL.qml \
    qml/components/platform.silica/StylerPL.qml \
    qml/components/platform.silica/TextAreaPL.qml \
    qml/components/platform.silica/TextFieldPL.qml \
    qml/components/platform.silica/TextSwitchPL.qml \
    qml/components/platform.silica/TimePickerDialogPL.qml \
    qml/components/platform.silica/ToolItemPL.qml \
    qml/components/platform.silica/ValueButtonPL.qml


SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS +=

HEADERS += \
    src/cameramodel.h \
    src/cameraproxy.h \
    src/controlmodel.h \
    src/effectsmodel.h \
    src/exifmodel.h \
    src/exposuremodel.h \
    src/format_converter.h \
    src/formatmodel.h \
    src/image.h \
    src/isomodel.h \
    src/encoder_jpeg.h \
    src/metadatamodel.h \
    src/resolutionmodel.h \
    src/settings.h \
    src/viewfinder.h \
    src/viewfinder2d.h \
    src/viewfinderitem.h \
    src/viewfinderrenderer.h \
    src/wbmodel.h \
    src/focusmodel.h \
    src/flashmodel.h \
    src/fsoperations.h \
    src/resourcehandler.h \
    src/storagemodel.h

LIBS += -ldl

RESOURCES += \
    shutter.qrc

equals(FLAVOR, "silica") {
    CONFIG += flavor_silica
    SOURCES += src/deviceinfo.cpp
} else:equals(FLAVOR, "kirigami") {
    CONFIG += flavor_kirigami
} else:equals(FLAVOR, "qtcontrols") {
    CONFIG += flavor_qtcontrols
} else:equals(FLAVOR, "uuitk") {
    CONFIG += flavor_uuitk
} else {
    error("Please specify platform using FLAVOR=platform as qmake option. Supported platforms: kirigami, silica, qtcontrols, uuitk.")
}

equals(DISABLE_SYSTEMD, "yes") {
    DEFINES += DISABLE_SYSTEMD
}

flavor_silica {
    message(SailfishOS build)
    CONFIG += sailfishapp sailfishapp_no_deploy_qml sailfishapp_i18n
    DEFINES += MER_EDITION_SAILFISH
}

# PREFIX
isEmpty(PREFIX) {
    flavor_silica {
        PREFIX = /usr
    } else:flavor_uuitk {
        PREFIX = /
    } else {
        PREFIX = /usr/local
    }
}

# PREFIX_RUNNING
isEmpty(PREFIX_RUNNING) {
    flavor_uuitk {
        PREFIX_RUNNING = .
    } else {
        PREFIX_RUNNING = $$PREFIX
    }
}

DATADIR = $$PREFIX/share/$${TARGET}

target.path = $$PREFIX/bin

qml.files = qml/*.qml \
            qml/pages \
            qml/cover \
            qml/components
qml.path = $$DATADIR/qml

js.files = qml/tools/*.js
js.path = $$DATADIR/qml/tools

icons.files = qml/pics/*.png qml/pics/*.svg
icons.path = $$DATADIR/qml/pics

qmlplatform.extra = mkdir -p ${INSTALL_ROOT}$$DATADIR/qml/components/platform && cp -L -v $$PWD/qml/components/platform.$$FLAVOR/*.qml ${INSTALL_ROOT}$$DATADIR/qml/components/platform
qmlplatform.path = $$DATADIR/qml/platform

desktopfile.files = harbour-shutter.desktop
desktopfile.path = $$PREFIX/share/applications

appicon.files = ../harbour-shutter.svg
appicon.path = $$PREFIX/share/icons/hicolor/scalable/apps

INSTALLS += qmlplatform qml js icons target desktopfile appicon

#End install config
