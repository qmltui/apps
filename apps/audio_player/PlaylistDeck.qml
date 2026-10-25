pragma ComponentBehavior: Bound
import QtQml
import QmlTui
import QmlTui.Controls

// Playlist Editor Window (Deck):
// Scrollable tracklist (Synth Demo Album, Local Files/URLs),
// current playing track indicator, cursor row selection, and [+ADD][-REM][DEMO][CLR] bar.
Panel {
    id: plDeck

    required property AudioEngine engine

    signal openLoadDialogRequested()

    color: Theme.playlistBg
    border.style: Border.Single
    border.color: Theme.frame
    title: Row {
        spacing: 1
        Text {
            text: "PLAYLIST"
            color: Theme.titleBarGold
            bold: true
        }
        Text {
            text: "(" + plDeck.engine.playlistCount + " TRKS)"
            color: Theme.subtle
        }
    }
    titleTrailing: Text {
        text: plDeck.engine.formatTime(plDeck.engine.displayPosition) + " / " + plDeck.engine.totalDurationText
        color: Theme.ledGreen
        bold: true
    }

    Column {
        id: plInner
        anchors.fill: parent
        anchors.topMargin: 1
        anchors.bottomMargin: 1
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // Tracklist Area
        Item {
            id: listArea
            width: parent.width
            height: Math.max(1, plInner.height - 2)

            ListView {
                id: trackListView
                anchors.fill: parent
                clip: true
                focusPolicy: Qt.NoFocus
                model: plDeck.engine.playlistModel
                currentIndex: plDeck.engine.selectedTrackIndex

                delegate: Rectangle {
                    id: rowDel

                    required property int index
                    required property var modelData

                    readonly property bool isSelected: rowDel.index === plDeck.engine.selectedTrackIndex
                    readonly property bool isPlayingTrack: rowDel.modelData && rowDel.modelData.current
                    readonly property string durStr: (rowDel.modelData && rowDel.modelData.durationText) ? rowDel.modelData.durationText : "--:--"
                    readonly property string leftLabel: {
                        const prefix = isPlayingTrack ? "▶ " : (isSelected ? "▸ " : "  ");
                        const num = rowDel.modelData ? rowDel.modelData.number : (rowDel.index + 1);
                        const title = rowDel.modelData ? rowDel.modelData.displayTitle : "";
                        return prefix + num + ". " + title;
                    }

                    width: trackListView.width
                    height: 1
                    color: isSelected ? Theme.playlistSelectedBg : "default"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.engine.playTrackAt(rowDel.index)
                    }

                    Text {
                        x: 0
                        width: Math.max(0, rowDel.width - rowDel.durStr.length - 2)
                        text: Theme.fitText(rowDel.leftLabel, Math.max(0, rowDel.width - rowDel.durStr.length - 2))
                        color: rowDel.isPlayingTrack ? Theme.playlistActive : Theme.playlistText
                        bold: rowDel.isPlayingTrack || rowDel.isSelected
                    }

                    Text {
                        anchors.right: parent.right
                        text: rowDel.durStr
                        color: rowDel.isPlayingTrack ? Theme.playlistActive : Theme.subtle
                        bold: rowDel.isPlayingTrack
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                visible: plDeck.engine.playlistCount === 0
                text: "Playlist empty · press L to load files or click [DEMO]"
                color: Theme.muted
            }
        }

        // Playlist Footer Controls Bar
        Item {
            width: parent.width
            height: 1

            Row {
                anchors.left: parent.left
                spacing: 1

                Text {
                    text: "[+ADD L]"
                    color: Theme.ledGreen
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.openLoadDialogRequested()
                    }
                }
                Text {
                    text: "[-REM Del]"
                    color: Theme.subtle
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.engine.removeSelectedTrack()
                    }
                }
                Text {
                    text: "[DEMO]"
                    color: plDeck.engine.source === AudioEngine.Demo ? Theme.titleBarGold : Theme.subtle
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.engine.loadDemoAlbum()
                    }
                }
                Text {
                    text: "[CLR]"
                    color: Theme.muted
                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.engine.clearPlaylist()
                    }
                }
            }

            Text {
                anchors.right: parent.right
                text: "↑↓/Enter"
                color: Theme.muted
            }
        }
    }
}
