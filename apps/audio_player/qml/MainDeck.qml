pragma ComponentBehavior: Bound
import QtQml
import QmlTui
import QmlTui.Controls

// Main Player Window (Deck):
// Status LED + LCD clock + Mini-Visualizer, scrolling LED track marquee,
// kbps / kHz / MONO / STEREO indicators, Volume & Balance controls,
// interactive Seek bar, Transport buttons (Z X C V B L), SHUF/REP, and [PL][VIS] toggles.
Panel {
    id: deck

    required property AudioEngine engine
    property bool showPlaylist: true
    property bool showVisualizer: true
    property bool timeRemaining: false
    property int miniVisMode: 0 // 0 = mini spectrum, 1 = mini scope, 2 = off
    property int marqueeOffset: 0

    readonly property bool fileSource: engine.source === AudioEngine.File
    readonly property bool demoSource: engine.source === AudioEngine.Demo
    readonly property string playStateLabel: engine.playing ? "▶ PLAYING"
                                           : engine.stopped ? "■ STOPPED" : "⏸ PAUSED"
    readonly property string playStateColor: engine.playing ? Theme.playing
                                           : engine.stopped ? Theme.subtle : Theme.warning
    readonly property string clockText: {
        const pos = engine.displayPosition;
        const dur = engine.displayDuration;
        if (timeRemaining && dur > 0)
            return "-" + engine.formatTime(Math.max(0, dur - pos));
        return " " + engine.formatTime(pos);
    }

    signal togglePlaylistRequested()
    signal toggleVisualizerRequested()
    signal openLoadDialogRequested()

    function miniSpectrumString(cols: int): string {
        if (cols <= 0)
            return "";
        const levels = deck.engine.bandLevels;
        const count = levels ? levels.length : 0;
        if (count === 0)
            return "▁".repeat(cols);
        let out = "";
        for (let i = 0; i < cols; ++i) {
            const bandIdx = Math.min(count - 1, Math.floor((i * count) / cols));
            const lvl = levels[bandIdx] || 0.0;
            const gIdx = Math.max(1, Math.min(8, Math.round(lvl * 8)));
            out += deck.engine.playing ? Theme.blockGlyphs[gIdx] : "▁";
        }
        return out;
    }

    function miniScopeString(cols: int): string {
        if (cols <= 0)
            return "";
        const wave = deck.engine.waveformMono;
        const count = wave ? wave.length : 0;
        if (count === 0 || !deck.engine.playing)
            return "─".repeat(cols);
        const glyphs = ["▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"];
        let out = "";
        for (let i = 0; i < cols; ++i) {
            const idx = Math.min(count - 1, Math.floor((i * count) / cols));
            const sample = (wave[idx] || 0.0) * 0.5 + 0.5;
            const g = Math.max(0, Math.min(7, Math.floor(sample * 8)));
            out += glyphs[g];
        }
        return out;
    }

    color: Theme.panel
    border.style: Border.Double
    border.color: Theme.activeFrame
    titleAlignment: Qt.AlignLeft
    title: Row {
        spacing: 1
        Text {
            text: "⚡ AUDIO PLAYER"
            color: Theme.titleBarGold
            bold: true
        }
        Text {
            text: " " + deck.engine.sourceName + " "
            reversed: true
            bold: true
            color: deck.fileSource ? Theme.fileSource
                 : deck.demoSource ? Theme.ledGreen : Theme.muted
        }
        Text {
            text: deck.playStateLabel
            color: deck.playStateColor
            bold: true
        }
    }
    titleTrailing: Row {
        spacing: 1
        Text {
            visible: deck.demoSource
            text: deck.engine.beatPulse > 0.5 ? "● BEAT" : "○ beat"
            color: deck.engine.beatPulse > 0.5 ? Theme.secondary : Theme.dim
            bold: deck.engine.beatPulse > 0.5
        }
        Text {
            text: "[" + Theme.skinName + "]"
            color: Theme.subtle
            MouseArea {
                anchors.fill: parent
                onClicked: Theme.nextSkin()
            }
        }
    }

    Timer {
        interval: 220
        running: deck.visible && deck.engine.playing
        repeat: true
        onTriggered: deck.marqueeOffset = (deck.marqueeOffset + 1) % 4096
    }

    Column {
        id: innerCol
        anchors.fill: parent
        spacing: 0

        // -------------------------------------------------------------------
        // TOP DECK AREA: Left LCD (Clock + Mini-Vis) | Right Marquee + Sliders
        // -------------------------------------------------------------------
        Row {
            id: topDeckRow
            width: parent.width
            height: 4
            spacing: 1

            // Left LCD Window (Clock + Mini-Spectrum)
            Rectangle {
                id: lcdBox
                width: Math.min(22, Math.max(14, Math.floor(topDeckRow.width * 0.34)))
                height: 4
                color: Theme.lcdBackground
                border.style: Border.Single
                border.color: Theme.frame

                MouseArea {
                    anchors.fill: parent
                    onClicked: (event) => {
                        if (event.y <= 1)
                            deck.timeRemaining = !deck.timeRemaining;
                        else
                            deck.miniVisMode = (deck.miniVisMode + 1) % 3;
                    }
                }

                // Row 1 inside LCD: Clock Label + Digital Clock
                Row {
                    x: 1
                    y: 1
                    spacing: 1

                    Text {
                        text: deck.timeRemaining ? "REM  " : "TIME "
                        color: Theme.dim
                    }
                    Text {
                        text: deck.clockText
                        color: Theme.ledGreen
                        bold: true
                    }
                }

                // Row 2 inside LCD: Mini-Visualizer (Spectrum / Scope / Off)
                Text {
                    x: 1
                    y: 2
                    width: Math.max(0, lcdBox.width - 2)
                    text: deck.miniVisMode === 0 ? deck.miniSpectrumString(Math.max(0, lcdBox.width - 2))
                        : deck.miniVisMode === 1 ? deck.miniScopeString(Math.max(0, lcdBox.width - 2))
                        : Theme.fitText(deck.engine.statusText, Math.max(0, lcdBox.width - 2))
                    color: deck.miniVisMode === 1 ? Theme.scopeAccent : Theme.ledGreen
                    bold: true
                }
            }

            // Right Stack: Marquee Ticker, Format/Stereo/Deck Toggles, Volume & Balance
            Column {
                id: rightInfoCol
                width: Math.max(10, topDeckRow.width - lcdBox.width - topDeckRow.spacing)
                height: 4
                spacing: 0

                // Row 0: Scrolling Green LED Marquee Box
                Rectangle {
                    id: marqueeBox
                    width: parent.width
                    height: 1
                    color: Theme.lcdBackground

                    readonly property bool hasError: deck.engine.errorText.length > 0
                    readonly property string bannerText: hasError
                        ? ("✖ " + deck.engine.errorText)
                        : (deck.engine.marqueeString + " │ " + deck.engine.statusText)

                    Text {
                        x: 1
                        width: Math.max(0, marqueeBox.width - 2)
                        text: marqueeBox.hasError
                            ? Theme.fitText(marqueeBox.bannerText, Math.max(0, marqueeBox.width - 2))
                            : Theme.marqueeSlice(marqueeBox.bannerText, deck.marqueeOffset, Math.max(0, marqueeBox.width - 2))
                        color: marqueeBox.hasError ? Theme.error : Theme.ledGreen
                        bold: true
                    }
                }

                // Row 1: Bitrate, Sample Rate, MONO/STEREO, and [PL][VIS] toggles
                Item {
                    width: parent.width
                    height: 1

                    Row {
                        anchors.left: parent.left
                        spacing: 1

                        Text {
                            text: String(deck.engine.bitrateKbps) + "kbps"
                            color: Theme.ledGreen
                            bold: true
                        }
                        Text {
                            text: String(deck.engine.sampleRateKhz) + "kHz"
                            color: Theme.ledGreen
                            bold: true
                        }
                        Text {
                            text: deck.engine.stereo ? "STEREO" : "MONO"
                            color: Theme.ledGreen
                            bold: true
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        spacing: 1

                        Text {
                            text: " PL "
                            reversed: deck.showPlaylist
                            bold: true
                            color: deck.showPlaylist ? Theme.ledGreen : Theme.muted
                            MouseArea {
                                anchors.fill: parent
                                onClicked: deck.togglePlaylistRequested()
                            }
                        }
                        Text {
                            text: " VIS "
                            reversed: deck.showVisualizer
                            bold: true
                            color: deck.showVisualizer ? Theme.ledGreen : Theme.muted
                            MouseArea {
                                anchors.fill: parent
                                onClicked: deck.toggleVisualizerRequested()
                            }
                        }
                    }
                }

                // Row 2: Volume Slider + Balance Slider
                Row {
                    id: volBalRow
                    width: parent.width
                    height: 1
                    spacing: 1

                    readonly property int balTextWidth: 11
                    readonly property int volReadoutWidth: 9
                    readonly property int volBarWidth: Math.max(4, width - balTextWidth - volReadoutWidth - 3)

                    Text {
                        text: "VOL " + String(Math.round(deck.engine.volume * 100)).padStart(3, " ") + "%"
                        color: Theme.text
                        bold: true
                    }

                    ProgressBar {
                        id: volBar
                        width: volBalRow.volBarWidth
                        value: deck.engine.volume * 100
                        from: 0
                        to: 100
                        style: "block"
                        thresholds: [
                            { value: 65, color: Theme.vuLow },
                            { value: 85, color: Theme.warning },
                            { value: 100, color: Theme.hot }
                        ]

                        MouseArea {
                            anchors.fill: parent
                            onClicked: (event) => {
                                if (volBar.width > 0)
                                    deck.engine.volume = Math.max(0.0, Math.min(1.0, event.x / volBar.width));
                            }
                            onWheel: (event) => {
                                deck.engine.adjustVolume(event.wheelDeltaY > 0 ? 0.05 : -0.05);
                            }
                        }
                    }

                    Text {
                        text: "BAL:" + Theme.formatBalance(deck.engine.balance).padStart(4, " ")
                        color: Math.abs(deck.engine.balance) > 0.04 ? Theme.warning : Theme.subtle
                        bold: Math.abs(deck.engine.balance) > 0.04

                        MouseArea {
                            anchors.fill: parent
                            onClicked: deck.engine.balance = 0.0
                            onWheel: (event) => {
                                deck.engine.adjustBalance(event.wheelDeltaY > 0 ? 0.1 : -0.1);
                            }
                        }
                    }
                }

                // Row 3: Status / Track counter summary line
                Item {
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: Theme.fitText(deck.engine.statusText, Math.max(0, parent.width - 8))
                        color: Theme.subtle
                    }
                    Text {
                        anchors.right: parent.right
                        visible: deck.engine.playlistCount > 0
                        text: Math.max(1, deck.engine.currentPlaylistIndex + 1) + "/" + deck.engine.playlistCount
                        color: Theme.titleBarGold
                        bold: true
                    }
                }
            }
        }

        // -------------------------------------------------------------------
        // SEEK BAR ROW
        // -------------------------------------------------------------------
        Row {
            id: seekRow
            width: parent.width
            height: 1
            spacing: 1

            readonly property string posStr: deck.engine.formatTime(deck.engine.displayPosition)
            readonly property string durStr: deck.engine.formatTime(deck.engine.displayDuration)
            readonly property int barCols: Math.max(6, width - posStr.length - durStr.length - 4)

            Text {
                text: seekRow.posStr
                color: Theme.ledGreen
                bold: true
            }

            ProgressBar {
                id: seekBar
                width: seekRow.barCols
                value: deck.engine.displayPosition
                from: 0
                to: Math.max(1, deck.engine.displayDuration)
                style: "segmented"
                color: Theme.titleBarGold

                MouseArea {
                    anchors.fill: parent
                    onClicked: (event) => {
                        if (seekBar.width > 0)
                            deck.engine.seekToRatio(event.x / seekBar.width);
                    }
                }
            }

            Text {
                text: seekRow.durStr
                color: Theme.subtle
            }
        }

        // -------------------------------------------------------------------
        // TRANSPORT BUTTONS & SHUFFLE / REPEAT ROW
        // -------------------------------------------------------------------
        Item {
            id: transportRow
            width: parent.width
            height: 1

            Row {
                anchors.left: parent.left
                spacing: 1

                Text {
                    text: "[|◀ Z]"
                    color: Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.previousTrack()
                    }
                }
                Text {
                    text: "[▶ X]"
                    color: deck.engine.playing ? Theme.ledGreen : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.play()
                    }
                }
                Text {
                    text: "[⏸ C]"
                    color: (!deck.engine.playing && !deck.engine.stopped) ? Theme.warning : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.togglePlayback()
                    }
                }
                Text {
                    text: "[■ V]"
                    color: deck.engine.stopped ? Theme.secondary : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.stop()
                    }
                }
                Text {
                    text: "[▶| B]"
                    color: Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.nextTrack()
                    }
                }
                Text {
                    text: "[⏏ L]"
                    color: Theme.titleBarGold
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.openLoadDialogRequested()
                    }
                }
            }

            Row {
                anchors.right: parent.right
                spacing: 1

                Text {
                    text: " SHUF "
                    reversed: deck.engine.shuffle
                    bold: true
                    color: deck.engine.shuffle ? Theme.ledGreen : Theme.muted
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.toggleShuffle()
                    }
                }
                Text {
                    text: " REP "
                    reversed: deck.engine.repeat
                    bold: true
                    color: deck.engine.repeat ? Theme.ledGreen : Theme.muted
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.toggleRepeat()
                    }
                }
            }
        }
    }
}
