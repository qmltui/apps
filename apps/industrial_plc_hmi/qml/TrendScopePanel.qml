pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import QmlTui.Charts
import IndustrialPlcHmi

Panel {
    id: root

    property int machineState: PlcTypes.Idle
    property int traceMode: PlcTypes.AllTraces
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property var tempHistory: []
    property var speedHistory: []

    signal traceCycleRequested

    color: HmiTheme.panelBg
    border {
        style: Border.Rounded
        color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal
    }

    title: Text {
        text: root.width >= 42 ? "◆ THERMAL & PROCESS SCOPE" : (root.width >= 34 ? "◆ TREND SCOPE" : "◆ SCOPE")
        color: HmiTheme.textSecondary
        bold: true
    }

    titleTrailing: Text {
        text: root.temperature.toFixed(1) + "° / " + root.maxTempLimit.toFixed(0) + "°C"
        color: root.temperature >= root.maxTempLimit ? HmiTheme.textDanger : HmiTheme.traceTemp
        bold: true
    }

    footer: Text {
        text: "← 60s"
        color: HmiTheme.textDim
    }

    footerTrailing: Text {
        text: root.width >= 34 ? (root.traceMode === PlcTypes.AllTraces ? "[T] ●TEMP ●RPM" : (root.traceMode === PlcTypes.TempOnly ? "[T] ●TEMP ONLY" : "[T] ●RPM ONLY")) : (root.traceMode === PlcTypes.AllTraces ? "[T] ALL" : (root.traceMode === PlcTypes.TempOnly ? "[T] TEMP" : "[T] RPM"))
        color: root.traceMode === PlcTypes.SpeedOnly ? HmiTheme.traceSpeed : HmiTheme.traceTemp
    }

    Rectangle {
        anchors {
            fill: parent
            leftMargin: 1
            rightMargin: 1
        }
        color: HmiTheme.recessedBg

        ChartView {
            anchors.fill: parent
            focusPolicy: Qt.NoFocus
            gridStyle: ChartView.Lines
            cellColorMode: ChartView.LastDot

            axisX: ValueAxis {
                visible: false
                showGrid: false
            }

            axisY: ValueAxis {
                min: 20.0
                max: 100.0
                tickCount: 3
                suffix: "°"
                color: HmiTheme.traceGrid
                labelColor: HmiTheme.textDim
                gridColor: HmiTheme.traceGrid
            }

            axisYRight: ValueAxis {
                id: rpmAxis
                visible: root.width >= 44 && root.traceMode !== PlcTypes.TempOnly
                min: 0.0
                max: 3000.0
                tickCount: 3
                color: HmiTheme.traceGrid
                labelColor: HmiTheme.textDim
                showGrid: false
            }

            ReferenceLine {
                value: root.maxTempLimit
                color: HmiTheme.traceLimit
                dashed: true
                dim: root.machineState !== PlcTypes.Error
            }

            LineSeries {
                name: "RPM"
                axisY: rpmAxis
                visible: root.traceMode !== PlcTypes.TempOnly
                color: HmiTheme.traceSpeed
                dim: root.traceMode === PlcTypes.AllTraces
                values: root.speedHistory
            }

            LineSeries {
                name: "TEMP"
                visible: root.traceMode !== PlcTypes.SpeedOnly
                color: root.temperature >= root.maxTempLimit ? HmiTheme.statusError : HmiTheme.traceTemp
                values: root.tempHistory
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.traceCycleRequested()
        }
    }
}
