import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import TrafficMapper 1.0

import "../custom-items"

Item {

    property alias isGatePlacingEnabled: btnAddGate.checked

    GroupBox {
        id: gateListWrapper
        anchors.fill: parent
        anchors.bottomMargin: 40
        title: "Gates"
        background: CustomItemBackground {}
        label: CustomItemBackground {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: parent.width
            height: title.font.pixelSize + 20

            Text {
                id: title
                text: "GATES"
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

        ListView {
            id: gateList
            anchors.fill: parent
            anchors.topMargin: title.font.pixelSize + 25
            model: gateModel
            focus: true

            delegate: Rectangle {
                width: parent.width
                height: 40
                color: (index % 2 == 0) ? "#DDDDDD" : "#EEEEEE"

                Label {
                    x: 5
                    y: 5
                    text: "Name: " + name
                }

                Label {
                    x: 5
                    y: 20
                    text: "Passed vehicles: " + sumVehicleNr
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: gateList.currentIndex = index
                }
            }

            highlight: Rectangle {
                border.color: "#cd5555"
                border.width: 1
                color: "transparent"
                z: 2
            }
        }
    }

    CustomButton {
        id: btnAddGate
        text: "ADD GATE"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width / 2 - 5
        height: 30
        enabled: videoPlayer.isVideoLoaded
        checkable: true
    }

    CustomButton {
        id: btnDeleteGate
        text: "DELETE GATE"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width / 2 - 5
        height: 30
        enabled: gateList.count > 0

        onClicked: gateModel.removeData(gateList.currentIndex)
    }
}
