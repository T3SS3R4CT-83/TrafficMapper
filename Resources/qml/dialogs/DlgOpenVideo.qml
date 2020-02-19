import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {

    id: openVideoDialog
    title: "Please choose a file"
    selectedNameFilter: "Video files (*.mp4)"
    nameFilters: ["Video files (*.mp4)"]
    folder: "file:///D:/Videos/"

    signal videoFileOpened(url fileUrl)

    onAccepted: videoFileOpened(openVideoDialog.fileUrl)
}
