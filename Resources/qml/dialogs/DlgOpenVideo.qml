import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgOpenVideo
    title: "Open traffic video file"
    nameFilters: ["Video files (*.mp4)"]
    selectedNameFilter: "Video files (*.mp4)"
    folder: "file:///D:/Videos/"
//    folder: shortcuts.movies

    signal videoFileOpened(url fileUrl)

    onAccepted: videoFileOpened(dlgOpenVideo.fileUrl)
}
