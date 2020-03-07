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
    standardButtons: StandardButton.NoButton
    visible: false

    ChartView {
        title: "Bar series"
        anchors.fill: parent
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
                id: lblCars
                label: "Cars"
                values: [2, 2, 3, 4, 5, 6]
            }
            BarSet { label: "Susan"; values: [5, 1, 2, 4, 1, 7] }
            BarSet { label: "James"; values: [3, 5, 8, 13, 5, 8] }
        }
    }

    Button {
        text: "Update"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        onClicked: {
            x_axis.categories = tracker.getAxisX()
            y_axis.max = 30
            lblCars.values = tracker.getCarValues()
        }
    }
}
