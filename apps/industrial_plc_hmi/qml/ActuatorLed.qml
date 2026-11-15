pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Rectangle {
    id: root

    property string name: "VALVE"
    property bool active: false
    property color activeColor: HmiTheme.statusProcessing
    property color inactiveColor: HmiTheme.recessedBg

    width: parent ? Math.floor((parent.width - 1) / 2) : 10
    height: 1
    color: root.active ? root.activeColor : root.inactiveColor

    Text {
        anchors.centerIn: parent
        text: root.active ? "[" + root.name + ": ON]" : "[" + root.name + ": OFF]"
        color: root.active ? HmiTheme.badgeInk : HmiTheme.textDim
        bold: root.active
    }
}
