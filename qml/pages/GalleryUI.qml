import QtQuick 2.5
import uk.co.piggz.pinhole 1.0

import "../components/"
import "../components/platform"

PagePL {
    id: galleryPage

    property var fileList: ({

                            })
    property alias showButtons: btnClose.visible

    function isVideo(idx) {
        return fileList.get(idx).isVideo
    }

    function removeFile(idx) {
        var path = fileList.get(idx).filePath
        console.log("Removing", path)
        if (fsOperations.deleteFile(path)) {
            fileList.remove(idx)
            if (gallery.count === 0) {
                console.log("Closing empty gallery!")
                pageStack.pop()
            }
        } else {
            console.log("Error deleting file:", path)
        }
    }

    function getFileName(idx) {
        var fullPath = fileList.get(idx).filePath
        var lastSep = fullPath.lastIndexOf("/")
        var fileName = fullPath.substr(lastSep + 1, fullPath.length - lastSep)
        return fileName
    }

    RoundButton {
        id: btnClose
        visible: true
        iconSource: styler.customIconPrefix + "../pics/icon-m-close.png"
        size: styler.themeItemSizeSmall

        anchors {
            top: parent.top
            topMargin: styler.themePaddingMedium
            right: parent.right
            rightMargin: styler.themePaddingMedium
        }

        onClicked: {
            console.log("Clicked close button")
            pageStack.pop()
        }
    }

    RoundButton {
        id: btnAbout
        visible: showButtons
        iconSource: styler.customIconPrefix + "../pics/icon-m-about.png"
        size: styler.themeItemSizeSmall

        anchors {
            top: parent.top
            topMargin: styler.themePaddingMedium
            left: parent.left
            leftMargin: styler.themePaddingMedium
        }

        onClicked: {
            var filePath = fileList.get(gallery.currentIndex).filePath
            var mediaPage = isVideo(gallery.currentIndex) ? "AboutVideo.qml" : "AboutImage.qml"
            pageStack.push(Qt.resolvedUrl(mediaPage), {
                               "filePath": filePath,
                               "fileName": getFileName(gallery.currentIndex),
                           })
        }
    }

    /*
    RemorsePopup {
        id: remorse
    }
*/
    Row {
        id: rowBottom

        visible: showButtons
        spacing: styler.themePaddingMedium
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: styler.themePaddingMedium
        }

        RoundButton {
            id: btnRemove

            iconSource: styler.customIconPrefix + "../pics/icon-m-delete.png"
            size: styler.themeItemSizeSmall

            function showRemorseItem() {
                var deleteIndex = gallery.currentIndex
                remorse.execute(qsTr("Deleting %1").arg(getFileName(
                                                            deleteIndex)),
                                function () {
                                    removeFile(deleteIndex)
                                })
            }

            onClicked: {
                console.log("Clicked delete button")
                showRemorseItem()
            }
        }
        RoundButton {
            id: btnShare
            visible: gallery.canShare
            iconSource: styler.customIconPrefix +  "../pics/icon-m-share.png"
            size: styler.themeItemSizeSmall

            onClicked: {
                var filePath = fileList.get(gallery.currentIndex).filePath
                var mimeType = "image/jpeg"
                gallery.share.resources = [filePath]
                gallery.share.mimeType = mimeType
                gallery.share.trigger()
            }
        }

    }

    SlideshowPL {
        id: gallery
        model: fileList
    }
}
