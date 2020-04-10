import QtQuick 2.12
import QtQuick.Controls 2.12

import com.elte.t3ss3r4ct 1.0

MenuBar {
    id: mainMenu
    height: 25
    anchors.left: parent.left
    anchors.right: parent.right

    Menu {
        title: "File"

        MenuItem {
            text: "Open video..."
            onTriggered: dlgOpenVideo.open()
        }

        MenuItem {
            text: "Open cache..."
            enabled: videoPlayer.isVideoLoaded
            onTriggered: dlgOpenCache.open()
        }

        MenuSeparator {}

        MenuItem {
            text: "Export video..."
            enabled: videoPlayer.isVideoLoaded
            onTriggered: {
                progressWindow.initAndOpen("Exporting frames", "Processing frames:")
                tracker.exportFrames()
            }
        }
    }

    Menu {
        title: "Traffic analysis"

//        MenuItem {
//            text: "Extract detection data"
//            enabled: videoPlayer.isVideoLoaded
//            onTriggered: dlgSaveCache.open()
//        }

//        MenuSeparator {}

        MenuItem {
            text: "Analize traffic video"
            enabled: videoPlayer.isVideoLoaded
            onTriggered: {
                progressWindow.initAndOpen("Analysing traffic video", "Processing frames:")
                tracker.analizeVideo()
            }
        }
    }

    Menu {
        title: "Statistics"

        MenuItem {
            text: "Open stat window"
            onTriggered: statWindow.open()
        }
    }

//    Menu {
//        title: "Other"
//
//        MenuItem {
//            text: "Open stat window"
//            onTriggered: {
//                var component = Qt.createComponent("StatWindow.qml")
//                mainWindow.statWindow = component.createObject(mainWindow)
//                mainWindow.statWindow.closing.connect(mainWindow.statWindow.destroy)
//            }
//        }
//    }
}
