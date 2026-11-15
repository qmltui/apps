pragma ComponentBehavior: Bound
import QtQml
import QmlTui
import QmlTui.Controls

// Classic 8-row Winamp 2.x Head Unit (MainDeck):
// Double-bordered chassis with recessed black LCD (digital clock + clickable
// Mini-Spectrum / Braille Mini-Scope), action-hijacked green LED marquee,
// kbps / kHz / MONO·STEREO telemetry, latched [PL][SPEC][VIS] buttons,
// color-graded Volume & center-detent Balance, interactive segmented Seek bar,
// chiseled Z X C V B L transport controls, and SHUF / REP LED pills.
Panel {
    id: deck

    required property AudioEngine engine
    property bool showPlaylist: true
    property bool showSpectrum: true
    property bool showVisualizer: true
    property bool timeRemaining: false
    property int miniVisMode: 0 // 0 = mini spectrum, 1 = Braille mini scope, 2 = off
    property int marqueeOffset: 0
    property string actionBannerText: ""

    readonly property bool fileSource: engine.source === AudioEngine.File
    readonly property bool demoSource: engine.source === AudioEngine.Demo
    readonly property string playGlyph: engine.playing ? "▶"
                                      : engine.stopped ? "■" : "⏸"
    readonly property string playStateLabel: engine.playing ? "▶ PLAYING"
                                           : engine.stopped ? "■ STOPPED" : "⏸ PAUSED"
    readonly property string playStateColor: engine.playing ? Theme.playing
                                           : engine.stopped ? Theme.muted : Theme.warning
    readonly property string clockText: {
        const pos = engine.displayPosition;
        const dur = engine.displayDuration;
        if (timeRemaining && dur > 0)
            return "-" + engine.formatTime(Math.max(0, dur - pos));
        return " " + engine.formatTime(pos);
    }

    signal togglePlaylistRequested()
    signal toggleSpectrumRequested()
    signal toggleVisualizerRequested()
    signal openLoadDialogRequested()

    function flashAction(message: string) {
        deck.actionBannerText = message;
        actionBannerTimer.restart();
    }

    function flashVolume() {
        flashAction("VOLUME: " + String(Math.round(deck.engine.volume * 100)) + "%");
    }

    function flashBalance() {
        const pct = Math.round(deck.engine.balance * 100);
        if (Math.abs(pct) <= 4)
            flashAction("BALANCE: CENTER");
        else if (pct < 0)
            flashAction("BALANCE: " + String(Math.abs(pct)) + "% LEFT");
        else
            flashAction("BALANCE: " + String(pct) + "% RIGHT");
    }

    function flashSeek(targetMs: int) {
        const dur = deck.engine.displayDuration;
        const clamped = Math.max(0, dur > 0 ? Math.min(dur, targetMs) : targetMs);
        const pct = dur > 0 ? Math.round((clamped / dur) * 100) : 0;
        flashAction("SEEK TO: " + deck.engine.formatTime(clamped)
                    + (dur > 0 ? ("/" + deck.engine.formatTime(dur) + " (" + String(pct) + "%)") : ""));
    }

    function flashLlama() {
        flashAction(marqueeBox.innerWidth >= 32
                    ? "IT REALLY WHIPS THE LLAMA'S ASS!"
                    : "IT WHIPS THE LLAMA'S ASS!");
    }

    function miniSpectrumString(cols: int): string {
        if (cols <= 0)
            return "";
        const levels = deck.engine.bandLevels;
        const count = levels ? levels.length : 0;
        if (count === 0 || !deck.engine.playing)
            return "▁".repeat(cols);
        let out = "";
        for (let i = 0; i < cols; ++i) {
            const bandIdx = Math.min(count - 1, Math.floor((i * count) / cols));
            const lvl = levels[bandIdx] || 0.0;
            const gIdx = Math.max(1, Math.min(8, Math.round(lvl * 8)));
            out += Theme.blockGlyphs[gIdx];
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
            MouseArea {
                anchors.fill: parent
                onClicked: deck.flashLlama()
            }
        }
        Text {
            text: " " + deck.engine.sourceName + " "
            reversed: true
            bold: true
            color: deck.fileSource ? Theme.fileSource
                 : deck.demoSource ? Theme.ledGreen : Theme.muted
        }
        Text {
            visible: deck.width >= 48
            text: deck.playStateLabel
            color: deck.playStateColor
            bold: true
        }
    }

    titleTrailing: Row {
        spacing: 1

        Text {
            text: "[" + Theme.skinName + "]"
            color: Theme.titleBarGold
            bold: true
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    Theme.nextSkin();
                    deck.flashAction("SKIN: " + Theme.skinName);
                }
            }
        }
    }

    Timer {
        interval: 200
        running: deck.visible && deck.engine.playing && deck.actionBannerText.length === 0
        repeat: true
        onTriggered: deck.marqueeOffset = (deck.marqueeOffset + 1) % 4096
    }

    Timer {
        id: actionBannerTimer
        interval: 1200
        repeat: false
        onTriggered: deck.actionBannerText = ""
    }

    Column {
        id: innerCol
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 0

        // -------------------------------------------------------------------
        // TOP 4 ROWS: Left Recessed LCD | Right Marquee, Telemetry & Sliders
        // -------------------------------------------------------------------
        Row {
            id: topDeckRow
            width: parent.width
            height: 4
            spacing: 1

            // Left Recessed Black LCD Cluster (Clock + Mini-Visualizer)
            Rectangle {
                id: lcdBox
                width: Math.min(18, Math.max(15, Math.floor(topDeckRow.width * 0.33)))
                height: 4
                color: Theme.lcdBackground
                border.style: Border.Single
                border.color: Theme.frame

                Column {
                    id: lcdInner
                    anchors.fill: parent
                    spacing: 0

                    // LCD Row 0: Play LED + TIME/REM + Bold LED Clock
                    Item {
                        width: parent.width
                        height: 1

                        Row {
                            anchors.left: parent.left
                            spacing: 1

                            Text {
                                text: deck.playGlyph
                                color: deck.playStateColor
                                bold: true
                            }
                            Text {
                                visible: lcdInner.width >= 13
                                text: deck.timeRemaining ? "REM" : "TIME"
                                color: Theme.dim
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            text: deck.clockText
                            color: Theme.ledGreen
                            bold: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                deck.timeRemaining = !deck.timeRemaining;
                                deck.flashAction(deck.timeRemaining ? "TIME DISPLAY: REMAINING" : "TIME DISPLAY: ELAPSED");
                            }
                        }
                    }

                    // LCD Row 1: Mini-Spectrum / Braille Mini-Scope / Off
                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            visible: deck.miniVisMode === 0
                            width: parent.width
                            height: 1
                            text: deck.miniSpectrumString(parent.width)
                            color: Theme.ledGreen
                            bold: true
                        }

                        Canvas {
                            visible: deck.miniVisMode === 1
                            width: parent.width
                            height: 1
                            renderMode: Canvas.RenderMode.Braille
                            color: Theme.scopeAccent

                            WaveformTrace {
                                samples: deck.engine.waveformMono
                                color: Theme.scopeAccent
                            }
                        }

                        Text {
                            visible: deck.miniVisMode === 2
                            width: parent.width
                            height: 1
                            text: Theme.fitText("ANALYZER OFF", parent.width)
                            color: Theme.dim
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                deck.miniVisMode = (deck.miniVisMode + 1) % 3;
                                const names = ["SPECTRUM", "OSCILLOSCOPE", "OFF"];
                                deck.flashAction("LCD ANALYZER: " + names[deck.miniVisMode]);
                            }
                        }
                    }
                }
            }

            // Right 4-Row Stack: Marquee, Telemetry + Toggles, Vol/Bal, Status
            Column {
                id: rightInfoCol
                width: Math.max(12, topDeckRow.width - lcdBox.width - topDeckRow.spacing)
                height: 4
                spacing: 0

                // Row 0: Recessed Green LED Marquee (with Winamp Action Override)
                Rectangle {
                    id: marqueeBox
                    width: parent.width
                    height: 1
                    color: Theme.lcdBackground

                    readonly property int innerWidth: Math.max(0, width - 2)
                    readonly property bool hasAction: deck.actionBannerText.length > 0
                    readonly property bool hasError: !hasAction && deck.engine.errorText.length > 0

                    Text {
                        x: 1
                        width: marqueeBox.innerWidth
                        text: marqueeBox.hasAction
                            ? Theme.fitText(deck.actionBannerText, marqueeBox.innerWidth)
                            : marqueeBox.hasError
                                ? Theme.fitText("✖ " + deck.engine.errorText, marqueeBox.innerWidth)
                                : Theme.marqueeSlice(deck.engine.marqueeString, deck.marqueeOffset, marqueeBox.innerWidth)
                        color: marqueeBox.hasError ? Theme.error
                             : marqueeBox.hasAction ? Theme.titleBarGold : Theme.ledGreen
                        bold: true
                    }
                }

                // Row 1: Bitrate / Sample Rate / MONO·STEREO + [PL][SPEC][VIS] Push-Buttons
                Item {
                    id: telemetryRow
                    width: parent.width
                    height: 1

                    readonly property bool compactPills: width < 32

                    Row {
                        anchors.left: parent.left
                        spacing: 1

                        Text {
                            text: String(deck.engine.bitrateKbps) + (telemetryRow.width >= 37 ? "kbps" : "k")
                            color: Theme.ledGreen
                            bold: true
                        }
                        Text {
                            text: String(deck.engine.sampleRateKhz) + (telemetryRow.width >= 36 ? "kHz" : "k")
                            color: Theme.ledGreen
                            bold: true
                        }
                        Text {
                            text: telemetryRow.width >= 26
                                ? (deck.engine.stereo ? "STEREO" : "MONO")
                                : (deck.engine.stereo ? "ST" : "MO")
                            color: Theme.ledGreen
                            bold: true
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        spacing: 1

                        Text {
                            text: telemetryRow.compactPills ? "PL" : " PL "
                            reversed: deck.showPlaylist
                            bold: true
                            color: deck.showPlaylist ? Theme.ledGreen : Theme.muted
                            MouseArea {
                                anchors.fill: parent
                                onClicked: deck.togglePlaylistRequested()
                            }
                        }
                        Text {
                            text: telemetryRow.compactPills ? "SP" : " SPEC "
                            reversed: deck.showSpectrum
                            bold: true
                            color: deck.showSpectrum ? Theme.ledGreen : Theme.muted
                            MouseArea {
                                anchors.fill: parent
                                onClicked: deck.toggleSpectrumRequested()
                            }
                        }
                        Text {
                            text: telemetryRow.compactPills ? "VS" : " VIS "
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

                // Row 2: Color-Graded Volume Bar + Center-Detent Balance Control
                Row {
                    id: volBalRow
                    width: parent.width
                    height: 1
                    spacing: 1

                    readonly property string volText: "VOL " + String(Math.round(deck.engine.volume * 100)).padStart(3, " ") + "%"
                    readonly property string balText: "BAL:" + Theme.formatBalance(deck.engine.balance).padStart(4, " ")
                    readonly property int volBarWidth: Math.max(4, width - volText.length - balText.length - 2)

                    Text {
                        text: volBalRow.volText
                        color: Theme.text
                        bold: true
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                deck.engine.adjustVolume(0.05);
                                deck.flashVolume();
                            }
                            onWheel: (event) => {
                                deck.engine.adjustVolume(event.wheelDeltaY > 0 ? 0.05 : -0.05);
                                deck.flashVolume();
                            }
                        }
                    }

                    ProgressBar {
                        id: volBar
                        width: volBalRow.volBarWidth
                        value: deck.engine.volume * 100
                        from: 0
                        to: 100
                        style: "block"
                        color: Theme.dim
                        thresholds: [
                            { value: 65, color: Theme.vuLow },
                            { value: 85, color: Theme.warning },
                            { value: 100, color: Theme.hot }
                        ]

                        MouseArea {
                            anchors.fill: parent
                            onClicked: (event) => {
                                if (volBar.width > 0) {
                                    deck.engine.volume = Math.max(0.0, Math.min(1.0, event.x / volBar.width));
                                    deck.flashVolume();
                                }
                            }
                            onWheel: (event) => {
                                deck.engine.adjustVolume(event.wheelDeltaY > 0 ? 0.05 : -0.05);
                                deck.flashVolume();
                            }
                        }
                    }

                    Text {
                        text: volBalRow.balText
                        color: Math.abs(deck.engine.balance) > 0.04 ? Theme.warning : Theme.subtle
                        bold: Math.abs(deck.engine.balance) > 0.04
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                deck.engine.balance = 0.0;
                                deck.flashBalance();
                            }
                            onWheel: (event) => {
                                deck.engine.adjustBalance(event.wheelDeltaY > 0 ? 0.1 : -0.1);
                                deck.flashBalance();
                            }
                        }
                    }
                }

                // Row 3: Status / BPM Line + Beat LED & Playlist Track Counter
                Item {
                    width: parent.width
                    height: 1

                    readonly property string counterText: deck.engine.playlistCount > 0
                        ? ((deck.demoSource ? (deck.engine.beatPulse > 0.5 ? "● " : "  ") : "")
                           + String(Math.max(1, deck.engine.currentPlaylistIndex + 1))
                           + "/" + String(deck.engine.playlistCount))
                        : ""

                    Text {
                        anchors.left: parent.left
                        text: Theme.fitText(deck.engine.statusText.replace(/ │ /g, " · "),
                                            Math.max(0, parent.width - parent.counterText.length - 2))
                        color: Theme.text
                        dim: true
                    }
                    Text {
                        anchors.right: parent.right
                        visible: parent.counterText.length > 0
                        text: parent.counterText
                        color: (deck.demoSource && deck.engine.beatPulse > 0.5) ? Theme.secondary : Theme.titleBarGold
                        bold: true
                    }
                }
            }
        }

        // -------------------------------------------------------------------
        // ROW 4: INTERACTIVE SEGMENTED SEEK BAR
        // -------------------------------------------------------------------
        Row {
            id: seekRow
            width: parent.width
            height: 1
            spacing: 1

            readonly property string posStr: deck.engine.formatTime(deck.engine.displayPosition)
            readonly property string durStr: deck.engine.formatTime(deck.engine.displayDuration)
            readonly property int barCols: Math.max(6, width - posStr.length - durStr.length - 2)

            Text {
                text: seekRow.posStr
                color: Theme.ledGreen
                bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        deck.timeRemaining = !deck.timeRemaining;
                        deck.flashAction(deck.timeRemaining ? "TIME DISPLAY: REMAINING" : "TIME DISPLAY: ELAPSED");
                    }
                }
            }

            ProgressBar {
                id: seekBar
                width: seekRow.barCols
                value: deck.engine.displayPosition
                from: 0
                to: Math.max(1, deck.engine.displayDuration)
                style: "line"
                color: Theme.titleBarGold
                trackColor: Theme.dim

                MouseArea {
                    anchors.fill: parent
                    onClicked: (event) => {
                        if (seekBar.width > 0) {
                            const ratio = Math.max(0.0, Math.min(1.0, event.x / seekBar.width));
                            deck.engine.seekToRatio(ratio);
                            deck.flashSeek(Math.round(ratio * deck.engine.displayDuration));
                        }
                    }
                    onWheel: (event) => {
                        const delta = event.wheelDeltaY > 0 ? 5000 : -5000;
                        const nextPos = deck.engine.displayPosition + delta;
                        deck.engine.seekTrack(delta);
                        deck.flashSeek(nextPos);
                    }
                }
            }

            Text {
                text: seekRow.durStr
                color: Theme.text
                dim: true
            }
        }

        // -------------------------------------------------------------------
        // ROW 5: CHISELED TRANSPORT BUTTONS & SHUF / REP LED PILLS
        // -------------------------------------------------------------------
        Item {
            id: transportRow
            width: parent.width
            height: 1

            readonly property bool compactKeys: width < 50

            Row {
                anchors.left: parent.left
                spacing: 1

                Text {
                    text: transportRow.compactKeys ? "[|◀]" : "[|◀ Z]"
                    color: Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.previousTrack()
                    }
                }
                Text {
                    text: transportRow.compactKeys ? "[▶]" : "[▶ X]"
                    color: deck.engine.playing ? Theme.ledGreen : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.play()
                    }
                }
                Text {
                    text: transportRow.compactKeys ? "[⏸]" : "[⏸ C]"
                    color: (!deck.engine.playing && !deck.engine.stopped) ? Theme.warning : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.togglePlayback()
                    }
                }
                Text {
                    text: transportRow.compactKeys ? "[■]" : "[■ V]"
                    color: deck.engine.stopped ? Theme.secondary : Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.stop()
                    }
                }
                Text {
                    text: transportRow.compactKeys ? "[▶|]" : "[▶| B]"
                    color: Theme.text
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.nextTrack()
                    }
                }
                Text {
                    text: transportRow.compactKeys ? "[⏏]" : "[⏏ L]"
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
                        onClicked: {
                            deck.engine.toggleShuffle();
                            deck.flashAction(deck.engine.shuffle ? "SHUFFLE: ON" : "SHUFFLE: OFF");
                        }
                    }
                }
                Text {
                    text: " REP "
                    reversed: deck.engine.repeat
                    bold: true
                    color: deck.engine.repeat ? Theme.ledGreen : Theme.muted
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            deck.engine.toggleRepeat();
                            deck.flashAction(deck.engine.repeat ? "REPEAT: ON" : "REPEAT: OFF");
                        }
                    }
                }
            }
        }
    }
}
