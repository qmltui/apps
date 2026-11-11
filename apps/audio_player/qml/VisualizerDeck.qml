pragma ComponentBehavior: Bound
import QtQml
import QmlTui
import QmlTui.Controls

// Visualizer Deck:
// Modes (cycled with W or click):
//   0: SPECTRUM + SCOPE (24-Band Spectrum Analyzer + Braille Stereo Oscilloscope + VU)
//   1: 24-BAND SPECTRUM (Full-panel 24-Band FFT Analyzer with peak-hold caps)
//   2: PHOSPHOR SCOPE   (Fading CRT persistence traces + Stereo VU)
Panel {
    id: visDeck

    required property AudioEngine engine
    property int visMode: 0 // 0=Combo, 1=Spectrum, 2=Phosphor
    property bool blendOverlaps: false
    property bool showVu: true

    readonly property int bandCount: engine.bandCount
    readonly property bool showSpectrumSection: visMode === 0 || visMode === 1
    readonly property bool showCanvasSection: visMode !== 1
    readonly property string modeTitle: {
        switch (visMode) {
        case 1: return "24-BAND SPECTRUM";
        case 2: return "PHOSPHOR SCOPE";
        default: return "SPECTRUM + SCOPE";
        }
    }

    signal cycleModeRequested()

    function spectrumRow(row: int, rows: int, barWidth: int, gap: int, pad: int): string {
        const levels = visDeck.engine.bandLevels;
        const peaks = visDeck.engine.peakLevels;
        let line = " ".repeat(pad);
        const sep = " ".repeat(gap);
        for (let i = 0; i < visDeck.bandCount; ++i) {
            const lvl = (levels && i < levels.length) ? levels[i] : 0.0;
            const pk = (peaks && i < peaks.length) ? peaks[i] : 0.0;
            const fill = lvl * rows - row;
            let glyph;
            if (fill >= 1.0) {
                glyph = "█";
            } else if (fill > 0.0) {
                glyph = Theme.blockGlyphs[Math.round(fill * 8)];
            } else {
                const peakRow = Math.min(rows - 1, Math.floor(pk * rows));
                glyph = (peakRow === row && pk > 0) ? "▔" : " ";
            }
            line += glyph.repeat(barWidth);
            if (i < visDeck.bandCount - 1)
                line += sep;
        }
        return line;
    }

    function labelRow(barWidth: int, gap: int, pad: int, maxWidth: int): string {
        const labels = visDeck.engine.bandLabels;
        const pitch = barWidth + gap;
        const showAll = pitch >= 4;
        let line = " ".repeat(pad);
        for (let i = 0; i < visDeck.bandCount; ++i) {
            const label = (showAll || i % 2 === 0) ? labels[i] : "";
            line += label.padEnd(pitch, " ");
        }
        return line.substring(0, maxWidth);
    }

    color: Theme.panel
    border.style: Border.Single
    border.color: Theme.frame
    title: Text {
        text: "VISUALIZER // " + visDeck.modeTitle
              + (visDeck.visMode === 1 ? "" : " · LP " + visDeck.engine.scopeLowPassLabel)
        color: Theme.titleBarGold
        bold: true
        MouseArea {
            anchors.fill: parent
            onClicked: visDeck.cycleModeRequested()
        }
    }
    titleTrailing: Text {
        text: "[W: MODE]"
        color: Theme.ledGreen
        bold: true
        MouseArea {
            anchors.fill: parent
            onClicked: visDeck.cycleModeRequested()
        }
    }

    Column {
        id: visInner
        anchors.fill: parent
        spacing: 1

        readonly property int specHeight: !visDeck.showSpectrumSection ? 0
            : (visDeck.visMode === 1 ? visInner.height : Math.max(4, Math.floor(visInner.height * 0.48)))
        readonly property int scopeHeight: !visDeck.showCanvasSection ? 0
            : (visDeck.visMode === 0 ? Math.max(3, visInner.height - specHeight - 1) : visInner.height)

        // -------------------------------------------------------------------
        // 24-Band FFT Spectrum Analyzer Section
        // -------------------------------------------------------------------
        Item {
            id: specBox
            visible: visDeck.showSpectrumSection
            width: parent.width
            height: visInner.specHeight

            readonly property bool showLabels: height >= 5
            readonly property int barRows: Math.max(1, height - (showLabels ? 1 : 0))
            readonly property int gap: width >= visDeck.bandCount * 4 ? 1 : 0
            readonly property int barWidth: Math.max(1,
                Math.floor((width - (visDeck.bandCount - 1) * gap) / visDeck.bandCount))
            readonly property int usedWidth: visDeck.bandCount * barWidth + (visDeck.bandCount - 1) * gap
            readonly property int leftPad: Math.max(0, Math.floor((width - usedWidth) / 2))

            Column {
                anchors.top: parent.top
                anchors.left: parent.left
                width: parent.width
                height: specBox.barRows
                spacing: 0

                Repeater {
                    model: specBox.barRows

                    delegate: Text {
                        required property int index
                        readonly property int rowFromBottom: specBox.barRows - 1 - index
                        readonly property real heightRatio: (rowFromBottom + 1) / specBox.barRows

                        height: 1
                        bold: true
                        text: visDeck.spectrumRow(rowFromBottom, specBox.barRows,
                                                  specBox.barWidth, specBox.gap, specBox.leftPad)
                        color: heightRatio > 0.8 ? Theme.secondary
                             : heightRatio > 0.5 ? Theme.mid : Theme.accent
                    }
                }
            }

            Text {
                visible: specBox.showLabels
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                height: 1
                text: visDeck.labelRow(specBox.barWidth, specBox.gap, specBox.leftPad, specBox.width)
                color: Theme.dim
            }
        }

        // -------------------------------------------------------------------
        // Braille Scope + Stereo VU Section
        // -------------------------------------------------------------------
        Row {
            id: lowerScopeRow
            visible: visDeck.showCanvasSection
            width: parent.width
            height: visInner.scopeHeight
            spacing: 1

            readonly property bool vuVisible: visDeck.showVu && width >= 44
            readonly property int vuColWidth: vuVisible ? Math.min(20, Math.max(14, Math.floor(width * 0.30))) : 0

            // Compact Stereo VU Deck
            Item {
                id: vuBox
                visible: lowerScopeRow.vuVisible
                width: lowerScopeRow.vuColWidth
                height: parent.height

                Column {
                    width: parent.width
                    y: Math.max(0, Math.floor((parent.height - height) / 2))
                    spacing: parent.height >= 6 ? 1 : 0

                    Text {
                        visible: vuBox.height >= 5
                        text: "STEREO VU"
                        color: Theme.titleBarGold
                        bold: true
                    }
                    VuMeter {
                        label: "L"
                        level: visDeck.engine.vuLeft
                        accentColor: Theme.vuLow
                        labelColor: Theme.scopeAccent
                        meterWidth: vuBox.width
                    }
                    VuMeter {
                        label: "R"
                        level: visDeck.engine.vuRight
                        accentColor: Theme.vuLow
                        labelColor: Theme.scopeRight
                        meterWidth: vuBox.width
                    }
                }
            }

            // Braille Canvas (Stereo Waveform, Phosphor CRT Persistence)
            Canvas {
                id: visCanvas
                width: lowerScopeRow.vuVisible
                    ? Math.max(4, lowerScopeRow.width - vuBox.width - lowerScopeRow.spacing)
                    : lowerScopeRow.width
                height: parent.height
                renderMode: Canvas.RenderMode.Braille
                cellColorMode: visDeck.blendOverlaps ? Canvas.CellColorMode.Average : Canvas.CellColorMode.LastDot
                color: Theme.scopeAccent

                // Mode 0: Stereo Oscilloscope Traces
                WaveformTrace {
                    visible: visDeck.visMode === 0
                    samples: visDeck.engine.waveformRight
                    color: Theme.scopeRight
                }
                WaveformTrace {
                    visible: visDeck.visMode === 0
                    samples: visDeck.engine.waveformLeft
                    color: Theme.scopeAccent
                }

                // Mode 2: Phosphor CRT Persistence
                PersistenceTrace {
                    visible: visDeck.visMode === 2
                    samples: visDeck.engine.waveformRight
                    color: Theme.scopeRight
                    fadeColor: "ansi:black"
                }
                PersistenceTrace {
                    visible: visDeck.visMode === 2
                    samples: visDeck.engine.waveformLeft
                    color: Theme.scopeAccent
                    fadeColor: "ansi:black"
                }
            }
        }
    }
}
