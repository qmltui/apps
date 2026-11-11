pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Dedicated Braille Oscilloscope panel inside the single-hairline divided grid.
// W cycles the visualization mode:
//   0 = Stereo Scope (L/R time-domain waveforms)
//   1 = Phosphor Scope (CRT persistence trails)
//   2 = Radial Scope (Circular / Radial bass-reactive oscilloscope)
Item {
    id: visDeck

    required property AudioEngine engine
    property int visMode: 0 // 0=Stereo Scope, 1=Phosphor Scope, 2=Radial Scope
    property bool blendOverlaps: false

    readonly property string modeTitle: {
        switch (visMode) {
        case 1: return "PHOSPHOR SCOPE";
        case 2: return "RADIAL SCOPE";
        default: return "STEREO SCOPE";
        }
    }

    signal cycleModeRequested()

    Column {
        id: visInner
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // Header Row (click cycles mode; no redundant shortcut label)
        Text {
            id: visHeader
            width: parent.width
            height: 1
            text: Theme.fitText("VISUALIZER // " + visDeck.modeTitle, width)
            color: Theme.header
            bold: true
            MouseArea {
                anchors.fill: parent
                onClicked: visDeck.cycleModeRequested()
            }
        }

        // Full-Panel Braille Oscilloscope Canvas
        Canvas {
            id: visCanvas
            width: parent.width
            height: Math.max(2, visInner.height - 2)
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

            // Mode 1: Phosphor CRT Persistence (ANSI palette shades)
            PersistenceTrace {
                visible: visDeck.visMode === 1
                samples: visDeck.engine.waveformRight
                color: Theme.scopeRight
                fadeColor: "ansi:brightblack"
            }
            PersistenceTrace {
                visible: visDeck.visMode === 1
                samples: visDeck.engine.waveformLeft
                color: Theme.scopeAccent
                fadeColor: "ansi:brightblack"
            }

            // Mode 2: Circular / Radial Oscilloscope (512 samples, bass-reactive R_base,
            // 16-sample seam blending, and glowing stroke)
            RadialScopeTrace {
                visible: visDeck.visMode === 2
                samples: visDeck.engine.waveformMono
                bassEnergy: visDeck.engine.bassEnergy
                color: Theme.scopeAccent
                fadeColor: "ansi:brightblack"
            }
        }
    }
}
