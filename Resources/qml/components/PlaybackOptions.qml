import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4

import TrafficMapper 1.0

Item {
    height: cbWrapper.height + title.font.pixelSize + 35

    GroupBox {
        anchors.fill: parent
        background: Rectangle {
            border.color: "#AAAAAA"
            color: "transparent"
        }
        label: Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: parent.width
            height: title.font.pixelSize + 20
            border.color: "#AAAAAA"
            color: "transparent"

            Text {
                id: title
                text: "PLAYBACK OPTIONS"
                color: "#444444"
                anchors.fill: parent
                font.pixelSize: 12
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
//                renderType: Text.NativeRendering
            }
        }

        Column {
            id: cbWrapper
            y: title.font.pixelSize + 20
            
            Switch {
                id: swDetections
                text: "Detections"
                checked: true
                onClicked: changePlaybackOptions()
            }
            
            Switch {
                id: swPaths
                text: "Paths"
                checked: true
                onClicked: changePlaybackOptions()
            }
            
            Switch {
                id: swLabels
                text: "Labels"
                checked: true
                onClicked: changePlaybackOptions()
            }
            
            Switch {
                id: swPositions
                text: "Positions"
                checked: true
                onClicked: changePlaybackOptions()
            }
        }
    }

    function changePlaybackOptions() {
        videoOverlay.setPlaybackOptions(
            swDetections.checked,
            swPaths.checked,
            swLabels.checked,
            swPositions.checked
        );
    }
}
