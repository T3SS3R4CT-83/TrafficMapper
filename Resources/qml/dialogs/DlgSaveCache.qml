import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgSaveCache
    title: "Save cache file"
    nameFilters: ["TrafficMapper Cache files (*.tmc)"]
    selectedNameFilter: "TrafficMapper Cache files (*.tmc)"
    folder: "file:///D:/Videos/"
    selectExisting: false

    onAccepted: {
        progressWindow.initAndOpen("Saving cache file...", "Processing frames:")
        tracker.extractDetectionData(dlgSaveCache.fileUrl)
    }
}
