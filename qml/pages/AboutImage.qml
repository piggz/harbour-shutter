import QtQuick 2.0
import QtMultimedia 5.6
import uk.co.piggz.shutter 1.0

import "../components/"
import "../components/platform"

PagePL {
    id: pageImage
    property var filePath
    property var fileName

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    Image {
        id: image
        visible: false
        source: filePath
        asynchronous: true
        onStatusChanged: {
            if (status === Image.Ready) {
                mediaAbout.mediaSize = app.sizeToStr(sourceSize)
            }
        }
    }

    ExifModel {
        id: modelExif
        source: filePath
    }

    AboutMedia {
        id: mediaAbout
        mediaModel: modelExif
        mediaSize: ""
        file: fileName
        fileSize: fsOperations.getFileSizeHuman(filePath)
    }
}
