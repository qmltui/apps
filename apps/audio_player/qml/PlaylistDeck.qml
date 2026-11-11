pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Playlist pane inside the single-hairline divided grid:
// Focusable with the mouse or keyboard, with inline header actions and a
// scrollable tracklist.
Item {
    id: plDeck

    required property AudioEngine engine
    readonly property bool playlistFocused: trackListView.activeFocus

    signal openLoadDialogRequested()

    // Clicking anywhere in the playlist pane focuses the track list
    MouseArea {
        anchors.fill: parent
        onClicked: trackListView.forceActiveFocus()
    }

    Connections {
        target: plDeck.engine
        function onSelectedTrackChanged() {
            if (trackListView.currentIndex !== plDeck.engine.selectedTrackIndex)
                trackListView.currentIndex = plDeck.engine.selectedTrackIndex;
        }
    }

    Column {
        id: plInner
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // Header Row
        Item {
            id: headerRow
            width: parent.width
            height: 1

            readonly property int titleCols: plDeck.playlistFocused ? 10 : 8
            readonly property int availMetaCols: Math.max(0,
                width - rightActions.implicitWidth - titleCols - 3)
            readonly property string metaLabel: {
                const count = String(plDeck.engine.playlistCount);
                const dur = plDeck.engine.totalDurationText;
                const full = "· " + count + " tracks · " + dur;
                if (availMetaCols >= full.length)
                    return full;
                const medium = "· " + count + " · " + dur;
                if (availMetaCols >= medium.length)
                    return medium;
                const compact = "· " + count;
                if (availMetaCols >= compact.length)
                    return compact;
                return "";
            }

            Row {
                anchors.left: parent.left
                spacing: 1

                Text {
                    text: plDeck.playlistFocused ? "▸ PLAYLIST" : "PLAYLIST"
                    color: plDeck.playlistFocused ? Theme.accent : Theme.header
                    bold: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: trackListView.forceActiveFocus()
                    }
                }
                Text {
                    visible: headerRow.metaLabel.length > 0
                    text: headerRow.metaLabel
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: trackListView.forceActiveFocus()
                    }
                }
            }

            Row {
                id: rightActions
                anchors.right: parent.right
                spacing: 1

                Text {
                    text: "+add"
                    color: Theme.accent
                    MouseArea {
                        anchors.fill: parent
                        onClicked: plDeck.openLoadDialogRequested()
                    }
                }
                Text {
                    text: "-rem"
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            trackListView.forceActiveFocus();
                            plDeck.engine.removeSelectedTrack();
                        }
                    }
                }
                Text {
                    text: "demo"
                    color: plDeck.engine.source === AudioEngine.Demo ? Theme.header : Theme.text
                    dim: plDeck.engine.source !== AudioEngine.Demo
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            trackListView.forceActiveFocus();
                            plDeck.engine.loadDemoAlbum();
                        }
                    }
                }
                Text {
                    text: "clr"
                    color: Theme.text
                    dim: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            trackListView.forceActiveFocus();
                            plDeck.engine.clearPlaylist();
                        }
                    }
                }
            }
        }

        // Tracklist Area
        Item {
            id: listArea
            width: parent.width
            height: Math.max(1, plInner.height - 2)

            ListView {
                id: trackListView
                anchors.fill: parent
                clip: true
                focusPolicy: Qt.StrongFocus
                model: plDeck.engine.playlistModel
                currentIndex: plDeck.engine.selectedTrackIndex

                onCurrentIndexChanged: {
                    if (trackListView.currentIndex >= 0
                            && trackListView.currentIndex !== plDeck.engine.selectedTrackIndex) {
                        plDeck.engine.selectedTrackIndex = trackListView.currentIndex;
                    }
                }

                onItemActivated: (index) => {
                    plDeck.engine.playTrackAt(index);
                }

                Keys.onSpacePressed: (event) => {
                    plDeck.engine.togglePlayback();
                    event.accepted = true;
                }

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
                        color: rowDel.isPlayingTrack ? Theme.playlistActive : Theme.text
                        dim: !rowDel.isPlayingTrack
                        bold: rowDel.isPlayingTrack
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                visible: plDeck.engine.playlistCount === 0
                text: "Empty playlist"
                color: Theme.text
                dim: true
            }
        }
    }
}
