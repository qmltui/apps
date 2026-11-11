pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Composition root for the Audio Player TUI: owns the AudioEngine, view toggles,
// keyboard bindings, and a single-hairline divided grid layout:
//   Left Column:  PlaylistDeck
//   Right Column: SpectrumDeck (top) ─── VisualizerDeck (bottom)
//   Bottom Bar:   MainDeck (full-width Now Playing + Seek + Transport)
Terminal {
    id: root

    property bool showMainDeck: true
    property bool showPlaylist: true
    property bool showSpectrum: true
    property bool showVisualizer: true
    property bool blendOverlaps: false
    property int visMode: 0 // 0=Stereo Scope, 1=Phosphor Scope, 2=Radial Scope

    readonly property bool hasRightColumn: showSpectrum || showVisualizer
    readonly property bool bothRightVisible: showSpectrum && showVisualizer
    readonly property bool hasUpperArea: showPlaylist || hasRightColumn
    readonly property bool bothUpperColumnsVisible: showPlaylist && hasRightColumn
    readonly property bool anyDeckVisible: showMainDeck || hasUpperArea

    title: "AUDIO PLAYER // TERMINAL AUDIO PLAYER & VISUALIZER"
    color: Theme.background
    focus: true

    AudioEngine {
        id: audioEngine
    }

    Component.onCompleted: {
        if (cliOptions.volume !== undefined)
            audioEngine.volume = cliOptions.volume;
        if (cliOptions.inputs && cliOptions.inputs.length > 0 && !cliOptions.demo)
            audioEngine.loadPlaylist(cliOptions.inputs);
        else if (cliOptions.demo)
            audioEngine.useDemo();
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
            audioEngine.togglePlayback();
            break;
        case Qt.Key_Left:
            audioEngine.seekTrack(-5000);
            break;
        case Qt.Key_Right:
            audioEngine.seekTrack(5000);
            break;
        case Qt.Key_Up:
            audioEngine.moveSelection(-1);
            break;
        case Qt.Key_Down:
            audioEngine.moveSelection(1);
            break;
        case Qt.Key_Return:
        case Qt.Key_Enter:
            audioEngine.playSelectedTrack();
            break;
        case Qt.Key_Delete:
        case Qt.Key_Backspace:
            audioEngine.removeSelectedTrack();
            break;
        default:
            switch (event.text ? event.text.toLowerCase() : "") {
            case "q": Qt.quit(); break;
            case " ":
            case "c": audioEngine.togglePlayback(); break;
            case "x": audioEngine.play(); break;
            case "v": audioEngine.stop(); break;
            case "z":
            case "p": audioEngine.previousTrack(); break;
            case "b":
            case "n": audioEngine.nextTrack(); break;
            case "s": audioEngine.toggleShuffle(); break;
            case "r": audioEngine.toggleRepeat(); break;
            case "l": addTrackDialog.open(); break;
            case "t": mainDeck.timeRemaining = !mainDeck.timeRemaining; break;
            case "1": root.showMainDeck = !root.showMainDeck; break;
            case "2": root.showPlaylist = !root.showPlaylist; break;
            case "3": root.showSpectrum = !root.showSpectrum; break;
            case "4": root.showVisualizer = !root.showVisualizer; break;
            case "w": root.visMode = (root.visMode + 1) % 3; break;
            case "a": root.blendOverlaps = !root.blendOverlaps; break;
            case "+":
            case "=": audioEngine.adjustVolume(0.05); break;
            case "-": audioEngine.adjustVolume(-0.05); break;
            default: handled = false;
            }
        }
        event.accepted = handled;
    }

    // ==========================================
    // SINGLE-HAIRLINE DIVIDED WORKSPACE
    // ==========================================
    Item {
        id: workspace
        anchors.top: parent.top
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right

        readonly property int deckRows: root.showMainDeck ? 2 : 0
        readonly property int upperHeight: root.hasUpperArea
            ? Math.max(4, height - deckRows - 1)
            : 0
        readonly property int leftPaneWidth: !root.showPlaylist ? 0
            : (!root.hasRightColumn ? width : Math.max(38, Math.min(width - 44, Math.floor((width - 1) * 0.39))))
        readonly property int rightPaneWidth: !root.hasRightColumn ? 0
            : (width - leftPaneWidth - (root.bothUpperColumnsVisible ? 1 : 0))
        readonly property int specPaneHeight: !root.showSpectrum ? 0
            : (!root.showVisualizer ? upperHeight : Math.min(12, Math.max(6, Math.floor((upperHeight - 1) * 0.38))))
        readonly property int visPaneHeight: !root.showVisualizer ? 0
            : (upperHeight - specPaneHeight - (root.bothRightVisible ? 1 : 0))

        Column {
            visible: root.anyDeckVisible
            width: workspace.width
            y: root.hasUpperArea ? 0 : Math.max(0, Math.floor((workspace.height - 4) / 2))
            spacing: 0

            // UPPER AREA: PlaylistDeck │ (SpectrumDeck ─── VisualizerDeck)
            Row {
                visible: root.hasUpperArea
                width: workspace.width
                height: workspace.upperHeight
                spacing: 0

                PlaylistDeck {
                    id: playlistDeck
                    visible: root.showPlaylist
                    width: workspace.leftPaneWidth
                    height: parent.height
                    engine: audioEngine
                    onOpenLoadDialogRequested: addTrackDialog.open()
                }

                // Single dimmed vertical hairline divider (with ├ junction when right column is split)
                Text {
                    visible: root.bothUpperColumnsVisible
                    width: 1
                    height: parent.height
                    color: Theme.border
                    dim: true
                    text: root.bothRightVisible
                        ? (("│\n").repeat(Math.max(1, workspace.specPaneHeight))
                           + "├\n"
                           + ("│\n").repeat(Math.max(1, workspace.visPaneHeight)))
                        : ("│\n").repeat(Math.max(1, parent.height))
                }

                Column {
                    visible: root.hasRightColumn
                    width: workspace.rightPaneWidth
                    height: parent.height
                    spacing: 0

                    SpectrumDeck {
                        id: spectrumDeck
                        visible: root.showSpectrum
                        width: parent.width
                        height: workspace.specPaneHeight
                        engine: audioEngine
                    }

                    // Single dimmed horizontal hairline separating Spectrum and Oscilloscope
                    Text {
                        visible: root.bothRightVisible
                        width: parent.width
                        height: 1
                        color: Theme.border
                        dim: true
                        text: "─".repeat(Math.max(0, parent.width))
                    }

                    VisualizerDeck {
                        id: visualizerDeck
                        visible: root.showVisualizer
                        width: parent.width
                        height: workspace.visPaneHeight
                        engine: audioEngine
                        visMode: root.visMode
                        blendOverlaps: root.blendOverlaps
                        onCycleModeRequested: root.visMode = (root.visMode + 1) % 3
                    }
                }
            }

            // Single dimmed horizontal hairline (with ┴ junction when split above)
            Text {
                width: workspace.width
                height: 1
                color: Theme.border
                dim: true
                text: (root.hasUpperArea && root.bothUpperColumnsVisible)
                    ? ("─".repeat(workspace.leftPaneWidth) + "┴" + "─".repeat(Math.max(0, workspace.rightPaneWidth)))
                    : "─".repeat(Math.max(0, workspace.width))
            }

            // FULL-WIDTH BOTTOM PLAYER BAR
            MainDeck {
                id: mainDeck
                visible: root.showMainDeck
                width: workspace.width
                height: 2
                engine: audioEngine
            }

            // Bottom framing hairline when operating in standalone Mini-Player mode (!hasUpperArea)
            Text {
                visible: !root.hasUpperArea && root.showMainDeck
                width: workspace.width
                height: 1
                color: Theme.border
                dim: true
                text: "─".repeat(Math.max(0, workspace.width))
            }
        }

        Text {
            visible: !root.anyDeckVisible
            anchors.centerIn: parent
            text: "All decks hidden · 1 deck · 2 playlist · 3 spectrum · 4 scope"
            color: Theme.text
            dim: true
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
        engine: audioEngine
        showMainDeck: root.showMainDeck
        showPlaylist: root.showPlaylist
        showSpectrum: root.showSpectrum
        showVisualizer: root.showVisualizer
        blendOverlaps: root.blendOverlaps
        visMode: root.visMode
    }

    AddTrackDialog {
        id: addTrackDialog
        engine: audioEngine
        onClosedAndRestoreFocus: root.focus = true
    }
}
