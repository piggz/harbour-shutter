# Developing on `shutter`

## Debugging tips and tricks

### Getting `libcamera` logs

[libcamera]() relies on 2 environment variables to decide what to log, and where: `LIBCAMERA_LOG_LEVELS` and `LIBCAMERA_LOG_FILE` (see [here](https://libcamera.org/api-html/log_8h.html)).
Passing values for these in the app's execution environment will let you control the looging. The easiest way to do it is to modify the desktop file.
For SailfishOS, that is [harbour-shutter.desktop](harbour-shutter.desktop), for Ubuntu Touch [click/harbour-shutter-ui.desktop](click/harbour-shutter-ui.desktop)

Change the `Exec=harbour-shutter` line to `Exec=env LIBCAMERA_LOG_LEVELS='*:DEBUG' env LIBCAMERA_LOG_FILE="/tmp/libcamera.log" harbour-shutter`.
