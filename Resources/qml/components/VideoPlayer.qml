import QtQuick 2.12
import QtMultimedia 5.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtQml 2.12
import QtQuick.Dialogs 1.2

import com.elte.t3ss3r4ct 1.0

import "../custom-items"

Item {
    id: videoPlayer
    width: 782
    height: 480



    signal videoFileOpened(url fileUrl)

    property bool isVideoLoaded: false
    property Gate currentGateItem: null



    Connections {
        target: dlgOpenVideo
        function onVideoFileOpened(fileUrl) { mediaPlayer.source = fileUrl }
    }

    Connections {
        target: tracker
        function onProgressUpdated(currentFrameIdx, allFrameNr) {
            sliderBg.width = (slider.width - 2) * currentFrameIdx / allFrameNr
        }
    }

    Keys.onLeftPressed: {
        mediaPlayer.pause()
        var frameTime = 1000 / GlobalMeta.VIDEO_FPS
        if (mediaPlayer.position > 0)
            mediaPlayer.seek(mediaPlayer.position - frameTime)
        console.log(mediaPlayer.position / frameTime)
    }
    Keys.onRightPressed: {
        mediaPlayer.pause()
        var frameTime = 1000 / GlobalMeta.VIDEO_FPS
        if (mediaPlayer.position < mediaPlayer.duration - frameTime)
            mediaPlayer.seek(mediaPlayer.position + frameTime)
        console.log(mediaPlayer.position / frameTime)
    }
    Keys.onSpacePressed: switchPlayState()



    Rectangle {
        id: videoWrapper
        objectName: "videoWrapper"
        x: 0
        y: 0
        width: parent.width
        height: 440
       // color: "#222222"
        
        Image {
            anchors.fill: parent
            source: "qrc:/img/placeholder.png"
        }

        VideoOutput {
            id: videoOutput
            source: mediaPlayer
            anchors.fill: parent
            filters: [videoFilter]
        }

        Item {
            id: gateContainer
            anchors.fill: parent

            MouseArea{
                id: gateMouseArea
                anchors.fill: parent

                onPressed: {
                    if (gateList.isGatePlacingEnabled) {
                        currentGateItem = Qt.createQmlObject('import com.elte.t3ss3r4ct 1.0; Gate {}', gateContainer)

                        currentGateItem.anchors.fill = gateContainer
                        currentGateItem.startPos = Qt.point(mouseX, mouseY)
                        currentGateItem.endPos = Qt.point(mouseX, mouseY)
                    }
                }
                onPositionChanged: {
                    if (gateList.isGatePlacingEnabled)
                        currentGateItem.endPos = Qt.point(mouseX,mouseY)
                }
                onReleased: {
                    if (gateList.isGatePlacingEnabled) {
                        currentGateItem.endPos = Qt.point(mouseX,mouseY)
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
        background: CustomButtonBackground {}
    }

    Button {
        id: btnPlay
        width: 30
        height: 30
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.bottom: parent.bottom
        icon.source: "qrc:/svg/media-play.svg"
        background: CustomButtonBackground {}

        onClicked: switchPlayState()
    }

    ProgressBar {
        id: slider
        width: parent.width - 170
        height: 10
        anchors.left: parent.left
        anchors.leftMargin: 80
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        value: mediaPlayer.position / mediaPlayer.duration
        background: Rectangle {
            anchors.fill: parent
            color: "#EEEEEE"
            border.color: "#AAAAAA"
            Rectangle {
                id: sliderBg
                x: 1
                y: 1
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
            z:2
        }

        MouseArea {
            anchors.fill: parent

            property bool isSeeking: false

            onPressed: {
                isSeeking = true
                mediaPlayer.seek(mediaPlayer.duration * mouseX / width)
            }
            onPositionChanged: {
                if (isSeeking && mediaPlayer.seekable)
                    mediaPlayer.seek(mediaPlayer.duration * mouseX / width)
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
        text: getPlayTime()
        font.pixelSize: 12
    }



    MediaPlayer {
        id: mediaPlayer
        objectName: "mediaPlayer"
        muted: btnMute.checked
        notifyInterval: 10

        onPlaying: {
            btnPlay.icon.source = "qrc:/svg/media-pause.svg"
        }
        onPaused: {
            btnPlay.icon.source = "qrc:/svg/media-play.svg"
        }
        onStopped: {
            btnPlay.icon.source = "qrc:/svg/media-play.svg"
        }
        onStatusChanged: {
            if (status === MediaPlayer.Loaded) {
                setVideo()
            } else if (status === MediaPlayer.InvalidMedia) {
                dlgVideoLoadingError.visible = true
                unsetVideo()
            }
        }
    }

    MessageDialog {
        id: dlgVideoLoadingError
        title: "Video file could not be loaded!"
        icon: StandardIcon.Critical
        text: "The video file may be invalid or the required codecs are not installed!"
        standardButtons: StandardButton.Ok
    }



    function setVideo() {
        mediaPlayer.play()
        mediaPlayer.pause()

        var path = mediaPlayer.source.toString().replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/, "")
        path = decodeURIComponent(path);

        GlobalMeta.VIDEO_URL = path
        GlobalMeta.VIDEO_WIDTH = mediaPlayer.metaData.resolution.width
        GlobalMeta.VIDEO_HEIGHT = mediaPlayer.metaData.resolution.height
        GlobalMeta.VIDEO_FPS = mediaPlayer.metaData.videoFrameRate
        GlobalMeta.VIDEO_LENGTH = mediaPlayer.metaData.duration
        GlobalMeta.VIDEO_FRAMECOUNT = Math.floor(GlobalMeta.VIDEO_LENGTH * 0.001 * GlobalMeta.VIDEO_FPS)

        videoPlayer.isVideoLoaded = true

        mainWindow.title = "TrafficMapper v1.0 - " + path

        printVideoMetaData()
    }

    function unsetVideo() {
        GlobalMeta.VIDEO_URL = ""
        GlobalMeta.VIDEO_WIDTH = 0
        GlobalMeta.VIDEO_HEIGHT = 0
        GlobalMeta.VIDEO_FPS = 0
        GlobalMeta.VIDEO_LENGTH = 0
        GlobalMeta.VIDEO_FRAMECOUNT = 0

        videoPlayer.isVideoLoaded = false

        mainWindow.title = "TrafficMapper v1.0"
    }

    function switchPlayState() {
        if (mediaPlayer.playbackState == MediaPlayer.PlayingState)
            mediaPlayer.pause()
        else
            mediaPlayer.play()
    }

    function getPlayTime() {
        var position = new Date(mediaPlayer.position)
        var duration = new Date(mediaPlayer.duration)

        return position.getMinutes() + ":" + ("0" + position.getSeconds()).slice(-2) + " / " + duration.getMinutes() + ":" + ("0" + duration.getSeconds()).slice(-2)
    }

    function printVideoMetaData() {
        console.log()
        console.log("============================================================")
        console.log("                     VIDEO META DATA                        ")
        console.log("------------------------------------------------------------")
        console.log("Video resolution: " + GlobalMeta.VIDEO_WIDTH + " x " + GlobalMeta.VIDEO_HEIGHT)
        console.log("Video length:     " + GlobalMeta.VIDEO_LENGTH + " ms (" + GlobalMeta.VIDEO_FRAMECOUNT + " frame | " + GlobalMeta.VIDEO_FPS.toFixed(2) + " frame/s)")
        console.log("============================================================")
        console.log()
    }
}
