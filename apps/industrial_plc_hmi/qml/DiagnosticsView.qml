pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property int cycles: 0
    property bool valveA: false
    property bool valveB: false
    property int machineState: PlcTypes.Idle
    property double busVoltage: 24.1
    property double sensorLoopMa: 11.4
    property var busVoltageHistory: [24.1, 24.1, 24.0, 24.2, 24.1, 24.1, 24.0, 24.2, 24.1, 24.1]
    property var sensorLoopHistory: [11.4, 11.5, 11.4, 11.3, 11.5, 11.4, 11.6, 11.4, 11.5, 11.4]
    property var packetRateHistory: [12, 18, 25, 40, 65, 80, 95, 70, 85, 100, 110, 120]

    readonly property bool wide: width >= 96
    readonly property bool tall: height >= 22
    readonly property int innerWidth: Math.max(20, width - 4)
    readonly property int innerHeight: Math.max(14, height - 2)
    readonly property int halfWidth: Math.floor((innerWidth - 1) / 2)
    readonly property int topSlotHeight: Math.max(8, Math.floor((innerHeight - 1) * 0.52))
    readonly property int bottomSlotHeight: Math.max(6, innerHeight - 1 - topSlotHeight)

    anchors.fill: parent
    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

    title: Text {
        text: "◆ PLC HARDWARE & SUBSYSTEM DIAGNOSTICS"
        color: HmiTheme.textHighlight
        bold: true
    }

    titleTrailing: Text {
        visible: root.wide
        text: "RACK 00 · S7-1500 I/O"
        color: HmiTheme.textSecondary
    }

    footer: Text {
        text: "SCAN: 1000 ms · WATCHDOG ARMED"
        color: HmiTheme.textDim
    }

    footerTrailing: Text {
        text: "PROFINET LINK: UP ●"
        color: HmiTheme.textSuccess
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // LEFT RACK COLUMN: Digital Inputs & Digital Outputs
        Column {
            width: root.halfWidth
            height: root.innerHeight
            spacing: 1

            Panel {
                width: parent.width
                height: root.topSlotHeight
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle

                title: Text {
                    text: "SLOT 01 · DIGITAL INPUTS (DI)"
                    color: HmiTheme.textSecondary
                    bold: true
                }

                titleTrailing: Text {
                    visible: root.wide
                    text: "24VDC"
                    color: HmiTheme.textDim
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: root.tall ? 1 : 0

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: HmiTheme.recessedBg

                        Text {
                            anchors.centerIn: parent
                            text: root.machineState === PlcTypes.Error ? "CH: [1●] [2▲] [3●] [4○] [5●] [6●]" : (root.machineState === PlcTypes.Processing ? "CH: [1●] [2●] [3●] [4●] [5●] [6●]" : "CH: [1●] [2●] [3●] [4○] [5●] [6○]")
                            color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
                            bold: true
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: 0

                        Text {
                            text: "DI-01 E-STOP LOOP      [● NORMAL]"
                            color: HmiTheme.textSuccess
                        }

                        Text {
                            text: "DI-02 THERMAL OVERLOAD " + (root.machineState === PlcTypes.Error ? "[▲ TRIP! ]" : "[● NORMAL]")
                            color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
                            bold: root.machineState === PlcTypes.Error
                        }

                        Text {
                            text: "DI-03 PRESSURE SWITCH  [● CLOSED]"
                            color: HmiTheme.textPrimary
                        }

                        Text {
                            text: "DI-04 FLOW SENSOR      " + (root.machineState === PlcTypes.Processing ? "[● ACTIVE]" : "[○ IDLE  ]")
                            color: root.machineState === PlcTypes.Processing ? HmiTheme.textSuccess : HmiTheme.textDim
                        }

                        Text {
                            visible: root.tall
                            text: "DI-05 SAFETY GUARD     [● LOCKED]"
                            color: HmiTheme.textPrimary
                        }
                    }
                }
            }

            Panel {
                width: parent.width
                height: root.bottomSlotHeight
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle

                title: Text {
                    text: "SLOT 02 · DIGITAL OUTPUTS (DO)"
                    color: HmiTheme.textSecondary
                    bold: true
                }

                titleTrailing: Text {
                    visible: root.wide
                    text: "RELAY"
                    color: HmiTheme.textDim
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: 0

                    Rectangle {
                        visible: root.tall
                        width: parent.width
                        height: 1
                        color: HmiTheme.recessedBg

                        Text {
                            anchors.centerIn: parent
                            text: "CH: [1" + (root.valveA ? "●" : "○") + "] [2" + (root.valveB ? "●" : "○") + "] [3" + (root.machineState === PlcTypes.Processing ? "●" : "○") + "] [4" + (root.machineState === PlcTypes.Error ? "▲" : "○") + "]"
                            color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textHighlight
                            bold: true
                        }
                    }

                    Text {
                        text: "DO-01 VALVE A COIL     " + (root.valveA ? "[● ON ]" : "[○ OFF]")
                        color: root.valveA ? HmiTheme.textSuccess : HmiTheme.textDim
                        bold: root.valveA
                    }

                    Text {
                        text: "DO-02 VALVE B COIL     " + (root.valveB ? "[● ON ]" : "[○ OFF]")
                        color: root.valveB ? HmiTheme.textSuccess : HmiTheme.textDim
                        bold: root.valveB
                    }

                    Text {
                        text: "DO-03 MAIN CONTACTOR   " + (root.machineState === PlcTypes.Processing ? "[● ON ]" : "[○ OFF]")
                        color: root.machineState === PlcTypes.Processing ? HmiTheme.textSuccess : HmiTheme.textDim
                        bold: root.machineState === PlcTypes.Processing
                    }

                    Text {
                        visible: root.tall
                        text: "DO-04 ALARM STROBE     " + (root.machineState === PlcTypes.Error ? "[▲ ON ]" : "[○ OFF]")
                        color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textDim
                        bold: root.machineState === PlcTypes.Error
                    }
                }
            }
        }

        // RIGHT RACK COLUMN: Analog Loops & Fieldbus Stack
        Column {
            width: Math.max(16, root.innerWidth - root.halfWidth - 1)
            height: root.innerHeight
            spacing: 1

            Panel {
                width: parent.width
                height: root.topSlotHeight
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle

                title: Text {
                    text: "SLOT 03 · POWER & ANALOG BUS"
                    color: HmiTheme.textSecondary
                    bold: true
                }

                titleTrailing: Text {
                    visible: root.wide
                    text: "4-20mA"
                    color: HmiTheme.textDim
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: root.tall ? 1 : 0

                    Column {
                        width: parent.width
                        spacing: 0

                        Item {
                            width: parent.width
                            height: 1

                            Text {
                                anchors.left: parent.left
                                text: "24V Field Bus"
                                color: HmiTheme.textPrimary
                            }

                            Text {
                                anchors.right: parent.right
                                text: root.busVoltage.toFixed(2) + " VDC [OK]"
                                color: HmiTheme.textSuccess
                                bold: true
                            }
                        }

                        Sparkline {
                            width: parent.width
                            height: root.tall ? 2 : 1
                            filled: root.tall
                            values: root.busVoltageHistory
                            from: 23.0
                            to: 25.0
                            color: HmiTheme.statusIdle
                            backgroundColor: HmiTheme.recessedBg
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: 0

                        Item {
                            width: parent.width
                            height: 1

                            Text {
                                anchors.left: parent.left
                                text: "Thermal Loop"
                                color: HmiTheme.textPrimary
                            }

                            Text {
                                anchors.right: parent.right
                                text: root.sensorLoopMa.toFixed(1) + " mA"
                                color: HmiTheme.traceTemp
                                bold: true
                            }
                        }

                        Sparkline {
                            width: parent.width
                            height: root.tall ? 2 : 1
                            filled: root.tall
                            values: root.sensorLoopHistory
                            from: 4.0
                            to: 20.0
                            color: HmiTheme.traceTemp
                            backgroundColor: HmiTheme.recessedBg
                        }
                    }
                }
            }

            Panel {
                width: parent.width
                height: root.bottomSlotHeight
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle

                title: Text {
                    text: "SLOT 04 · PROFINET / MODBUS"
                    color: HmiTheme.textSecondary
                    bold: true
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: 0

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Packets Transmitted"
                            color: HmiTheme.textPrimary
                        }

                        Text {
                            anchors.right: parent.right
                            text: String(root.cycles * 12 + 24)
                            color: HmiTheme.textHighlight
                            bold: true
                        }
                    }

                    Sparkline {
                        width: parent.width
                        height: root.tall ? 2 : 1
                        filled: root.tall
                        values: root.packetRateHistory
                        from: 0
                        to: 160
                        color: HmiTheme.textAccent
                        backgroundColor: HmiTheme.recessedBg
                    }

                    Text {
                        text: "CRC Error Counter   : 0 ERRORS"
                        color: HmiTheme.textSuccess
                        bold: true
                    }
                }
            }
        }
    }
}
