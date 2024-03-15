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

## Building

### Building for Ubuntu Touch:

WARNING: This build has only been tested to work on the Original PinePhone, and does not currently work on the PinePhone Pro.

#### Setting up clickable

This app at the moment requires permissions described in the apparmor profile that are not covered by a profile I know of.
This means, that it is running unconfined, and lists several read paths, leading Clickable to error out on the review step.
To use this app in its current state nonetheless, one can configure Clickable to bypass this step (it is up to you whether
you trust the app enough to do this).

To do so, add the following lines in `~/.clickable/config.yaml`, as documented [here](https://clickable-ut.dev/en/dev/config.html):
```
build:
    skip_review: true
```

#### Building

The latest Ubuntu Touch is running on a Focal (20.04) base, which means libcamera is absent from the repositories, and the
version of libopencv is too old.
This means, that we first need to build these libraries, so they can be included in the app and deployed with it:
`clickable build --libs --arch arm64`
(this will take a while)

Another issue here is the version of Kirigami that is available in the distribution's repositories. Some QML components require
Kirigami 2.14, but the version available in the repos is 2.5.
This has been circumvented by providing QML files using Kirigami 2.5 in qml/platform.kirigami/ut

Once the libraries are built, you can build the app:
`clickable build --app --arch arm64`

After the build is finished, you can now deploy to your device:
`clickable --ssh <your_device_ip>`

