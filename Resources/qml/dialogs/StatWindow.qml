import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
import QtCharts 2.14
//import QtQuick.Controls.Styles 1.4

import com.elte.t3ss3r4ct 1.0

Dialog {

    id: statWindow
    width: 800
    height: 480
    title: "Traffic statistics"
//    standardButtons: Dialog.NoButton
    visible: true

    contentItem: Rectangle {

        Label {
            x: 20
            y: 10
            height: 26
            verticalAlignment: Text.AlignVCenter
            text: "Gate:"
        }

        ComboBox {
            id: cbGate
            x: 60
            y: 10
            width: 200
            height: 26
            model: gateModel
            textRole: "name"
            onActivated: updateGraph(gateModel.getData(currentIndex), intervalOptions.get(cbInterval.currentIndex).value)
        }

        Label {
            x: 300
            y: 10
            height: 26
            verticalAlignment: Text.AlignVCenter
            text: "Interval:"
        }

        ComboBox {
            id: cbInterval
            x: 350
            y: 10
            width: 100
            height: 26
            textRole: "key"
            model: ListModel {
                id: intervalOptions
                ListElement { key: "10 seconds"; value: 10 }
                ListElement { key: "30 seconds"; value: 30 }
                ListElement { key: "1 minute"; value: 60 }
            }
            onActivated: updateGraph(gateModel.getData(cbGate.currentIndex), intervalOptions.get(currentIndex).value)
        }

        ChartView {
            id: chartView
            title: "Bar series"
            anchors.fill: parent
            anchors.topMargin: 40
            legend.alignment: Qt.AlignBottom
            antialiasing: true

            StackedBarSeries {
                id: mySeries
                axisX: BarCategoryAxis {
                    id: x_axis
                    categories: statModel.axis_X_labels
                }
                axisY: ValueAxis {
                    id: y_axis
                    max: statModel.axis_Y_maxval
                }

                VBarModelMapper {
                    id: modelMapper
                    model: statModel
                    firstBarSetColumn: 0
                    lastBarSetColumn: 4
                    firstRow: 0
                    rowCount: 1
                }
            }

//            MouseArea {
//                anchors.fill: parent
//                onClicked: {
//                    console.log(statModel.rowCount())
//                    modelMapper.rowCount = 5
//                }
//            }
        }
    }

    function updateGraph(gate, interval) {
        statModel.updateStat(gate, interval)
        x_axis.categories = statModel.axis_X_labels
        y_axis.max = statModel.axis_Y_maxval
        modelMapper.rowCount = statModel.intervalNr
        chartView.title = gate.name
    }
}
