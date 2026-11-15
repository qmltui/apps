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
    property double peakTemperature: 28.5
    property double busVoltage: 24.1
    property double sensorLoopMa: 5.7
    property int cycles: 0
    property int errorCount: 0
    property int pulseFrame: 0
    property bool valveA: false
    property bool valveB: false
    property bool tallLayout: false
    property var speedHistory: []
    property var tempHistory: []
    property var pressureHistory: []

    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

    title: Text {
        text: "◆ TELEMETRY"
        color: HmiTheme.textSecondary
        bold: true
    }

    titleTrailing: Text {
        visible: root.width >= 27
        text: "60s"
        color: HmiTheme.textDim
    }

    footer: Text {
        text: "CYC:" + root.cycles
        color: HmiTheme.textSecondary
    }

    footerTrailing: Text {
        text: "ALM:" + root.errorCount
        color: root.errorCount > 0 ? HmiTheme.textDanger : HmiTheme.textDim
        bold: root.errorCount > 0
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: root.tallLayout ? 1 : 0

        Gauge {
            label: "Speed"
            value: root.speed
            unit: "RPM"
            from: 0
            to: 3000
            warningVal: 2400
            dangerVal: 2800
            decimals: 0
            history: root.speedHistory
            sparkHeight: root.tallLayout ? 2 : 1
            traceColor: HmiTheme.traceSpeed
        }

        Gauge {
            label: "Temp"
            value: root.temperature
            unit: "°C"
            from: 20
            to: 100
            warningVal: 72
            dangerVal: root.maxTempLimit
            decimals: 1
            history: root.tempHistory
            sparkHeight: root.tallLayout ? 2 : 1
            traceColor: HmiTheme.traceTemp
        }

        Gauge {
            label: "Pressure"
            value: root.pressure
            unit: "bar"
            from: 0
            to: 5
            warningVal: 3.5
            dangerVal: 4.5
            decimals: 2
            history: root.pressureHistory
            sparkHeight: root.tallLayout ? 2 : 1
            traceColor: HmiTheme.tracePressure
        }

        Column {
            width: parent.width
            spacing: 0

            Text {
                text: "▸ ACTUATORS & FLOW"
                color: HmiTheme.textSecondary
                bold: true
            }

            Row {
                width: parent.width
                spacing: 1

                ActuatorLed {
                    name: "V-A"
                    active: root.valveA
                }

                ActuatorLed {
                    name: "V-B"
                    active: root.valveB
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: HmiTheme.recessedBg

                Text {
                    anchors.centerIn: parent
                    text: {
                        const frames = [">  ", ">> ", ">>>", " >>", "  >"];
                        if (root.machineState === PlcTypes.Processing)
                            return "▐ FLOW " + frames[root.pulseFrame % frames.length] + " ACTIVE ▌";
                        if (root.machineState === PlcTypes.Maintenance)
                            return "▐ MAINT JOG " + (root.pulseFrame % 2 === 0 ? "◧" : "◨") + " ▌";
                        if (root.machineState === PlcTypes.Error)
                            return "▐ ▲ LOCKOUT TRIP ▌";
                        return "▐ ■ LINE STANDBY ▌";
                    }
                    color: root.machineState === PlcTypes.Processing ? HmiTheme.textSuccess : (root.machineState === PlcTypes.Error ? HmiTheme.textDanger : (root.machineState === PlcTypes.Maintenance ? HmiTheme.textWarning : HmiTheme.textDim))
                    bold: true
                }
            }
        }

        Panel {
            visible: root.tallLayout
            width: parent.width
            height: Math.max(3, parent.height - 20)
            color: HmiTheme.panelAltBg
            border.style: Border.Rounded
            border.color: HmiTheme.borderSubtle

            title: Text {
                text: "DUTY SUMMARY"
                color: HmiTheme.textDim
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
                        text: "Peak Temp"
                        color: HmiTheme.textDim
                    }

                    Text {
                        anchors.right: parent.right
                        text: root.peakTemperature.toFixed(1) + " °C"
                        color: HmiTheme.textPrimary
                    }
                }

                Item {
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Margin"
                        color: HmiTheme.textDim
                    }

                    Text {
                        anchors.right: parent.right
                        text: (root.maxTempLimit - root.temperature).toFixed(1) + " °C"
                        color: root.temperature >= root.maxTempLimit ? HmiTheme.textDanger : HmiTheme.textSuccess
                        bold: true
                    }
                }

                Item {
                    visible: parent.height >= 3
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Field Bus"
                        color: HmiTheme.textDim
                    }

                    Text {
                        anchors.right: parent.right
                        text: root.busVoltage.toFixed(1) + " VDC"
                        color: HmiTheme.statusIdle
                    }
                }

                Item {
                    visible: parent.height >= 4
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Loop Out"
                        color: HmiTheme.textDim
                    }

                    Text {
                        anchors.right: parent.right
                        text: root.sensorLoopMa.toFixed(1) + " mA"
                        color: HmiTheme.traceTemp
                    }
                }
            }
        }
    }
}
