# harbour-shutter

`Shutter` is a camera application for [Sailfish](https://sailfishos.org/) which exposes all available camera parameters to the user.
The app uses [libcamera](https://libcamera.org/) to interface with the device's cameras.

The application is licensed under the GPLv2+, with some source files specifically licensed under the LGPLv2(.1)+ so they can be re-used.

## GUI

`Shutter` can be built for various GUI options, called "flavors":
  - KDE's [Kirigami](https://develop.kde.org/frameworks/kirigami/) framework
  - SailfishOS's [Silica](https://sailfishos.org/develop/docs/silica/) QML module
  - Qt's [Qt Quick Controls](https://doc.qt.io/qt-6/qtquickcontrols-index.html)
  - Unity's [UI toolkit](https://docs.unity3d.com/Manual/UIElements.html)

These flavors are reflected in the `qml/components/platform.*` directories, which hold the files implementing the GUI.

At of March 2024, the Qt Quick Control flavor does not seem to be operational, but Silica and Kirigami should be.

## Building

### Building for SailfishOS:

#### Installing and seting up the Sailfish SDK
Building for Sailfish is done via the [Sailfish SDK](https://docs.sailfishos.org/Tools/Sailfish_SDK/). As mentioned in the [installation instructions](https://docs.sailfishos.org/Tools/Sailfish_SDK/Installation/#openssl-11-linux-only), it is using an old version of openssl.
On recent distributions, this will no longer be distributed, so you might have to build it yourself, or the installation will fail with SSL errors (see for instance [this post](https://forum.sailfishos.org/t/installing-sailfish-sdk-on-ubuntu-22-04/14121)).

#### Setting up your environment

Once the Sailfish SDK is installed, you should have the `sfdk` command, and can setup your build environment.

First, you need to tell the SDK what you want to build for. Use the `aarch64` flavor of the latest Sailfish OS version:
```
$ sfdk config --push target SailfishOS-4.5.0.18-aarch64
```

`shutter` relies on `libcamera` and `opencv`, but the `dev` packages are not included in the default repositories, so we have to point it
to the ones that contain these packages:

```
$ sfdk build-shell --maintain
[SailfishOS-4.5.0.18-aarch64] harbour-shutter # ssu ar native https://repo.sailfishos.org/obs/nemo:/devel:/hw:/native-common/sailfish_latest_aarch64/
[SailfishOS-4.5.0.18-aarch64] harbour-shutter # ssu ar chum https://repo.sailfishos.org/obs/sailfishos:/chum/4.5.0.24_aarch64/
[SailfishOS-4.5.0.18-aarch64] harbour-shutter # ssu ur
[SailfishOS-4.5.0.18-aarch64] harbour-shutter # zypper ref
```

Your build environment should now be able to find all dependencies for `shutter`.

#### Building the app

For a normal build, after you have followed the steps above, you should be able to build by simply running
```
$ sfdk build
```
in `shutter`'s top-level directory.

#### Deploying the app

Your build should have generated an `RPMS` directory, containing the app `.rpm` package.
Once you've uploaded it to your device, you can install it using `rpm --install <shutter>.rpm`

Because the RPM does not include the app's dependencies, there is a good chance that you will also need to install the following packages:

```
[defaultuser@PinePhonePro] ssu ar native https://repo.sailfishos.org/obs/nemo:/devel:/hw:/native-common/sailfish_latest_aarch64/
[defaultuser@PinePhonePro] ssu ar chum https://repo.sailfishos.org/obs/sailfishos:/chum/4.5.0.24_aarch64/
[defaultuser@PinePhonePro] ssu ur
[defaultuser@PinePhonePro] pkcon refresh
[defaultuser@PinePhonePro] pkcon install libcamera-devel
[defaultuser@PinePhonePro] pkcon install opencv
```

You can now finally install your custom-built `shutter`:
```
[defaultuser@PinePhonePro] pkcon install-local harbour-shutter<yourpackage>.rpm
```

It should now appear in your installed apps, and you can launch it like any other app installed e.g. from the store.

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

## Development

For documentation regarding development of `shutter`, please see [here](DEVELOPMENT.md).
