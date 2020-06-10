import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgOpenCache
    title: "Open cache file"
    nameFilters: ["TrafficMapper Cache files (*.tmc)"]
    selectedNameFilter: "TrafficMapper Cache files (*.tmc)"
    folder: "file:///D:/Videos/"

    signal cacheFileOpened(url fileUrl)

    onAccepted: {
        tracker.openCacheFile(dlgOpenCache.fileUrl)

        // var path = fileUrl.toString().replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/, "")
        // path = decodeURIComponent(path);

        // dlgAnalyzerSettings.cacheFileUrl = path

//        cacheFileOpened(dlgOpenCache.fileUrl)
    }
}
