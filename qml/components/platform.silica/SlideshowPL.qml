import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import Nemo.Thumbnailer 1.0

SlideshowView {
    id: gallery
    readonly property bool canShare: true
    property alias share: shareAction
    clip: true
    width: parent.width
    height: parent.height
    z: -1

    currentIndex: count - 1

    delegate: Rectangle {
        id: delegate
        width: parent.width
        height: parent.height
        color: 'black'

        Thumbnail {
            id: thumbnail

            sourceSize.width: parent.width
            sourceSize.height: parent.height
            anchors.fill: parent
            fillMode: Thumbnail.PreserveAspectFit
            source: filePath
            mimeType: isVideo ? "video/" : "image/"
            smooth: true

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    showButtons = !showButtons
                }
            }
        }
    }

    ShareAction {
        id: shareAction
    }
}
