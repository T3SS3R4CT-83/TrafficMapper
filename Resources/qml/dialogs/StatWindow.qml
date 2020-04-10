import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.2
import QtCharts 2.14
//import QtQuick.Controls.Styles 1.4

import com.elte.t3ss3r4ct 1.0

Dialog {

    id: statWindow
    width: 800
    height: 600
    title: "Traffic statistics"
//    standardButtons: Dialog.NoButton
    visible: false

    contentItem: Rectangle {

        ChartView {
            title: "Bar series"
            anchors.fill: parent
            anchors.bottomMargin: 50
            legend.alignment: Qt.AlignBottom
            antialiasing: true

            StackedBarSeries {
                id: mySeries
                axisX: BarCategoryAxis {
                    id: x_axis
                    categories: ["2007", "2008", "2009", "2010", "2011", "2012" ]
                }
                axisY: ValueAxis {
                    id: y_axis
                }
                BarSet {
                    id: lblBus
                    label: "Buses"
                    values: [2, 2, 3, 4, 5, 6]
                }
                BarSet {
                    id: lblCars
                    label: "Cars"
                    values: [2, 2, 3, 4, 5, 6]
                }
                BarSet {
                    id: lblTruck
                    label: "Trucks"
                    values: [2, 2, 3, 4, 5, 6]
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                }
            }
        }

        ComboBox {
            textRole: "key"
            model: ListModel {
                id: intervalOptions
                ListElement { key: "10 seconds"; value: 10 }
                ListElement { key: "30 seconds"; value: 30 }
                ListElement { key: "1 minute"; value: 60 }
            }
            onActivated: initGraph(intervalOptions.get(currentIndex).value)
        }

        Button {
            text: "Update"
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            onClicked: {
            }
        }
    }

    function initGraph(interval) {
        tracker.generateStatistics(null, interval)
        x_axis.categories = tracker.getAxisX()
        y_axis.max = tracker.getAxisY()
        lblBus.values = tracker.getBusValues()
        lblCars.values = tracker.getCarValues()
        lblTruck.values = tracker.getTruckValues()
    }
}
