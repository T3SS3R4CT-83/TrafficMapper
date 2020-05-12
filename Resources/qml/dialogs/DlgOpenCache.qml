import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgOpenCache
    title: "Open cache file"
    nameFilters: ["TrafficMapper Cache files (*.tmc)"]
    selectedNameFilter: "TrafficMapper Cache files (*.tmc)"
    folder: "file:///D:/Videos/"

    onAccepted: tracker.openCacheFile(dlgOpenCache.fileUrl)
}
