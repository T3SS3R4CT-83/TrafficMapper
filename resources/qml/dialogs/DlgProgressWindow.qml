import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2

Dialog {
    id: progressWindow
    width: 600
    height: 170
    title: ""
    standardButtons: Dialog.NoButton
    visible: false

    Connections {
        target: tracker
        function onProgressUpdated(currentFrameIdx, allFrameNr) {
            progressMessage.text = internal.message + " [" + (currentFrameIdx + 1) + " / " + allFrameNr + "]"
            progressBar.value = currentFrameIdx / allFrameNr * 100
        }
        function onAnalysisEnded() { close() }
    }

    Connections {
        target: mediaPlayer
        function onProgressUpdated(currentFrameIdx, allFrameNr) {
            progressMessage.text = internal.message + " [" + (currentFrameIdx + 1) + " / " + allFrameNr + "]"
            progressBar.value = currentFrameIdx / allFrameNr * 100
        }
    }

    QtObject {
        id: internal
        property string message: ""
        property int value: 0
        property var handler
    }



    Text {
        id: progressMessage
        x: 20
        y: 20
        width: parent.width - 40
        text: internal.message
    }

    ProgressBar {
        id: progressBar
        x: 20
        y: 60
        width: parent.width - 40
        height: 20
        from: 0
        to: 100
        value: 0
    }

    Button {
        id: btnCancel
        text: "Cancel"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 20
        anchors.rightMargin: 20
        onClicked: {
            progressMessage.text = "Cancelling process..."
            internal.handler.stop()
        }
    }



    function openForTracker() {
        title = "Analysing traffic video"
        internal.message = "Processing frames: "
        internal.handler = tracker
        progressBar.value = 0
        open()
    }

    function openForVideoExport() {
        title = "Exporting video"
        internal.message = "Processing frames: "
        internal.handler = mediaPlayer
        progressBar.value = 0
        open()
    }
}
