pragma ComponentBehavior: Bound
import QmlTui

// One-row footer listing the active Audio Player shortcuts. Narrow terminals get a compact variant.
Rectangle {
    id: bar

    required property AudioEngine engine
    property bool showMainDeck: true
    property bool showPlaylist: true
    property bool showVisualizer: true
    property int visMode: 0
    property bool blendOverlaps: false

    readonly property bool compact: width < 140
    readonly property bool tiny: width < 115

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
            visible: !bar.compact
            key: "ZXCVB"
            label: "Transport"
            activeColor: Theme.accent
        }
        KeyHint {
            key: "L"
            label: "Load"
            activeColor: Theme.titleBarGold
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
            activeColor: Theme.titleBarGold
        }
        KeyHint {
            key: "3"
            label: "Vis"
            active: bar.showVisualizer
            activeColor: Theme.scopeAccent
        }
        KeyHint {
            visible: bar.showVisualizer
            key: "W"
            label: ["Combo", "Spec", "Glow"][bar.visMode] || "Combo"
            active: bar.visMode !== 0
            activeColor: Theme.scopeAccent
        }
        KeyHint {
            visible: !bar.tiny
            key: "[ ]"
            label: "LP"
            active: bar.engine.scopeLowPassHz > 0
            activeColor: Theme.scopeAccent
        }
        KeyHint {
            visible: !bar.tiny
            key: "A"
            label: "Blend"
            active: bar.blendOverlaps
            activeColor: Theme.scopeRight
        }
        KeyHint {
            visible: !bar.tiny
            key: "K"
            label: "Skin"
            active: Theme.skinIndex !== 0
            activeColor: Theme.titleBarGold
        }
        KeyHint {
            key: "Q"
            label: "Quit"
            activeColor: Theme.accent
        }
    }
}
