pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property int machineState: PlcTypes.Idle
    property int speed: 0
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property double pressure: 1.0
    property bool valveA: false
    property bool valveB: false
    property int pulseFrame: 0

    readonly property bool spacious: height >= 9
    readonly property bool wide: width >= 44
    readonly property double flowLpm: root.machineState === PlcTypes.Processing ? (12.0 + (root.speed / 3000.0) * 58.0) : (root.machineState === PlcTypes.Maintenance ? 6.5 : 0.0)
    readonly property list<string> rotorGlyphs: ["◐", "◓", "◑", "◒"]
    readonly property list<string> pipeFlows: ["══▶", "═▶═", "▶══"]
    readonly property string activePipe: root.pipeFlows[root.pulseFrame % root.pipeFlows.length]
    readonly property string rotorChar: (root.machineState === PlcTypes.Processing || root.machineState === PlcTypes.Maintenance) ? root.rotorGlyphs[root.pulseFrame % root.rotorGlyphs.length] : "○"
    readonly property color vesselColor: root.temperature >= root.maxTempLimit ? HmiTheme.statusError : (root.temperature >= root.maxTempLimit * 0.85 ? HmiTheme.textWarning : HmiTheme.traceTemp)
    readonly property string vesselFillBar: {
        const ratio = Math.max(0.0, Math.min(1.0, (root.temperature - 20.0) / 80.0));
        const blocks = ["▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"];
        const g = blocks[Math.min(7, Math.floor(ratio * 8))];
        return g.repeat(root.wide ? 8 : 4);
    }

    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

    title: Text {
        text: root.wide ? "◆ P&ID MIMIC · REACTOR LOOP" : "◆ MIMIC"
        color: HmiTheme.textSecondary
        bold: true
    }

    titleTrailing: Text {
        text: root.flowLpm.toFixed(1) + " L/m"
        color: root.flowLpm > 0 ? HmiTheme.textSuccess : HmiTheme.textDim
        bold: root.flowLpm > 0
    }

    footer: Text {
        text: root.machineState === PlcTypes.Error ? "▲ TRIP LOCK" : "● E-STOP OK"
        color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
    }

    footerTrailing: Text {
        visible: root.wide
        text: root.pressure.toFixed(2) + " bar"
        color: HmiTheme.tracePressure
    }

    Rectangle {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        color: HmiTheme.recessedBg

        Column {
            anchors.centerIn: parent
            width: Math.max(10, parent.width - 2)
            spacing: root.spacious ? 1 : 0

            // Top schematic row: Feed -> Valve A -> Vessel Top -> Valve B -> Out
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 0

                Text {
                    visible: root.wide
                    text: "[FEED]"
                    color: HmiTheme.textDim
                }

                Text {
                    visible: root.wide
                    text: root.valveA ? root.activePipe : "───"
                    color: root.valveA ? HmiTheme.statusProcessing : HmiTheme.textDim
                    bold: root.valveA
                }

                Text {
                    text: root.valveA ? "(⋈A●)" : "(⋈A○)"
                    color: root.valveA ? HmiTheme.textSuccess : HmiTheme.textDim
                    bold: root.valveA
                }

                Text {
                    text: root.valveA ? "═╦▶" : "─┬─"
                    color: root.valveA ? HmiTheme.statusProcessing : HmiTheme.textDim
                }

                Text {
                    text: root.wide ? "╭────────╮" : "╭────╮"
                    color: root.vesselColor
                    bold: true
                }

                Text {
                    text: root.valveB ? "▶" : "─"
                    color: root.valveB ? HmiTheme.statusProcessing : HmiTheme.textDim
                }

                Text {
                    text: root.valveB ? "(⋈B●)" : "(⋈B○)"
                    color: root.valveB ? HmiTheme.textSuccess : HmiTheme.textDim
                    bold: root.valveB
                }

                Text {
                    visible: root.wide
                    text: root.valveB ? root.activePipe : "───"
                    color: root.valveB ? HmiTheme.statusProcessing : HmiTheme.textDim
                    bold: root.valveB
                }

                Text {
                    visible: root.wide
                    text: "[OUT]"
                    color: root.valveB ? HmiTheme.textPrimary : HmiTheme.textDim
                }
            }

            // Middle schematic row: Drive M1 + Vessel Thermal Core + Live Readout
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 1

                Text {
                    text: "[M1 " + root.rotorChar + " " + String(root.speed).padStart(4, " ") + "]"
                    color: root.speed > 0 ? HmiTheme.traceSpeed : HmiTheme.textDim
                    bold: root.speed > 0
                }

                Row {
                    spacing: 0

                    Text {
                        text: "│"
                        color: root.vesselColor
                    }

                    Text {
                        text: root.vesselFillBar
                        color: root.vesselColor
                        bold: true
                    }

                    Text {
                        text: "│"
                        color: root.vesselColor
                    }
                }

                Text {
                    visible: root.wide
                    text: root.temperature.toFixed(1) + "°C"
                    color: root.vesselColor
                    bold: true
                }
            }

            // Vessel bottom cap + status callout
            Row {
                visible: parent.parent.height >= 4
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 1

                Text {
                    text: root.wide ? (root.machineState === PlcTypes.Processing ? "▸ AGITATOR ON " : (root.machineState === PlcTypes.Maintenance ? "▸ JOG 150 RPM " : "▪ DRIVE IDLE  ")) : (root.machineState === PlcTypes.Processing ? "▸ RUN      " : (root.machineState === PlcTypes.Maintenance ? "▸ JOG      " : "▪ IDLE     "))
                    color: root.machineState === PlcTypes.Processing ? HmiTheme.textSuccess : (root.machineState === PlcTypes.Maintenance ? HmiTheme.textWarning : HmiTheme.textDim)
                }

                Text {
                    text: root.wide ? "╰────────╯" : "╰────╯"
                    color: root.vesselColor
                    bold: true
                }

                Text {
                    visible: root.wide
                    text: "LIM " + root.maxTempLimit.toFixed(0) + "°C"
                    color: HmiTheme.textDim
                }
            }
        }
    }
}
