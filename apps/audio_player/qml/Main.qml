pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Composition root for the Audio Player TUI: owns the AudioEngine, view toggles,
// keyboard bindings, and Hybrid Winamp Workspace:
//   Left Column:  Classic 8-Row Winamp MainDeck (top) + PlaylistDeck (bottom)
//   Right Column: SpectrumDeck (top) ─── VisualizerDeck (bottom)
Terminal {
    id: root

    property bool showMainDeck: true
    property bool showPlaylist: true
    property bool showSpectrum: true
    property bool showVisualizer: true
    property bool blendOverlaps: false
    property int visMode: 0 // 0=Stereo Scope, 1=Phosphor Scope, 2=Radial Scope

    readonly property bool hasLeftColumn: showMainDeck || showPlaylist
    readonly property bool hasRightColumn: showSpectrum || showVisualizer
    readonly property bool bothColumnsVisible: hasLeftColumn && hasRightColumn
    readonly property bool bothRightVisible: showSpectrum && showVisualizer
    readonly property bool anyDeckVisible: hasLeftColumn || hasRightColumn

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
        case Qt.Key_Left: {
            const nextPos = audioEngine.displayPosition - 5000;
            audioEngine.seekTrack(-5000);
            mainDeck.flashSeek(nextPos);
            break;
        }
        case Qt.Key_Right: {
            const nextPos = audioEngine.displayPosition + 5000;
            audioEngine.seekTrack(5000);
            mainDeck.flashSeek(nextPos);
            break;
        }
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
            case "s":
                audioEngine.toggleShuffle();
                mainDeck.flashAction(audioEngine.shuffle ? "SHUFFLE: ON" : "SHUFFLE: OFF");
                break;
            case "r":
                audioEngine.toggleRepeat();
                mainDeck.flashAction(audioEngine.repeat ? "REPEAT: ON" : "REPEAT: OFF");
                break;
            case "l": addTrackDialog.open(); break;
            case "t":
                mainDeck.timeRemaining = !mainDeck.timeRemaining;
                mainDeck.flashAction(mainDeck.timeRemaining ? "TIME DISPLAY: REMAINING" : "TIME DISPLAY: ELAPSED");
                break;
            case "1": root.showMainDeck = !root.showMainDeck; break;
            case "2": root.showPlaylist = !root.showPlaylist; break;
            case "3": root.showSpectrum = !root.showSpectrum; break;
            case "4": root.showVisualizer = !root.showVisualizer; break;
            case "w": root.visMode = (root.visMode + 1) % 3; break;
            case "a": root.blendOverlaps = !root.blendOverlaps; break;
            case "k":
                Theme.nextSkin();
                mainDeck.flashAction("SKIN: " + Theme.skinName);
                break;
            case "+":
            case "=":
                audioEngine.adjustVolume(0.05);
                mainDeck.flashVolume();
                break;
            case "-":
                audioEngine.adjustVolume(-0.05);
                mainDeck.flashVolume();
                break;
            case ",":
            case "<":
                audioEngine.adjustBalance(-0.1);
                mainDeck.flashBalance();
                break;
            case ".":
            case ">":
                audioEngine.adjustBalance(0.1);
                mainDeck.flashBalance();
                break;
            case "[":
                audioEngine.stepScopeLowPass(-1);
                mainDeck.flashAction("SCOPE LPF: " + audioEngine.scopeLowPassLabel);
                break;
            case "]":
                audioEngine.stepScopeLowPass(1);
                mainDeck.flashAction("SCOPE LPF: " + audioEngine.scopeLowPassLabel);
                break;
            default: handled = false;
            }
        }
        event.accepted = handled;
    }

    // ==========================================
    // HYBRID WINAMP STACK + HAIRLINE VISUALIZERS
    // ==========================================
    Item {
        id: workspace
        anchors.top: parent.top
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right

        readonly property int leftPaneWidth: !root.hasLeftColumn ? 0
            : (!root.hasRightColumn ? Math.min(width, 60)
                                    : Math.max(44, Math.min(58, Math.min(width - 36, Math.floor((width - 1) * 0.49)))))
        readonly property int rightPaneWidth: !root.hasRightColumn ? 0
            : (width - leftPaneWidth - (root.bothColumnsVisible ? 1 : 0))
        readonly property int specPaneHeight: !root.showSpectrum ? 0
            : (!root.showVisualizer ? height : Math.min(12, Math.max(6, Math.floor((height - 1) * 0.36))))
        readonly property int visPaneHeight: !root.showVisualizer ? 0
            : (height - specPaneHeight - (root.bothRightVisible ? 1 : 0))

        Row {
            visible: root.anyDeckVisible
            x: root.hasRightColumn ? 0 : Math.max(0, Math.floor((workspace.width - workspace.leftPaneWidth) / 2))
            width: root.hasRightColumn ? workspace.width : workspace.leftPaneWidth
            height: workspace.height
            spacing: 0

            // LEFT COLUMN: Classic 8-Row Winamp Head Unit + Playlist Pane
            Item {
                id: leftColumn
                visible: root.hasLeftColumn
                width: workspace.leftPaneWidth
                height: parent.height

                readonly property int deckHeight: root.showMainDeck ? Math.min(8, height) : 0
                readonly property int playlistHeight: root.showPlaylist ? Math.max(0, height - deckHeight) : 0
                readonly property int deckInset: root.bothColumnsVisible ? 1 : 0

                MainDeck {
                    id: mainDeck
                    visible: root.showMainDeck
                    x: leftColumn.deckInset
                    y: root.showPlaylist ? 0 : Math.max(0, Math.floor((leftColumn.height - leftColumn.deckHeight) / 2))
                    width: Math.max(0, leftColumn.width - leftColumn.deckInset * 2)
                    height: leftColumn.deckHeight
                    engine: audioEngine
                    showPlaylist: root.showPlaylist
                    showSpectrum: root.showSpectrum
                    showVisualizer: root.showVisualizer
                    onTogglePlaylistRequested: root.showPlaylist = !root.showPlaylist
                    onToggleSpectrumRequested: root.showSpectrum = !root.showSpectrum
                    onToggleVisualizerRequested: root.showVisualizer = !root.showVisualizer
                    onOpenLoadDialogRequested: addTrackDialog.open()
                }

                PlaylistDeck {
                    id: playlistDeck
                    visible: root.showPlaylist
                    x: 0
                    y: leftColumn.deckHeight
                    width: leftColumn.width
                    height: leftColumn.playlistHeight
                    engine: audioEngine
                    onOpenLoadDialogRequested: addTrackDialog.open()
                }
            }

            // Single dimmed vertical hairline divider (with ├ junction when right column is split)
            Text {
                visible: root.bothColumnsVisible
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

            // RIGHT COLUMN: 24-Band FFT Spectrum + Braille Oscilloscope
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
