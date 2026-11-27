pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import QmlTui.Charts

Terminal {
    id: rootWindow
    color: "black"

    // Waveform Properties
    property double waveTime: 0.0
    property double frequency: 2.0
    property double noiseLevel: 0.0
    property string waveType: "sine" // sine, square, triangle, sawtooth

    // Lissajous Properties
    property double lissajousPhase: 0.0
    property int freqA: 3
    property int freqB: 4

    // App state
    property bool paused: false
    property bool inspectEnabled: false
    property double waveSpeed: 0.08
    property int simulatedCpu: 4

    // Explicit key handling to quit, as per project rules
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || event.key === Qt.Key_Escape) {
            Qt.quit();
            event.accepted = true;
            return;
        }

        // Additional shortcut keys
        if (event.key === Qt.Key_Space) {
            rootWindow.paused = !rootWindow.paused;
            event.accepted = true;
        } else if (event.key === Qt.Key_I) {
            rootWindow.inspectEnabled = !rootWindow.inspectEnabled;
            event.accepted = true;
        }
    }

    // Mathematical calculations for the wave chart in data coordinates (x in [0, 1], y in [-1.4, 1.4])
    function generateWaveformPoints(time: real, freq: real, noise: real, type: string): var {
        const pts = [];
        const samples = 140;
        for (let i = 0; i <= samples; i++) {
            const x = i / samples;
            const angle = x * Math.PI * 2 * freq + time;
            let yVal = 0.0;

            if (type === "sine") {
                yVal = Math.sin(angle);
            } else if (type === "square") {
                yVal = Math.sin(angle) >= 0 ? 1.0 : -1.0;
            } else if (type === "triangle") {
                yVal = (Math.abs((angle % (Math.PI * 2)) - Math.PI) / Math.PI) * 2.0 - 1.0;
            } else if (type === "sawtooth") {
                yVal = ((angle % (Math.PI * 2)) / Math.PI) - 1.0;
            }

            if (noise > 0) {
                yVal += (Math.random() - 0.5) * noise;
            }

            pts.push({
                x: x,
                y: Math.max(-1.4, Math.min(1.4, yVal))
            });
        }
        return pts;
    }

    // Mathematical calculations for the Lissajous X-Y phase plot in unit coordinates
    function generateLissajousPoints(phase: real, fA: int, fB: int): var {
        const pts = [];
        const steps = 160;
        for (let i = 0; i < steps; i++) {
            const theta = (i / steps) * Math.PI * 2;
            pts.push({
                x: Math.sin(fA * theta + phase),
                y: Math.sin(fB * theta)
            });
        }
        return pts;
    }

    function cycleWaveType(): void {
        if (waveType === "sine")
            waveType = "square";
        else if (waveType === "square")
            waveType = "triangle";
        else if (waveType === "triangle")
            waveType = "sawtooth";
        else
            waveType = "sine";
    }

    function cycleNoise(): void {
        if (noiseLevel === 0.0)
            noiseLevel = 0.15;
        else if (noiseLevel === 0.15)
            noiseLevel = 0.35;
        else if (noiseLevel === 0.35)
            noiseLevel = 0.6;
        else
            noiseLevel = 0.0;
    }

    function updatePlots(): void {
        waveformSeries.points = rootWindow.generateWaveformPoints(rootWindow.waveTime, rootWindow.frequency, rootWindow.noiseLevel, rootWindow.waveType);
        lissajousSeries.points = rootWindow.generateLissajousPoints(rootWindow.lissajousPhase, rootWindow.freqA, rootWindow.freqB);
    }

    Component.onCompleted: rootWindow.updatePlots()

    // Animation Timer
    Timer {
        id: frameTimer
        interval: 40 // ~25 FPS
        running: true
        repeat: true
        onTriggered: {
            if (!rootWindow.paused) {
                rootWindow.waveTime += rootWindow.waveSpeed;
                rootWindow.lissajousPhase += 0.04;
                if (Math.random() < 0.1) {
                    rootWindow.simulatedCpu = Math.floor(Math.random() * 3) + 3;
                }
            }
            rootWindow.updatePlots();
        }
    }

    // Outer Layout Wrapper
    Rectangle {
        anchors.fill: parent
        color: "#111111"

        // 1. HEADER PANEL
        Rectangle {
            id: header
            height: 3
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            border.style: Border.Double
            border.color: "cyan"
            color: "#161b22"

            Text {
                anchors.centerIn: parent
                text: "📊 REAL-TIME VECTOR PLOTTING DASHBOARD"
                color: "yellow"
                bold: true
            }
        }

        // 2. FOOTER PANEL
        Rectangle {
            id: footer
            height: 3
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            border.style: Border.Single
            border.color: "cyan"
            color: "#0d1117"

            Text {
                anchors.centerIn: parent
                text: "Focus: [Tab] | Pause: [Space] | Inspect: [I] (" + (rootWindow.inspectEnabled ? "ON" : "OFF") + ") | Quit: [Q]/[Esc]"
                color: "gray"
            }
        }

        // 3. SIDEBAR CONTROLS & STATS (LEFT)
        Rectangle {
            id: sidebar
            width: 25
            clip: true
            anchors.top: header.bottom
            anchors.bottom: footer.top
            anchors.left: parent.left
            border.style: Border.Single
            border.color: "magenta"
            color: "#1e1e24"

            Column {
                anchors.fill: parent
                spacing: 0

                Text {
                    width: parent.width
                    text: "📊 CONTROLS"
                    color: "magenta"
                    bold: true
                    horizontalAlignment: Qt.AlignHCenter
                }

                // Play / Pause Toggle
                Button {
                    id: pauseBtn
                    focus: true
                    width: parent.width
                    height: 3
                    text: rootWindow.paused ? "▶ RESUME" : "⏸ PAUSE"
                    background: Rectangle {
                        color: "#21252b"
                        border.color: pauseBtn.activeFocus ? "cyan" : "gray"
                        border.style: pauseBtn.activeFocus ? Border.Double : Border.Single
                    }
                    onClicked: rootWindow.paused = !rootWindow.paused
                }

                // Waveform type selector
                Button {
                    id: waveBtn
                    width: parent.width
                    height: 3
                    text: "WAVE: " + rootWindow.waveType.toUpperCase()
                    background: Rectangle {
                        color: "#21252b"
                        border.color: waveBtn.activeFocus ? "cyan" : "gray"
                        border.style: waveBtn.activeFocus ? Border.Double : Border.Single
                    }
                    onClicked: rootWindow.cycleWaveType()
                }

                // Waveform frequency adjustments
                Row {
                    width: parent.width
                    height: 3
                    spacing: 1
                    Button {
                        id: freqMinusBtn
                        width: Math.floor((parent.width - 1) / 2)
                        height: 3
                        text: "FREQ -"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: freqMinusBtn.activeFocus ? "cyan" : "gray"
                            border.style: freqMinusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.frequency = Math.max(0.5, rootWindow.frequency - 0.5)
                    }
                    Button {
                        id: freqPlusBtn
                        width: parent.width - Math.floor((parent.width - 1) / 2) - 1
                        height: 3
                        text: "FREQ +"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: freqPlusBtn.activeFocus ? "cyan" : "gray"
                            border.style: freqPlusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.frequency = Math.min(10.0, rootWindow.frequency + 0.5)
                    }
                }

                // Noise toggles
                Button {
                    id: noiseBtn
                    width: parent.width
                    height: 3
                    text: "NOISE: " + Math.round(rootWindow.noiseLevel * 100) + "%"
                    background: Rectangle {
                        color: "#21252b"
                        border.color: noiseBtn.activeFocus ? "cyan" : "gray"
                        border.style: noiseBtn.activeFocus ? Border.Double : Border.Single
                    }
                    onClicked: rootWindow.cycleNoise()
                }

                Text {
                    width: parent.width
                    text: "🌀 LISSAJOUS"
                    color: "cyan"
                    bold: true
                    horizontalAlignment: Qt.AlignHCenter
                }

                // Lissajous Freq A adjustments
                Row {
                    width: parent.width
                    height: 3
                    spacing: 1
                    Button {
                        id: xFreqMinusBtn
                        width: Math.floor((parent.width - 1) / 2)
                        height: 3
                        text: "X: -"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: xFreqMinusBtn.activeFocus ? "cyan" : "gray"
                            border.style: xFreqMinusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.freqA = Math.max(1, rootWindow.freqA - 1)
                    }
                    Button {
                        id: xFreqPlusBtn
                        width: parent.width - Math.floor((parent.width - 1) / 2) - 1
                        height: 3
                        text: "X: +"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: xFreqPlusBtn.activeFocus ? "cyan" : "gray"
                            border.style: xFreqPlusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.freqA = Math.min(12, rootWindow.freqA + 1)
                    }
                }

                // Lissajous Freq B adjustments
                Row {
                    width: parent.width
                    height: 3
                    spacing: 1
                    Button {
                        id: yFreqMinusBtn
                        width: Math.floor((parent.width - 1) / 2)
                        height: 3
                        text: "Y: -"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: yFreqMinusBtn.activeFocus ? "cyan" : "gray"
                            border.style: yFreqMinusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.freqB = Math.max(1, rootWindow.freqB - 1)
                    }
                    Button {
                        id: yFreqPlusBtn
                        width: parent.width - Math.floor((parent.width - 1) / 2) - 1
                        height: 3
                        text: "Y: +"
                        background: Rectangle {
                            color: "#21252b"
                            border.color: yFreqPlusBtn.activeFocus ? "cyan" : "gray"
                            border.style: yFreqPlusBtn.activeFocus ? Border.Double : Border.Single
                        }
                        onClicked: rootWindow.freqB = Math.min(12, rootWindow.freqB + 1)
                    }
                }

                Text {
                    width: parent.width
                    text: "📈 STATS"
                    color: "green"
                    bold: true
                    horizontalAlignment: Qt.AlignHCenter
                }

                Text {
                    width: parent.width
                    text: " • Freq: " + rootWindow.frequency.toFixed(1) + " Hz"
                    color: "silver"
                }

                Text {
                    width: parent.width
                    text: " • Ratio: " + rootWindow.freqA + " : " + rootWindow.freqB
                    color: "silver"
                }

                Text {
                    width: parent.width
                    text: " • 25 FPS | CPU ~" + rootWindow.simulatedCpu + "%"
                    color: "silver"
                }
            }
        }

        // 4. MAIN PANEL (RIGHT CONTAINER)
        Rectangle {
            id: mainContent
            anchors.top: header.bottom
            anchors.bottom: footer.top
            anchors.left: sidebar.right
            anchors.right: parent.right
            color: "transparent"

            // Split into two charts stacked vertically
            Column {
                anchors.fill: parent
                spacing: 0

                // Upper Chart (Oscilloscope)
                Rectangle {
                    width: parent.width
                    height: Math.floor(parent.height / 2)
                    border.style: Border.Double
                    border.color: "green"
                    color: "#0a140a"

                    Text {
                        id: upperTitle
                        x: 2
                        y: 1
                        text: "OSCILLOSCOPE WAVEFORM VIEW"
                        color: "green"
                        bold: true
                    }

                    // Oscilloscope ChartView (user-composed crosshair inspection)
                    ChartView {
                        id: waveformChart

                        readonly property chartHit hit: (waveformHover.containsMouse && waveformChart.plotRevision >= 0) ? waveformChart.pick(waveformHover.x + waveformHover.mouseX, waveformHover.y + waveformHover.mouseY, ChartView.NearestX) : waveformChart.pick(-1, -1)

                        anchors {
                            top: upperTitle.bottom
                            bottom: parent.bottom
                            left: parent.left
                            right: parent.right
                        }
                        gridStyle: ChartView.Dots

                        axisX: ValueAxis {
                            id: waveAxisX
                            min: 0.0
                            max: 1.0
                            tickCount: 5
                            labelFormat: "%.2f"
                            suffix: "s"
                            color: "#006600"
                            labelColor: "#55aa55"
                            gridColor: "#004400"
                        }

                        axisY: ValueAxis {
                            id: waveAxisY
                            min: -1.4
                            max: 1.4
                            tickCount: 3
                            labelFormat: "%.1f"
                            color: "#006600"
                            labelColor: "#55aa55"
                            gridColor: "#004400"
                        }

                        ReferenceLine {
                            value: 0.0
                            color: "#006600"
                            dashed: true
                            dim: true
                        }

                        ReferenceLine {
                            orientation: Qt.Vertical
                            behindData: true
                            visible: waveformChart.hit.valid
                            value: waveformChart.hit.x
                            color: "#88ff88"
                            dashed: true
                            dim: true
                        }

                        LineSeries {
                            id: waveformSeries
                            name: rootWindow.waveType.toUpperCase()
                            color: "green"
                        }

                        Text {
                            visible: waveformChart.hit.valid
                            x: waveformChart.hit.cellX
                            y: waveformChart.hit.cellY
                            text: "●"
                            color: waveformChart.hit.color
                            bold: true
                        }

                        Text {
                            visible: waveformChart.hit.valid && waveformChart.plotRect.width >= 14
                            x: waveformChart.hit.cellX >= waveformChart.plotRect.x + waveformChart.plotRect.width / 2 ? waveformChart.plotRect.x : Math.max(waveformChart.plotRect.x, waveformChart.plotRect.x + waveformChart.plotRect.width - implicitWidth)
                            y: (waveformChart.hit.cellY <= waveformChart.plotRect.y + 1 && waveformChart.plotRect.height >= 3) ? (waveformChart.plotRect.y + waveformChart.plotRect.height - 1) : waveformChart.plotRect.y
                            text: waveformChart.hit.valid ? ("[x:" + waveAxisX.formatValue(waveformChart.hit.x) + "  " + waveformSeries.name + ":" + waveAxisY.formatValue(waveformChart.hit.y) + "]") : ""
                            color: "#88ff88"
                            dim: true
                        }

                        MouseArea {
                            id: waveformHover
                            x: waveformChart.plotRect.x
                            y: waveformChart.plotRect.y
                            width: waveformChart.plotRect.width
                            height: waveformChart.plotRect.height
                            hoverEnabled: rootWindow.inspectEnabled
                        }
                    }
                }

                // Lower Chart (Phase XY / Lissajous Orbit)
                Rectangle {
                    width: parent.width
                    height: parent.height - Math.floor(parent.height / 2)
                    border.style: Border.Double
                    border.color: "cyan"
                    color: "#0a141a"

                    Text {
                        id: lowerTitle
                        x: 2
                        y: 1
                        text: "XY PHASE ORBIT PLOT"
                        color: "cyan"
                    }

                    // Lissajous XY Phase Orbit ChartView (user-composed crosshair inspection)
                    ChartView {
                        id: lissajousChart

                        readonly property chartHit hit: (lissajousHover.containsMouse && lissajousChart.plotRevision >= 0) ? lissajousChart.pick(lissajousHover.x + lissajousHover.mouseX, lissajousHover.y + lissajousHover.mouseY, ChartView.NearestPoint) : lissajousChart.pick(-1, -1)

                        anchors {
                            top: lowerTitle.bottom
                            bottom: parent.bottom
                            left: parent.left
                            right: parent.right
                        }
                        gridStyle: ChartView.Dots

                        axisX: ValueAxis {
                            id: lissAxisX
                            min: -1.2
                            max: 1.2
                            tickCount: 3
                            labelFormat: "%.1f"
                            color: "#006666"
                            labelColor: "#55aaaa"
                            gridColor: "#004444"
                        }

                        axisY: ValueAxis {
                            id: lissAxisY
                            min: -1.2
                            max: 1.2
                            tickCount: 3
                            labelFormat: "%.1f"
                            color: "#006666"
                            labelColor: "#55aaaa"
                            gridColor: "#004444"
                        }

                        ReferenceLine {
                            value: 0.0
                            orientation: Qt.Horizontal
                            color: "#006666"
                            dashed: true
                            dim: true
                        }

                        ReferenceLine {
                            value: 0.0
                            orientation: Qt.Vertical
                            color: "#006666"
                            dashed: true
                            dim: true
                        }

                        ReferenceLine {
                            orientation: Qt.Vertical
                            behindData: true
                            visible: lissajousChart.hit.valid
                            value: lissajousChart.hit.x
                            color: "#88ffff"
                            dashed: true
                            dim: true
                        }

                        ReferenceLine {
                            orientation: Qt.Horizontal
                            behindData: true
                            visible: lissajousChart.hit.valid
                            value: lissajousChart.hit.y
                            color: "#88ffff"
                            dashed: true
                            dim: true
                        }

                        LineSeries {
                            id: lissajousSeries
                            name: "ORBIT"
                            color: "cyan"
                            closed: true
                        }

                        Text {
                            visible: lissajousChart.hit.valid
                            x: lissajousChart.hit.cellX
                            y: lissajousChart.hit.cellY
                            text: "●"
                            color: lissajousChart.hit.color
                            bold: true
                        }

                        Text {
                            visible: lissajousChart.hit.valid && lissajousChart.plotRect.width >= 14
                            x: lissajousChart.hit.cellX >= lissajousChart.plotRect.x + lissajousChart.plotRect.width / 2 ? lissajousChart.plotRect.x : Math.max(lissajousChart.plotRect.x, lissajousChart.plotRect.x + lissajousChart.plotRect.width - implicitWidth)
                            y: (lissajousChart.hit.cellY <= lissajousChart.plotRect.y + 1 && lissajousChart.plotRect.height >= 3) ? (lissajousChart.plotRect.y + lissajousChart.plotRect.height - 1) : lissajousChart.plotRect.y
                            text: lissajousChart.hit.valid ? ("[ORBIT (" + lissAxisX.formatValue(lissajousChart.hit.x) + ", " + lissAxisY.formatValue(lissajousChart.hit.y) + ")]") : ""
                            color: "#88ffff"
                            dim: true
                        }

                        MouseArea {
                            id: lissajousHover
                            x: lissajousChart.plotRect.x
                            y: lissajousChart.plotRect.y
                            width: lissajousChart.plotRect.width
                            height: lissajousChart.plotRect.height
                            hoverEnabled: rootWindow.inspectEnabled
                        }
                    }
                }
            }
        }
    }
}
