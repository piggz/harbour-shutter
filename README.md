# harbour-shutter

This is a camera application for [Sailfish](https://sailfishos.org/) which exposes all available camera parameters to the user.
The app uses [libcamera](https://libcamera.org/) to interface with the device's cameras.

The application is licensed under the GPLv2+, with some source files specifically licensed under the LGPLv2(.1)+ so they can be re-used.

## GUI

Shutter can be built for various GUI options, called "flavors":
  - KDE's [Kirigami](https://develop.kde.org/frameworks/kirigami/) framework
  - SailfishOS's [Silica](https://sailfishos.org/develop/docs/silica/) QML module
  - Qt's [Qt Quick Controls](https://doc.qt.io/qt-6/qtquickcontrols-index.html)
  - Unity's [UI toolkit](https://docs.unity3d.com/Manual/UIElements.html)

These flavors are reflected in the `qml/components/platform.*` directories, which hold the files implementing the GUI.

At of March 2024, the Qt Quick Control flavor does not seem to be operational, but Silica and Kirigami should be.
