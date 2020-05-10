import QtQuick 2.0
import QtQuick.Dialogs 1.2

FileDialog {
    id: dlgSaveChart
    title: "Save chart"
    selectedNameFilter: "PNG image format (*.png)"
    nameFilters: ["PNG image format (*.png)"]
    folder: "file:///D:/"
    selectExisting: false
    onAccepted: {
        var path = dlgSaveChart.fileUrl.toString()
        path = path.replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/,"")
        path = decodeURIComponent(path);

        chartView.grabToImage(function(result) {
            result.saveToFile(path)
        });
    }
}
