pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Rectangle {
    id: root

    property int machineState: PlcTypes.Idle
    property bool alarmBlinking: false
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property int targetSpeed: 1500
    property string latestStatus: "READY FOR OPERATOR COMMANDS"

    height: 1
    color: root.machineState === PlcTypes.Error ? (root.alarmBlinking ? HmiTheme.statusError : HmiTheme.footerBgError) : HmiTheme.panelAltBg

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 1
        text: root.machineState === PlcTypes.Error ? "▲ CRITICAL FAULT: OVER-TEMP LIMIT (" + root.temperature.toFixed(1) + "°C >= " + root.maxTempLimit.toFixed(1) + "°C) — PRESS [R] TO RESET" : "● STATUS: " + root.latestStatus
        color: root.machineState === PlcTypes.Error ? (root.alarmBlinking ? HmiTheme.badgeInk : HmiTheme.textDanger) : HmiTheme.textPrimary
        bold: true
    }

    Text {
        visible: root.width >= 96 && root.machineState !== PlcTypes.Error
        anchors.right: parent.right
        anchors.rightMargin: 1
        text: "SET: " + root.targetSpeed + " RPM · LIM: " + root.maxTempLimit.toFixed(1) + "°C"
        color: HmiTheme.textDim
    }
}
