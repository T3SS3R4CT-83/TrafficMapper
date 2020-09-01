import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2

Dialog {
    id: dlgCameraCalibration
    width: 250
    height: 100
    title: "Start video analysis"
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    visible: false

    Switch {
        id: swUseGPU
        x: 10
        y: 10
        text: qsTr("Use GPU")
        checked: false
        checkable: true
    }

    onAccepted: {
        dlgProgressWindow.openForTracker()
        tracker.analizeVideo(swUseGPU.checked)
    }
}
