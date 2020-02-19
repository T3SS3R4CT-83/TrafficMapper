import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
//import QtQuick.Controls.Styles 1.4

Dialog {

    id: progressWindow
    width: 600
    height: 170
    title: ""
    standardButtons: StandardButton.Cancel
    visible: false

    Connections {
        target: tracker
        onProgressUpdated: {
            progressMessage.text = internal.message + " [" + (_currentFrameIdx + 1) + " / " + _allFrameNr + "]"
            progressBar.value = _currentFrameIdx / _allFrameNr * 100
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

    function initAndOpen(_title, _message) {
        title = _title
        internal.message = _message
        progressBar.value = 0
        open()
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



    onRejected: {
        progressMessage.text = "Cancelling process..."
        tracker.terminate()
    }
}
