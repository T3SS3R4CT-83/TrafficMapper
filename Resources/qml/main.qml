import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import "./components"
import "./dialogs"

Window {
    id: mainWindow
    title: "TrafficMapper v1.0"
    width: 1000
    height: 525
    visible: true

    MainMenu {
        id: mainMenu  // TODO: Remove later
    }

    VideoPlayer {
        id: videoPlayer
        x: 10
        y: 35
    }

    PlaybackOptions {
        id: playbackOptions
        x: videoPlayer.width + 20
        y: 35
        width: mainWindow.width - videoPlayer.width - 30
    }

    GateList {
        id: gateList
        x: videoPlayer.width + 20
        y: 45 + playbackOptions.height
        width: mainWindow.width - videoPlayer.width - 30
        height: videoPlayer.height - playbackOptions.height - 10
    }

    ProgressWindow {
        id: progressWindow
    }

    GateProperties {
        id: gatePropertiesDialog
    }

    DlgOpenVideo {
        id: dlgOpenVideo
    }

    DlgSaveCache {
        id: dlgSaveCache
    }

    DlgOpenCache {
        id: dlgOpenCache
    }
}
