import QtQuick 2.14
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15
import QtQuick.Window 2.2

import TrafficMapper 1.0

import "../custom-items"

Window {
    id: dlgCameraCalibration
    width: 980
    minimumWidth: 980
    maximumWidth: 980
    height: 638
    minimumHeight: 638
    maximumHeight: 638
    title: "Camera calibration"
    modality: Qt.WindowModal
    visible: false

    property alias isRectSettingEnabled: btnSetRect.checked

    CameraCalibration {
        id: cameraCalibration
        x: 10
        y: 10
        width: 960
        height: 540

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                if (!isRectSettingEnabled) return

                switch (cameraCalibration.pointSet)
                {
                case 0:
                    cameraCalibration.point_0 = Qt.point(mouse.x, mouse.y)
                    ++cameraCalibration.pointSet
                    break
                case 1:
                    cameraCalibration.point_1 = Qt.point(mouse.x, mouse.y)
                    ++cameraCalibration.pointSet
                    break
                case 2:
                    cameraCalibration.point_2 = Qt.point(mouse.x, mouse.y)
                    ++cameraCalibration.pointSet
                    break
                case 3:
                    cameraCalibration.point_3 = Qt.point(mouse.x, mouse.y)
                    ++cameraCalibration.pointSet
                    isRectSettingEnabled = false
                    break
                }
            }

            onPositionChanged: {
                if (cameraCalibration.pointSet > 0 && cameraCalibration.pointSet < 4) {
                    cameraCalibration.point_hover = Qt.point(mouse.x, mouse.y)
                    cameraCalibration.update()
                }
            }
        }
    }

    GroupBox {
        id: gbSetRect
        title: "Calibration rectangle"
        x: 10
        anchors.top: cameraCalibration.bottom
        anchors.topMargin: 10

        CustomButton {
            id: btnSetRect
            text: cameraCalibration.pointSet == 4 ? "CLEAR" : "SET"
            checkable: cameraCalibration.pointSet != 4

            onClicked: {
                cameraCalibration.pointSet = 0
                cameraCalibration.update()
            }
        }
    }

    GroupBox {
        id: gbDistances
        title: "Distances (meter)"
        anchors.top: cameraCalibration.bottom
        anchors.topMargin: 10
        anchors.left: gbSetRect.right
        anchors.leftMargin: 10

        RowLayout {
            spacing: 10

            Repeater {
                id: distanceRepeater
                model: ["#CD5555", "#336699"]

                Rectangle {
                    color: modelData
                    width: 70
                    height: 30

                    property real realValue: spinbox.value / 100
                
                    SpinBox {
                        id: spinbox
                        from: 0
                        value: 1000
                        to: 100 * 100
                        stepSize: 10
                        anchors.centerIn: parent
                        width: 64
                        editable: true

                        validator: DoubleValidator {
                            bottom: Math.min(spinbox.from, spinbox.to)
                            top:  Math.max(spinbox.from, spinbox.to)
                        }

                        textFromValue: function(value, locale) {
                            return Number(value / 100).toLocaleString(locale, 'f', 2)
                        }

                        valueFromText: function(text, locale) {
                            return Number.fromLocaleString(locale, text) * 100
                        }

                        MouseArea {
                            anchors.fill: parent
                            anchors.rightMargin: 20

                            onWheel: {
                                if (wheel.angleDelta.y > 0)
                                    spinbox.value += 100
                                else
                                    spinbox.value -= 100
                            }
                        }
                    }
                }
            }

            CustomButton {
                text: "Apply"
                enabled: cameraCalibration.pointSet == 4

                onClicked: {
                    cameraCalibration.distance_0 = distanceRepeater.itemAt(0).realValue
                    cameraCalibration.distance_1 = distanceRepeater.itemAt(1).realValue
                    cameraCalibration.calculateHomography()
                    cameraCalibration.update()
                }
            }
        }
    }

    CustomButton {
        id: btnSave
        text: "SAVE"
        fontSize: 10
        fontBold: true
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        onClicked: {
            vehicleModel.loadHomographyMatrix(cameraCalibration)
            dlgCameraCalibration.visible = false
        }
    }

    CustomButton {
        id: btnCancel
        text: "CANCEL"
        fontSize: 10
        fontBold: true
        anchors.right: btnSave.left
        anchors.rightMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        onClicked: {
            cameraCalibration.pointSet = 0
            dlgCameraCalibration.visible = false
        }
    }

    function initAndOpen() {
        cameraCalibration.loadRandomFrame()
        cameraCalibration.update()
        dlgCameraCalibration.visible = true
    }
}
