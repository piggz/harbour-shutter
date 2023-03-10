import QtQuick 2.0
import QtMultimedia 5.6

import "../components/"
import "../components/platform"

PagePL {
    id: videoPage

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    backNavigation: false

    property string videoFile
    property double controlsOpacity: 1.0

    DisplayBlanking {
        preventBlanking: player.playbackState === MediaPlayer.PlayingState
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        MediaPlayer {
            id: player

            source: "file://" + videoFile
            autoPlay: true

            onPositionChanged: {
                seekSlider.value = position
            }

            onStopped: {
                pageStack.pop(null, true)
            }
        }

        VideoOutput {
            id: video

            anchors.fill: parent
            source: player
            fillMode: VideoOutput.PreserveAspectFit

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    controlsOpacity > 0 ? controlsOpacity = 0 : controlsOpacity = 1.0
                }
            }
        }

        Item {
            id: itemsControls
            anchors.fill: parent
            opacity: controlsOpacity

            Behavior on opacity {
                FadeAnimation {
                }
            }

            RoundButton {
                id: btnCloseVideo

                enabled: controlsOpacity > 0
                icon.source: "image://styler/icon-m-close"
                size: styler.itemSizeMedium

                anchors {
                    top: parent.top
                    topMargin: styler.paddingMedium
                    right: parent.right
                    rightMargin: styler.paddingMedium
                }

                onClicked: {
                    pageStack.pop(null, true)
                }
            }

            RoundButton {
                id: btnPlayPause

                enabled: controlsOpacity > 0
                icon.source: player.playbackState === MediaPlayer.PlayingState ? "image://styler/icon-m-pause" : "image://styler/icon-m-play"
                size: styler.itemSizeMedium
                anchors.centerIn: parent

                onClicked: {
                    if (player.playbackState === MediaPlayer.PausedState) {
                        player.play()
                    } else if (player.playbackState === MediaPlayer.PlayingState) {
                        player.pause()
                    } else if (player.playbackState === MediaPlayer.StoppedState) {
                        player.seek(0)
                        player.play()
                    }
                }
            }

            Row {
                id: rowBottomVideo

                width: parent.width
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: styler.paddingMedium
                }

                Slider {
                    id: seekSlider

                    width: parent.width
                    anchors.verticalCenter: parent.verticalCenter
                    minimumValue: 0
                    maximumValue: player.duration > 0 ? player.duration : 1
                    value: player.position
                    stepSize: 0
                    enabled: player.seekable && controlsOpacity > 0

                    onReleased: {
                        player.seek(value)
                    }
                }
            }

            Timer {
                running: true
                interval: 333
                repeat: false
                onTriggered: controlsOpacity = 0
            }
        }
    }
}
