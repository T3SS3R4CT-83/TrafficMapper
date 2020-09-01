import QtQuick 2.12
import QtMultimedia 5.15
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15

import TrafficMapper 1.0

import "../custom-items"

Item {
//    id: videoPlayer
//    width: 16 * height / 9
//    height: Math.min(parent.width / 16 * 9, parent.height)


    property bool isVideoLoaded: false
    property Gate currentGateItem: null

    Connections {
        target: dlgOpenVideo
        function onVideoFileOpened(fileUrl) {
            mediaPlayer.loadVideo(fileUrl)
        }
    }
//    Connections {
//        target: tracker
//        function onProgressUpdated(currentFrameIdx, allFrameNr) {
//            sliderBg.width = (slider.width - 2) * currentFrameIdx / allFrameNr
//        }
//    }

//    Keys.onLeftPressed: {
//        mediaPlayer.pause()
//        var frameTime = 1000 / GlobalMeta.VIDEO_FPS
//        if (mediaPlayer.position > 0)
//            mediaPlayer.setPosition(mediaPlayer.position - frameTime)
//        console.log(mediaPlayer.position / frameTime)
//    }
//    Keys.onRightPressed: {
//        mediaPlayer.pause()
//        var frameTime = 1000 / GlobalMeta.VIDEO_FPS
//        if (mediaPlayer.position < mediaPlayer.duration - frameTime)
//            mediaPlayer.seek(mediaPlayer.position + frameTime)
//        console.log(mediaPlayer.position / frameTime)
//    }
    Keys.onSpacePressed: switchPlayState()



    Item {
//        id: videoWrapper
//        objectName: "videoWrapper"
        x: 0
        y: 0
        width: parent.width
        height: parent.width * 9 / 16  
//       // color: "#222222"

        Image {
            anchors.fill: parent
            source: "qrc:/img/placeholder.png"
        }

        VideoOutput {
            id: videoOutput
            source: mediaPlayer
            anchors.fill: parent
            filters: [videoOverlay]

            Connections {
                target: mediaPlayer
                function onMediaStatusChanged(status) {
                    if (status === MediaPlayer.InvalidMedia) {
                        dlgVideoLoadingError.visible = true
                        isVideoLoaded = false
                    } else if (status === MediaPlayer.Loaded) {
                        isVideoLoaded = true
                    }
                }
                function onStateChanged(state) {
                    if (state === MediaPlayer.PlayingState)
                        btnPlay.icon.source = "qrc:/svg/media-pause.svg"
                    else
                        btnPlay.icon.source = "qrc:/svg/media-play.svg"
                }
            }

        }
    
        Item {
            id: gateContainer
            anchors.fill: parent

            MouseArea{
                id: gateMouseArea
                anchors.fill: parent

                onPressed: {
                    if (gateList.isGatePlacingEnabled) {
                        currentGateItem = Qt.createQmlObject('import TrafficMapper 1.0; Gate {}', gateContainer)

                        currentGateItem.anchors.fill = gateContainer
                        currentGateItem.setStartPos(Qt.point(mouseX, mouseY))
                        currentGateItem.setEndPos(Qt.point(mouseX, mouseY))
                    }
                }
                onPositionChanged: {
                    if (gateList.isGatePlacingEnabled)
                        currentGateItem.setEndPos(Qt.point(mouseX, mouseY))
                }
                onReleased: {
                    if (gateList.isGatePlacingEnabled) {
                        currentGateItem.setEndPos(Qt.point(mouseX, mouseY))
                        gatePropertiesDialog.open()
                        gateList.isGatePlacingEnabled = false
                    }
                }
            }
        }
    }

    Button {
        id: btnMute
        width: 30
        height: 30
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        icon.source: checked ? "qrc:/svg/sound-mute.svg" : "qrc:/svg/sound-on.svg"
        checkable: true
        background: CustomItemBackground {}

        onClicked: mediaPlayer.muted = checked
    }

    Button {
        id: btnPlay
        width: 30
        height: 30
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.bottom: parent.bottom
        icon.source: "qrc:/svg/media-play.svg"
        background: CustomItemBackground {}

        onClicked: switchPlayState()
    }

    ProgressBar {
        id: slider
        width: parent.width - 180
        height: 10
        anchors.left: parent.left
        anchors.leftMargin: 90
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        to: mediaPlayer.duration
        value: mediaPlayer.position
        background: CustomItemBackground {
            anchors.fill: parent
            Rectangle {
                x: 1; y: 1
                height: parent.height - 2
                color: "#B0CEFF"
            }
        }
        contentItem: Rectangle {
            anchors.left: parent.left
            width: parent.visualPosition * parent.width
            height: parent.height
            radius: 2
            color: "#CD5555"
            z: 2
        }

        MouseArea {
            anchors.fill: parent

            property bool isSeeking: false

            onPressed: {
                isSeeking = true
                mediaPlayer.setPosition(mediaPlayer.duration * mouseX / width)
            }
            onPositionChanged: {
                if (isSeeking && mediaPlayer.seekable)
                    mediaPlayer.setPosition(mediaPlayer.duration * mouseX / width)
            }
            onReleased: {
                isSeeking = false
            }
        }
    }

    Text {
        height: 30
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        verticalAlignment: Text.AlignVCenter
        text: mediaPlayer.positionLabel
        font.pixelSize: 12
    }



    MessageDialog {
        id: dlgVideoLoadingError
        title: "Video file could not be loaded!"
        icon: StandardIcon.Critical
        text: "The video file may be invalid or the required codecs are not installed!"
        standardButtons: StandardButton.Ok
    }

    function switchPlayState() {
        if (mediaPlayer.state === MediaPlayer.PlayingState)
            mediaPlayer.pause()
        else
            mediaPlayer.play()
    }
}
