pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property int machineState: PlcTypes.Idle
    property int traceMode: PlcTypes.AllTraces
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property var tempHistory: []
    property var speedHistory: []

    readonly property int innerWidth: Math.max(10, width - 4)
    readonly property int innerHeight: Math.max(4, height - 2)

    signal traceCycleRequested

    onTempHistoryChanged: {
        if (root.visible)
            tempCanvas.updatePlot();
    }
    onSpeedHistoryChanged: {
        if (root.visible)
            tempCanvas.updatePlot();
    }
    onMaxTempLimitChanged: tempCanvas.updatePlot()
    onTraceModeChanged: tempCanvas.updatePlot()
    onMachineStateChanged: tempCanvas.updatePlot()
    onVisibleChanged: {
        if (root.visible)
            tempCanvas.updatePlot();
    }

    function buildSeriesPoints(series: var, minVal: real, maxVal: real, canvasWidth: int, canvasHeight: int): var {
        if (!series || series.length < 2 || canvasWidth <= 0 || canvasHeight <= 0)
            return [];
        const w = canvasWidth * 2;
        const h = canvasHeight * 4;
        const len = series.length;
        const range = Math.max(0.001, maxVal - minVal);
        const pts = [];

        for (let i = 0; i < len; ++i) {
            const cx = Math.round((i / (len - 1)) * (w - 1));
            let cy = Math.round((h - 1) - ((series[i] - minVal) / range) * (h - 1));
            cy = Math.max(0, Math.min(h - 1, cy));
            pts.push({
                x: cx,
                y: cy
            });
        }
        return pts;
    }

    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

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

    Row {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        Column {
            width: 4
            height: root.innerHeight

            Text {
                text: "100°"
                color: HmiTheme.textDim
            }

            Item {
                width: 4
                height: Math.max(0, root.innerHeight - 2)

                Text {
                    anchors.centerIn: parent
                    text: " 60°"
                    color: HmiTheme.textDim
                }
            }

            Text {
                text: " 20°"
                color: HmiTheme.textDim
            }
        }

        Rectangle {
            width: Math.max(6, root.innerWidth - 5)
            height: root.innerHeight
            color: HmiTheme.recessedBg

            Canvas {
                id: tempCanvas
                anchors.fill: parent
                cellColorMode: Canvas.LastDot

                onWidthChanged: updatePlot()
                onHeightChanged: updatePlot()

                function updatePlot() {
                    if (width <= 0 || height <= 0)
                        return;

                    const w2 = Math.max(1, width * 2 - 1);
                    const h4 = Math.max(1, height * 4 - 1);

                    gridUpper.x2 = w2;
                    gridUpper.y1 = Math.round(h4 * 0.25);
                    gridUpper.y2 = gridUpper.y1;

                    gridMid.x2 = w2;
                    gridMid.y1 = Math.round(h4 * 0.50);
                    gridMid.y2 = gridMid.y1;

                    gridLower.x2 = w2;
                    gridLower.y1 = Math.round(h4 * 0.75);
                    gridLower.y2 = gridLower.y1;

                    const ly = Math.max(0, Math.min(h4, Math.round(h4 - ((root.maxTempLimit - 20.0) / 80.0) * h4)));
                    limitLine.y1 = ly;
                    limitLine.y2 = ly;
                    limitLine.x2 = w2;

                    speedPoly.points = root.traceMode !== PlcTypes.TempOnly ? root.buildSeriesPoints(root.speedHistory, 0.0, 3000.0, width, height) : [];
                    tempPoly.points = root.traceMode !== PlcTypes.SpeedOnly ? root.buildSeriesPoints(root.tempHistory, 20.0, 100.0, width, height) : [];
                }

                Line {
                    id: gridUpper
                    x1: 0
                    y1: 0
                    x2: 10
                    y2: 0
                    color: HmiTheme.traceGrid
                    dim: true
                }

                Line {
                    id: gridMid
                    x1: 0
                    y1: 0
                    x2: 10
                    y2: 0
                    color: HmiTheme.traceGrid
                    dim: true
                }

                Line {
                    id: gridLower
                    x1: 0
                    y1: 0
                    x2: 10
                    y2: 0
                    color: HmiTheme.traceGrid
                    dim: true
                }

                Line {
                    id: limitLine
                    x1: 0
                    y1: 0
                    x2: 10
                    y2: 0
                    color: HmiTheme.traceLimit
                    dim: root.machineState !== PlcTypes.Error
                }

                Polygon {
                    id: speedPoly
                    closed: false
                    color: HmiTheme.traceSpeed
                    dim: root.traceMode === PlcTypes.AllTraces
                }

                Polygon {
                    id: tempPoly
                    closed: false
                    color: root.temperature >= root.maxTempLimit ? HmiTheme.statusError : HmiTheme.traceTemp
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.traceCycleRequested()
            }
        }
    }
}
