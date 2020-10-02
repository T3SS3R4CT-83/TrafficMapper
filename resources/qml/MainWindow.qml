//import QtQuick 2.13
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.4

import QtQuick.Layouts 1.15 // TODO: Remove
import QtQuick 2.15

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



    VideoPlayer {
        id: videoPlayer
        x: 10
        y: 10
        width: parent.width - 218
        height: parent.height - 20
    }

    PlaybackOptions {
        id: playbackOptions
        x: videoPlayer.width + 20
        y: 10
        width: mainWindow.width - videoPlayer.width - 30
    }

    GateList {
        id: gateList
        x: videoPlayer.width + 20
        y: 20 + playbackOptions.height
        width: parent.width - videoPlayer.width - 30
        height: videoPlayer.height - playbackOptions.height - 10
    }



    DlgStatWindow {
        id: statWindow
    }

    DlgProgressWindow {
        id: dlgProgressWindow
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

    DlgAnalyzerSettings {
        id: dlgAnalyzerSettings
    }

    DlgCameraCalibration {
        id: dlgCameraCalibration
    }

    DlgExportVideo {
        id: dlgExportVideo
    }

    Window{
        id: videoWindow
        flags: Qt.FramelessWindowHint
        color: "black"

        Item {
            id: videoWindowWrapper
            anchors.fill: parent

            Keys.onEscapePressed: {
                videoWindow.close()
                videoPlayer.state = "embedded"
            }
        }
    }
}
