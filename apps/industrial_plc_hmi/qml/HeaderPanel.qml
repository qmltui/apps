pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Rectangle {
    id: root

    property int machineState: PlcTypes.Idle
    property string stationId: "STN#42"
    property int cycles: 0
    property bool alarmBlinking: false
    property string clockText: ""

    readonly property bool validState: root.machineState === PlcTypes.Idle || root.machineState === PlcTypes.Processing || root.machineState === PlcTypes.Error || root.machineState === PlcTypes.Maintenance
    readonly property bool isFault: !root.validState || root.machineState === PlcTypes.Error
    readonly property bool showExtended: width >= 94
    readonly property bool showClock: width >= 76

    signal skinCycleRequested

    height: 1
    color: root.isFault ? (root.alarmBlinking ? HmiTheme.headerBgErrorPulse : HmiTheme.headerBgError) : HmiTheme.headerBg

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1

        Text {
            text: "◆"
            color: HmiTheme.stateColor(root.machineState, root.validState)
            bold: true
        }

        Text {
            text: "PLC CONTROLLER & HMI · " + root.stationId
            color: HmiTheme.textPrimary
            bold: true
        }

        Text {
            visible: root.showExtended
            text: "│ CYC #" + String(root.cycles).padStart(4, "0") + " │"
            color: HmiTheme.textDim
        }

        Rectangle {
            visible: root.showExtended
            width: skinLabel.implicitWidth + 2
            height: 1
            color: HmiTheme.panelAltBg

            Text {
                id: skinLabel
                anchors.centerIn: parent
                text: "K:" + HmiTheme.skinName
                color: HmiTheme.textHighlight
                bold: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.skinCycleRequested()
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        Text {
            visible: root.showClock && root.clockText.length > 0
            text: root.clockText
            color: HmiTheme.textSecondary
        }

        Rectangle {
            width: 15
            height: 1
            color: HmiTheme.stateColor(root.machineState, root.validState)

            Text {
                anchors.centerIn: parent
                text: HmiTheme.stateBadgeText(root.machineState, root.validState)
                color: HmiTheme.badgeInk
                bold: true
            }
        }
    }
}
