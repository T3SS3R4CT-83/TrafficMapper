import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Button {
    id: customButton
    implicitHeight: 30
    implicitWidth: 100
    hoverEnabled: true
    checkable: false

    property int fontSize: 8
    property bool fontBold: false

    background: CustomButtonBackground {}
    contentItem: Text {
        text: customButton.text
        font.pointSize: fontSize
        font.bold: fontBold
        color: "#222222"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        // elide: Text.ElideRight
        // renderType: Text.NativeRendering
        smooth: true
    }
    states: [
        State {
            name: "hovered"; when: customButton.hovered && !customButton.checked && customButton.enabled
            PropertyChanges { target: customButton.background; color: "#DDDDDD" }
        },
        State {
            name: "checked"; when: customButton.checked && customButton.enabled
            PropertyChanges { target: customButton.background; border.color: "#CD5555"; color: "#FFD1D1" }
            PropertyChanges { target: customButton.contentItem; color: "#CD5555" }
        },
        State {
            name: "disabled"; when: !customButton.enabled
            PropertyChanges { target: customButton.background; border.color: "#DDDDDD" }
            PropertyChanges { target: customButton.contentItem; color: "#AAAAAA" }
        }
    ]
}
