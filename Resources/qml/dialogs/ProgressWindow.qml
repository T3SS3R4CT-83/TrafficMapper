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
        onProgressUpdated: {
            progressMessage.text = internal.message + " [" + (currentFrameIdx + 1) + " / " + allFrameNr + "]"
            progressBar.value = currentFrameIdx / allFrameNr * 100
        }
    }

    Connections {
        target: tracker
        onProcessTerminated: close()
    }

    QtObject {
        id: internal
        property string message: ""
        property int value: 0
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
            tracker.terminate()
        }
    }



    function initAndOpen(_title, _message) {
        title = _title
        internal.message = _message
        progressBar.value = 0
        open()
    }
}
