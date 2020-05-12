import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import com.elte.t3ss3r4ct 1.0

Item {

    property bool isGatePlacingEnabled: false

    GroupBox {
        id: gateListWrapper
        anchors.fill: parent
        anchors.bottomMargin: 40
        title: "Gates"
        background: Rectangle {
            width: parent.width
            border.color: "#777777"
            color: "transparent"
        }
        label: Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: parent.width
            height: title.font.pixelSize + 20
            border.color: "#777777"
            color: "transparent"

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
                renderType: Text.NativeRendering
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
                    text: name
                    anchors.fill: parent
                    padding: 5
                    verticalAlignment: Text.AlignVCenter
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

    Button {
        id: btnAddGate
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width / 2 - 5
        height: 30

        background: Rectangle {
            border.color: isGatePlacingEnabled ? "#CD5555" : "#777777"
            color: isGatePlacingEnabled ? "#FFD1D1" : "#FFFFFF"
        }
        contentItem: Text {
            text: "ADD GATE"
            font.pointSize: 8
            color: isGatePlacingEnabled ? "#CD5555" : "#222222"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            renderType: Text.NativeRendering
        }

        onClicked: isGatePlacingEnabled = !isGatePlacingEnabled
    }

    Button {
        id: btnDeleteGate
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width / 2 - 5
        height: 30
        text: "DELETE GATE"
        font.pointSize: 8
        enabled: gateList.count > 0

        background: Rectangle {
            border.color: enabled ? "#777777" : "#CCCCCC"
            color: "transparent"
        }
        contentItem: Text {
            text: "DELETE GATE"
            font.pointSize: 8
            color: enabled ? "#222222" : "#AAAAAA"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            renderType: Text.NativeRendering
        }

        onClicked: gateModel.removeData(gateList.currentIndex)
    }
}
