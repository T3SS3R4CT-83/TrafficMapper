import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Controls.Styles 1.4

import com.elte.t3ss3r4ct 1.0

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
                text: "Detections"
                checked: GlobalMeta.PLAYER_SHOW_DETECTIONS ? Qt.Checked : Qt.Unchecked
                onClicked: {
                    if (checked)
                        GlobalMeta.PLAYER_SHOW_DETECTIONS = true
                    else
                        GlobalMeta.PLAYER_SHOW_DETECTIONS = false
                }
            }
            
            Switch {
                text: "Paths"
                checked: GlobalMeta.PLAYER_SHOW_PATHS ? Qt.Checked : Qt.Unchecked
                onClicked: {
                    if (checked)
                        GlobalMeta.PLAYER_SHOW_PATHS = true
                    else
                        GlobalMeta.PLAYER_SHOW_PATHS = false
                }
            }
            
            Switch {
                text: "Labels"
                checked: GlobalMeta.PLAYER_SHOW_LABELS ? Qt.Checked : Qt.Unchecked
                onClicked: {
                    if (checked)
                        GlobalMeta.PLAYER_SHOW_LABELS = true
                    else
                        GlobalMeta.PLAYER_SHOW_LABELS = false
                }
            }
            
            Switch {
                text: "Positions"
                checked: GlobalMeta.PLAYER_SHOW_POSITIONS ? Qt.Checked : Qt.Unchecked
                onClicked: {
                    if (checked)
                        GlobalMeta.PLAYER_SHOW_POSITIONS = true
                    else
                        GlobalMeta.PLAYER_SHOW_POSITIONS = false
                }
            }
        }
    }
}
