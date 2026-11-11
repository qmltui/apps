pragma ComponentBehavior: Bound
import QmlTui

// One-row footer listing the non-redundant Audio Player shortcuts.
Rectangle {
    id: bar

    required property AudioEngine engine
    property bool showMainDeck: true
    property bool showPlaylist: true
    property bool showSpectrum: true
    property bool showVisualizer: true
    property int visMode: 0
    property bool blendOverlaps: false

    readonly property bool compact: width < 100
    readonly property bool tiny: width < 80

    height: 1
    color: Theme.barBackground

    Row {
        anchors.centerIn: parent
        spacing: bar.compact ? 1 : 2

        KeyHint {
            key: "SPC"
            label: bar.engine.playing ? "Pause" : "Play"
            active: !bar.engine.playing
            activeColor: Theme.warning
        }
        KeyHint {
            key: "L"
            label: "Load"
            activeColor: Theme.header
        }
        KeyHint {
            key: "1"
            label: "Deck"
            active: bar.showMainDeck
            activeColor: Theme.accent
        }
        KeyHint {
            key: "2"
            label: "PL"
            active: bar.showPlaylist
            activeColor: Theme.header
        }
        KeyHint {
            key: "3"
            label: "Spec"
            active: bar.showSpectrum
            activeColor: Theme.accent
        }
        KeyHint {
            key: "4"
            label: "Scope"
            active: bar.showVisualizer
            activeColor: Theme.scopeAccent
        }
        KeyHint {
            visible: bar.showVisualizer
            key: "W"
            label: ["Stereo", "Glow", "Radial"][bar.visMode] || "Stereo"
            active: bar.visMode !== 0
            activeColor: Theme.scopeAccent
        }
        KeyHint {
            visible: !bar.tiny && bar.showVisualizer
            key: "A"
            label: "Blend"
            active: bar.blendOverlaps
            activeColor: Theme.scopeRight
        }
        KeyHint {
            key: "Q"
            label: "Quit"
            activeColor: Theme.accent
        }
    }
}
