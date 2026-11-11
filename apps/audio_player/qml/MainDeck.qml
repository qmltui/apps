pragma ComponentBehavior: Bound
import QtQml
import QmlTui
import QmlTui.Controls

// Full-width 2-row bottom player bar:
//   Row 0: Brand + Play State + Now Playing Status/Track + Format/Modes/Volume
//   Row 1: Transport Controls + Clock + Full-Width Dotted Seek Bar + Duration
Item {
    id: deck

    required property AudioEngine engine
    property bool timeRemaining: false

    readonly property bool demoSource: engine.source === AudioEngine.Demo
    readonly property string playStateLabel: engine.playing ? "PLAYING"
                                           : engine.stopped ? "STOPPED" : "PAUSED"
    readonly property string playStateColor: engine.playing ? Theme.playing
                                           : engine.stopped ? Theme.muted : Theme.warning
    readonly property string clockText: {
        const pos = engine.displayPosition;
        const dur = engine.displayDuration;
        if (timeRemaining && dur > 0)
            return "-" + engine.formatTime(Math.max(0, dur - pos));
        return engine.formatTime(pos);
    }

    Column {
        id: innerCol
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 0

        // Row 0: Status, Now Playing Track, Format & Playback Modes
        Item {
            id: topRow
            width: parent.width
            height: 1

            readonly property bool hasError: deck.engine.errorText.length > 0
            readonly property string rawBanner: hasError
                ? ("✖ " + deck.engine.errorText)
                : deck.engine.statusText.replace(/ │ /g, " · ")
            readonly property int availBannerCols: Math.max(8,
                width - leftPrefix.implicitWidth - rightControls.implicitWidth - 6)

            Row {
                id: leftPrefix
                anchors.left: parent.left
                spacing: 2

                Text {
                    text: "⚡ AUDIO PLAYER"
                    color: Theme.header
                    bold: true
                }
                Text {
                    text: deck.playStateLabel
                    color: deck.playStateColor
                    bold: true
                }
            }

            Row {
                anchors.left: leftPrefix.right
                anchors.leftMargin: 2
                spacing: 2

                Text {
                    text: Theme.fitText(topRow.rawBanner, topRow.availBannerCols)
                    color: topRow.hasError ? Theme.error : Theme.text
                    bold: topRow.hasError
                }
                Text {
                    visible: deck.demoSource && deck.engine.beatPulse > 0.5
                    text: "●"
                    color: Theme.secondary
                    bold: true
                }
            }

            Row {
                id: rightControls
                anchors.right: parent.right
                spacing: 2

                Text {
                    visible: deck.width >= 92
                    text: String(deck.engine.bitrateKbps) + "k · " + String(deck.engine.sampleRateKhz) + "kHz"
                    color: Theme.text
                    dim: true
                }
                Text {
                    text: "SHUF"
                    color: deck.engine.shuffle ? Theme.accent : Theme.text
                    dim: !deck.engine.shuffle
                    bold: deck.engine.shuffle
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.toggleShuffle()
                    }
                }
                Text {
                    text: "REP"
                    color: deck.engine.repeat ? Theme.accent : Theme.text
                    dim: !deck.engine.repeat
                    bold: deck.engine.repeat
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.toggleRepeat()
                    }
                }
                Text {
                    visible: Math.abs(deck.engine.balance) > 0.04
                    text: "BAL " + Theme.formatBalance(deck.engine.balance)
                    color: Theme.warning
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.balance = 0.0
                        onWheel: (event) => {
                            deck.engine.adjustBalance(event.wheelDeltaY > 0 ? 0.1 : -0.1);
                        }
                    }
                }
                Text {
                    text: "VOL " + String(Math.round(deck.engine.volume * 100)) + "%"
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.adjustVolume(0.05)
                        onWheel: (event) => {
                            deck.engine.adjustVolume(event.wheelDeltaY > 0 ? 0.05 : -0.05);
                        }
                    }
                }
            }
        }

        // Row 1: Transport Controls + Full-Width Seek Bar
        Row {
            id: bottomRow
            width: parent.width
            height: 1
            spacing: 2

            readonly property string posStr: deck.clockText
            readonly property string durStr: deck.engine.formatTime(deck.engine.displayDuration)
            readonly property int barCols: Math.max(8,
                width - transportBtns.implicitWidth - posStr.length - durStr.length - 4)

            Row {
                id: transportBtns
                spacing: 2

                Text {
                    text: "|◀"
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.previousTrack()
                    }
                }
                Text {
                    text: "▶"
                    color: deck.engine.playing ? Theme.accent : Theme.text
                    dim: !deck.engine.playing
                    bold: deck.engine.playing
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.play()
                    }
                }
                Text {
                    text: "⏸"
                    color: (!deck.engine.playing && !deck.engine.stopped) ? Theme.warning : Theme.text
                    dim: deck.engine.playing || deck.engine.stopped
                    bold: !deck.engine.playing && !deck.engine.stopped
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.togglePlayback()
                    }
                }
                Text {
                    text: "■"
                    color: deck.engine.stopped ? Theme.secondary : Theme.text
                    dim: !deck.engine.stopped
                    bold: deck.engine.stopped
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.stop()
                    }
                }
                Text {
                    text: "▶|"
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.engine.nextTrack()
                    }
                }
            }

            Row {
                spacing: 1

                Text {
                    text: bottomRow.posStr
                    color: Theme.accent
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: deck.timeRemaining = !deck.timeRemaining
                    }
                }

                ProgressBar {
                    id: seekBar
                    width: bottomRow.barCols
                    value: deck.engine.displayPosition
                    from: 0
                    to: Math.max(1, deck.engine.displayDuration)
                    fillCharacter: "━"
                    emptyCharacter: "·"
                    color: Theme.header

                    MouseArea {
                        anchors.fill: parent
                        onClicked: (event) => {
                            if (seekBar.width > 0)
                                deck.engine.seekToRatio(event.x / seekBar.width);
                        }
                    }
                }

                Text {
                    text: bottomRow.durStr
                    color: Theme.text
                    dim: true
                }
            }
        }
    }
}
