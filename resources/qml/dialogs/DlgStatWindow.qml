import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
import QtCharts 2.14

import TrafficMapper 1.0

Dialog {
    id: statWindow
    width: 1000
    height: 600
    title: "Traffic statistics"
    visible: false

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
            onActivated: function(index) {
//                statModel.changeDisplayedGate(gateModel.getData(index));
                statModel.updateStat(gateModel.getData(currentIndex), intervalOptions.get(cbInterval.currentIndex).value)
            }
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
            onActivated: function(index) {
                // statModel.changeDisplayedInterval(intervalOptions.get(index).value)
                statModel.updateStat(gateModel.getData(cbGate.currentIndex), intervalOptions.get(currentIndex).value)
            }
//            onActivated: {
//                if (intervalOptions.get(currentIndex).value == 10) {
//                    x_axis.labelsFont.pointSize = 6
//                    x_axis.labelsAngle = -90
//                } else {
//                    x_axis.labelsFont.pointSize = 8
//                    x_axis.labelsAngle = 0
//                }
//            }
        }

        CheckBox {
            id: cbShowLabels
            x: 470
            y: 10
            height: 26
            checked: true
            text: "Show bar labels"
        }

        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.rightMargin: 20
            text: "Save chart..."

            onClicked: dlgSaveChart.open()
        }

        ChartView {
            id: chartView
            anchors.fill: parent
            anchors.topMargin: 40
            antialiasing: true
            legend.alignment: Qt.AlignBottom
            legend.font.pointSize: 10
            legend.font.bold: true
            title: statModel.graphTitle
            titleFont.pointSize: 11
            titleFont.bold: true
            titleFont.letterSpacing: 2

            StackedBarSeries {
                id: mySeries
                labelsVisible: cbShowLabels.checked

//                axisX: BarCategoryAxis { categories: ["2007", "2008", "2009", "2010", "2011", "2012" ] }
//                BarSet { label: "Bob"; values: [2, 2, 3, 4, 5, 6] }
//                BarSet { label: "Susan"; values: [5, 1, 2, 4, 1, 7] }
//                BarSet { label: "James"; values: [3, 5, 8, 13, 5, 8] }

                axisX: BarCategoryAxis {
                    id: x_axis
                    categories: statModel.axis_X_labels
                    labelsAngle: -90
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
                    rowCount: statModel.intervalNr
                }
            }
        }
    }

    DlgSaveChart { id: dlgSaveChart }



    function updateGraph(gate, interval) {
        statModel.updateStat(gate, interval)
//        x_axis.categories = statModel.axis_X_labels
//        y_axis.max = statModel.axis_Y_maxval
//        modelMapper.rowCount = statModel.intervalNr
//        chartView.title = gate.name
    }
}
