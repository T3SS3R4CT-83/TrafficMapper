import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import com.elte.t3ss3r4ct 1.0

Item {
    height: cbWrapper.height + title.font.pixelSize + 35

    GroupBox {
        anchors.fill: parent
        background: Rectangle {
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
                text: "PLAYBACK OPTIONS"
                color: "#444444"
                anchors.fill: parent
                font.pixelSize: 12
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                renderType: Text.NativeRendering
            }

//            Rectangle {
//                anchors.bottom: parent.bottom
//                anchors.horizontalCenter: parent.horizontalCenter
//                width: parent.width - 20
//                height: 1
//                color: "#444444"
//            }
        }

        Column {
            id: cbWrapper
            y: title.font.pixelSize + 20
            
            CheckBox {
                text: "Detections"
                checkState: GlobalMeta.PLAYER_SHOW_DETECTIONS ? Qt.Checked : Qt.Unchecked
                nextCheckState: function() {
                    if (checkState === Qt.Checked) {
                        GlobalMeta.PLAYER_SHOW_DETECTIONS = false
                        return Qt.Unchecked
                    } else {
                        GlobalMeta.PLAYER_SHOW_DETECTIONS = true
                        return Qt.Checked
                    }
                }
            }
            
            CheckBox {
                text: "Paths"
                checkState: GlobalMeta.PLAYER_SHOW_PATHS ? Qt.Checked : Qt.Unchecked
                nextCheckState: function() {
                    if (checkState === Qt.Checked) {
                        GlobalMeta.PLAYER_SHOW_PATHS = false
                        return Qt.Unchecked
                    } else {
                        GlobalMeta.PLAYER_SHOW_PATHS = true
                        return Qt.Checked
                    }
                }
            }
            
            CheckBox {
                text: "Labels"
                checkState: GlobalMeta.PLAYER_SHOW_LABELS ? Qt.Checked : Qt.Unchecked
                nextCheckState: function() {
                    if (checkState === Qt.Checked) {
                        GlobalMeta.PLAYER_SHOW_LABELS = false
                        return Qt.Unchecked
                    } else {
                        GlobalMeta.PLAYER_SHOW_LABELS = true
                        return Qt.Checked
                    }
                }
            }
            
            CheckBox {
                text: "Positions"
                checkState: GlobalMeta.PLAYER_SHOW_POSITIONS ? Qt.Checked : Qt.Unchecked
                nextCheckState: function() {
                    if (checkState === Qt.Checked) {
                        GlobalMeta.PLAYER_SHOW_POSITIONS = false
                        return Qt.Unchecked
                    } else {
                        GlobalMeta.PLAYER_SHOW_POSITIONS = true
                        return Qt.Checked
                    }
                }
            }
        }
    }

}
