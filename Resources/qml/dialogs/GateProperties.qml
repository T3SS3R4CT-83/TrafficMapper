import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2

Dialog {
    id: dlgGateProperties
    width: 300
    height: 90
    title: "Gate properties"
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    visible: false

    Text {
        id: lblInputFieldName
        x: 10
        y: 10
        height: 20
        text: "Gate's name:"
        font.pixelSize: 12
    }

    TextInput {
        id: inputGateName
        x: lblInputFieldName.width + 25
        y: 10
        width: parent.width - lblInputFieldName.width - 26
        height: 20
        text: ""
        font.pixelSize: 12
        focus: true
    }

    Rectangle {
        x: inputGateName.x - 6
        y: inputGateName.y - 6
        z: -1
        width: inputGateName.width + 6
        height: inputGateName.height + 6
        color: "#FFFFFF"
        border.color: "#AAAAAA"
    }

    onVisibilityChanged: {
        if (visible) inputGateName.forceActiveFocus()
    }

    onAccepted: {
        videoPlayer.currentGateItem.name = inputGateName.text
        gateModel.insertData(videoPlayer.currentGateItem)
        inputGateName.text = ""
    }

    onRejected: {
        videoPlayer.currentGateItem.destroy()
    }
}
