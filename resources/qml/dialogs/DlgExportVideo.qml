import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgSaveChart
    title: "Export video"
    nameFilters: ["MP4 video file (*.mp4)"]
    selectedNameFilter: "MP4 video file (*.mp4)"
    folder: "file:///D:/"
    selectExisting: false

    onAccepted: {
        var path = dlgSaveChart.fileUrl.toString()
        // path = path.replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/,"")
        // path = decodeURIComponent(path);

        mediaPlayer.exportVideo(path)
        dlgProgressWindow.openForVideoExport()
    }
}
