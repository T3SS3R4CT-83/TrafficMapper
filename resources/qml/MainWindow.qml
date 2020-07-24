//import QtQuick 2.13
//import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.4

import QtQuick.Layouts 1.15 // TODO: Remove
import QtQuick 2.12 // TODO: Remove

import "./components"
import "./dialogs"

ApplicationWindow {
    id: mainWindow
    title: Qt.application.name + " v" + Qt.application.version
    width: 1000
    height: 525 + footer.height
    visible: true
    menuBar: MainMenu {}
    background: Rectangle { color: "#FFFFFF" }
    footer: Footer { id: toolbar }

//    Rectangle {
//        id: r1
//
//        width: parent.width - 150
//        height: parent.height
//        anchors.left: parent.left
//
//        color: "#cd5555"
//
//        Rectangle {
//            anchors.horizontalCenter: parent.horizontalCenter
//            anchors.verticalCenter: parent.verticalCenter
//            width: height / 9 * 16
//            height: Math.min(parent.width / 16 * 9, parent.height)
//            color: "black"
//        }
//    }
//
//    Rectangle {
//        id: r2
//
//        width: 150
//        height: parent.height
//        anchors.right: parent.right
//
//        color: "#336699"
//    }




    VideoPlayer {
        id: videoPlayer
        x: 10
        y: 10
        width: parent.width - 218
        height: parent.height - 20
    }

//    PlaybackOptions {
//        id: playbackOptions
//        x: videoPlayer.width + 20
//        y: 35
//        width: mainWindow.width - videoPlayer.width - 30
//    }

    GateList {
        id: gateList
        x: videoPlayer.width + 20
//        y: 45 + playbackOptions.height
        y: 10
        width: parent.width - videoPlayer.width - 30
//        height: videoPlayer.height - playbackOptions.height - 10
        height: parent.height - 20
    }

    DlgStatWindow {
        id: statWindow
    }

    DlgProgressWindow {
        id: progressWindow
    }

    DlgGateProperties {
        id: gatePropertiesDialog
    }

    DlgOpenVideo {
        id: dlgOpenVideo
    }

    DlgOpenCache {
        id: dlgOpenCache
    }

//    DlgSaveCache {
//        id: dlgSaveCache
//    }

    DlgAnalyzerSettings {
        id: dlgAnalyzerSettings
    }

    DlgCameraCalibration {
        id: dlgCameraCalibration
    }
}
