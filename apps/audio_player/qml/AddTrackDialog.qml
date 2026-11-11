pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Minimalist modal dialog opened with L to load or append local files,
// directories, stream URLs, or the Synthwave Demo Album.
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
    width: Math.min(60, Math.max(42, parent ? parent.width - 8 : 54))
    height: 9

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
        border.style: Border.Rounded
        border.color: Theme.border
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 1
        anchors.bottomMargin: 1
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 1

        Item {
            width: parent.width
            height: 1

            Text {
                anchors.left: parent.left
                text: "LOAD / ADD MEDIA"
                color: Theme.header
                bold: true
            }
            Text {
                anchors.right: parent.right
                text: dialog.appendMode ? "mode: append" : "mode: replace"
                color: Theme.accent
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.appendMode = !dialog.appendMode
                }
            }
        }

        Text {
            text: "Path or stream URL:"
            color: Theme.text
            dim: true
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.playlistSelectedBg
            border.style: Border.None

            TextInput {
                id: pathInput
                anchors.fill: parent
                color: Theme.text
                cursorVisible: true
                cursorColor: Theme.accent
                onAccepted: dialog.submitPath()
            }
        }

        Row {
            spacing: 3

            Text {
                text: "load"
                color: Theme.accent
                bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.submitPath()
                }
            }
            Text {
                text: "demo album"
                color: Theme.header
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        dialog.engine.loadDemoAlbum();
                        dialog.close();
                    }
                }
            }
            Text {
                text: "cancel"
                color: Theme.text
                dim: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.close()
                }
            }
        }
    }
}
