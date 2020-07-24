import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgOpenCache
    title: "Open cache file"
    nameFilters: ["TrafficMapper Cache files (*.bin)"]
    selectedNameFilter: "TrafficMapper Cache files (*.bin)"
    folder: "file:///D:/Videos/"

    onAccepted: tracker.openCacheFile(dlgOpenCache.fileUrl)
}
