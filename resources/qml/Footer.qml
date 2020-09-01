import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import TrafficMapper 1.0

ToolBar {
    height: 34
    background: Rectangle {
        gradient: Gradient {
            GradientStop { position: 1; color: "#FFFFFF" }
            GradientStop { position: 0.5; color: "#FFFFFF" }
            GradientStop { position: 0; color: "#DDDDDD" }
        }
        Rectangle {
            width: parent.width
            height: 1
            anchors.top: parent.top
            color: "#BBBBBB"
        }
    }

    property int cacheLoaded: 0
    property int cacheAllFrames: 0

    RowLayout {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: 15

        ToolButton {
            text: "Open cache file..."
            onClicked: dlgOpenCache.open()
        }

        Label {
            text: "Cached frames: <b>" + tracker.cacheSize + "</b> / <b>" + mediaPlayer.videoMeta.framecount + "</b>"
        }

        ToolButton {
            text: "LOAD ALL SHIT"
            onClicked: {
                dlgOpenVideo.videoFileOpened(Qt.resolvedUrl("D:/Szakdolgozat/Videos/video_03.mp4"))
                tracker.openCacheFile("file:///D:/Szakdolgozat/Videos/video_03.nms.bin")
            }
        }
    }
}
