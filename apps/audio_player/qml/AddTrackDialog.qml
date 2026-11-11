pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Modal dialog opened with L or the [⏏ L] / [+ADD L] buttons to load or append
// local files, directories, stream URLs, or the Synthwave Demo Album.
Popup {
    id: dialog

    required property AudioEngine engine
    property bool appendMode: true

    signal closedAndRestoreFocus()

    function submitPath() {
        const trimmed = pathInput.text.trim();
        if (trimmed.length > 0) {
            if (dialog.appendMode)
                dialog.engine.appendPlaylistInputs([trimmed]);
            else
                dialog.engine.loadPlaylist([trimmed]);
        }
        dialog.close();
        dialog.closedAndRestoreFocus();
    }

    modal: true
    dimBackground: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: parent
    width: Math.min(64, Math.max(44, parent ? parent.width - 8 : 56))
    height: 11

    onOpened: {
        pathInput.focus = true;
    }

    onClosed: {
        dialog.closedAndRestoreFocus();
    }

    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_Escape) {
            dialog.close();
            event.accepted = true;
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            dialog.submitPath();
            event.accepted = true;
        } else if (event.key === Qt.Key_Tab) {
            dialog.appendMode = !dialog.appendMode;
            event.accepted = true;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.panel
        border.style: Border.Double
        border.color: Theme.activeFrame
    }

    Column {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "═══ AUDIO PLAYER // LOAD / ADD MEDIA ═══"
            color: Theme.titleBarGold
            bold: true
        }

        Text {
            text: "File, folder, or stream URL:"
            color: Theme.subtle
        }

        Rectangle {
            width: parent.width - 2
            height: 1
            color: Theme.playlistSelectedBg
            border.style: Border.None

            TextInput {
                id: pathInput
                anchors.fill: parent
                color: Theme.playlistActive
                cursorVisible: true
                cursorColor: Theme.ledGreen
                onAccepted: dialog.submitPath()
            }
        }

        Row {
            spacing: 2

            Text {
                text: "Action [Tab]:"
                color: Theme.muted
            }
            Text {
                text: dialog.appendMode ? "▸ APPEND TO PLAYLIST" : "▸ REPLACE PLAYLIST"
                color: Theme.ledGreen
                bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.appendMode = !dialog.appendMode
                }
            }
        }

        Row {
            spacing: 2

            Text {
                text: "[Enter: Load Path]"
                color: Theme.ledGreen
                bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.submitPath()
                }
            }
            Text {
                text: "[Demo Album]"
                color: Theme.titleBarGold
                bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        dialog.engine.loadDemoAlbum();
                        dialog.close();
                    }
                }
            }
            Text {
                text: "[Esc: Cancel]"
                color: Theme.subtle
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.close()
                }
            }
        }
    }
}
