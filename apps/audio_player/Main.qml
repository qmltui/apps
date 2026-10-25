pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Composition root for the Audio Player TUI: owns the AudioEngine, deck visibility toggles,
// keyboard bindings, and Split Docked Layout:
//   Left Column:  MainDeck (top) + PlaylistDeck (bottom)
//   Right Column: VisualizerDeck (full height)
Terminal {
    id: root

    property bool showMainDeck: true
    property bool showPlaylist: true
    property bool showVisualizer: true
    property bool blendOverlaps: false
    property int visMode: 0 // 0=Combo (Spectrum + Scope), 1=Spectrum, 2=Phosphor Scope

    readonly property bool hasLeftCol: showMainDeck || showPlaylist
    readonly property bool hasRightCol: showVisualizer
    readonly property bool anyDeckVisible: hasLeftCol || hasRightCol

    title: "AUDIO PLAYER // TERMINAL AUDIO PLAYER & VISUALIZER"
    color: Theme.background
    focus: true

    AudioEngine {
        id: engine
    }

    Component.onCompleted: {
        if (cliOptions.volume !== undefined)
            engine.volume = cliOptions.volume;
        if (cliOptions.inputs && cliOptions.inputs.length > 0 && !cliOptions.demo)
            engine.loadPlaylist(cliOptions.inputs);
        else if (cliOptions.demo)
            engine.useDemo();
    }

    Keys.onPressed: (event) => {
        if (addTrackDialog.visible)
            return;

        let handled = true;
        switch (event.key) {
        case Qt.Key_Escape:
            Qt.quit();
            break;
        case Qt.Key_Space:
            engine.togglePlayback();
            break;
        case Qt.Key_Left:
            engine.seekTrack(-5000);
            break;
        case Qt.Key_Right:
            engine.seekTrack(5000);
            break;
        case Qt.Key_Up:
            engine.moveSelection(-1);
            break;
        case Qt.Key_Down:
            engine.moveSelection(1);
            break;
        case Qt.Key_Return:
        case Qt.Key_Enter:
            engine.playSelectedTrack();
            break;
        case Qt.Key_Delete:
        case Qt.Key_Backspace:
            engine.removeSelectedTrack();
            break;
        default:
            switch (event.text ? event.text.toLowerCase() : "") {
            case "q": Qt.quit(); break;
            case " ":
            case "c": engine.togglePlayback(); break;
            case "x": engine.play(); break;
            case "v": engine.stop(); break;
            case "z":
            case "p": engine.previousTrack(); break;
            case "b":
            case "n": engine.nextTrack(); break;
            case "s": engine.toggleShuffle(); break;
            case "r": engine.toggleRepeat(); break;
            case "l": addTrackDialog.open(); break;
            case "t": mainDeck.timeRemaining = !mainDeck.timeRemaining; break;
            case "1": root.showMainDeck = !root.showMainDeck; break;
            case "2": root.showPlaylist = !root.showPlaylist; break;
            case "3": root.showVisualizer = !root.showVisualizer; break;
            case "w": root.visMode = (root.visMode + 1) % 3; break;
            case "a": root.blendOverlaps = !root.blendOverlaps; break;
            case "[": engine.stepScopeLowPass(-1); break;
            case "]": engine.stepScopeLowPass(1); break;
            case "+":
            case "=": engine.adjustVolume(0.05); break;
            case "-": engine.adjustVolume(-0.05); break;
            case "k": Theme.nextSkin(); break;
            default: handled = false;
            }
        }
        event.accepted = handled;
    }

    // ==========================================
    // DOCKED AUDIO PLAYER WORKSPACE
    // ==========================================
    Item {
        id: workspace
        anchors.top: parent.top
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right

        readonly property int leftColWidth: !root.hasLeftCol ? 0
            : (!root.hasRightCol ? width : Math.max(44, Math.min(width - 36, Math.floor(width * 0.48))))
        readonly property int rightColWidth: !root.hasRightCol ? 0
            : (width - leftColWidth)

        // Split Docked Columns (Left: Main + Playlist, Right: Visualizer)
        Row {
            visible: root.anyDeckVisible
            anchors.fill: parent
            spacing: 0

            // LEFT COLUMN: Main Player Deck + Playlist Editor
            Column {
                id: leftColumn
                visible: root.hasLeftCol
                width: workspace.leftColWidth
                height: parent.height
                spacing: 0

                readonly property int mainDeckHeight: !root.showMainDeck ? 0
                    : (!root.showPlaylist ? height : 8)
                readonly property int playlistHeight: !root.showPlaylist ? 0
                    : (height - mainDeckHeight)

                MainDeck {
                    id: mainDeck
                    visible: root.showMainDeck
                    width: parent.width
                    height: leftColumn.mainDeckHeight
                    engine: engine
                    showPlaylist: root.showPlaylist
                    showVisualizer: root.showVisualizer
                    onTogglePlaylistRequested: root.showPlaylist = !root.showPlaylist
                    onToggleVisualizerRequested: root.showVisualizer = !root.showVisualizer
                    onOpenLoadDialogRequested: addTrackDialog.open()
                }

                PlaylistDeck {
                    id: playlistDeck
                    visible: root.showPlaylist
                    width: parent.width
                    height: leftColumn.playlistHeight
                    engine: engine
                    onOpenLoadDialogRequested: addTrackDialog.open()
                }
            }

            // RIGHT COLUMN: Spectrum & Oscilloscope Visualizer
            VisualizerDeck {
                id: visualizerDeck
                visible: root.showVisualizer
                width: workspace.rightColWidth
                height: parent.height
                engine: engine
                visMode: root.visMode
                blendOverlaps: root.blendOverlaps
                onCycleModeRequested: root.visMode = (root.visMode + 1) % 3
            }
        }

        Text {
            visible: !root.anyDeckVisible
            anchors.centerIn: parent
            text: "All decks hidden · 1 main deck · 2 playlist · 3 visualizer"
            color: Theme.muted
        }
    }

    // ==========================================
    // FOOTER KEYMAP BAR & MODAL DIALOG
    // ==========================================
    KeyMapBar {
        id: footer
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        engine: engine
        showMainDeck: root.showMainDeck
        showPlaylist: root.showPlaylist
        showVisualizer: root.showVisualizer
        blendOverlaps: root.blendOverlaps
        visMode: root.visMode
    }

    AddTrackDialog {
        id: addTrackDialog
        engine: engine
        onClosedAndRestoreFocus: root.focus = true
    }
}
