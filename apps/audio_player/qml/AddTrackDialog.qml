pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Minimalist modal dialog opened with L to load or append local files,
// directories, stream URLs, or the Synthwave Demo Album.
Popup {
    id: dialog

    required property AudioEngine engine
    property bool appendMode: true
    // 0: pathInput, 1: load, 2: demo album, 3: cancel, 4: mode toggle
    property int focusedAction: 0

    signal closedAndRestoreFocus

    function setFocusedAction(action: int) {
        dialog.focusedAction = action;
        if (action === 0)
            pathInput.focus = true;
        else
            dialog.focus = true;
    }

    function loadDemo() {
        dialog.engine.loadDemoAlbum();
        dialog.close();
        dialog.closedAndRestoreFocus();
    }

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

    function activateFocusedAction() {
        if (dialog.focusedAction === 2) {
            dialog.loadDemo();
        } else if (dialog.focusedAction === 3) {
            dialog.close();
        } else if (dialog.focusedAction === 4) {
            dialog.appendMode = !dialog.appendMode;
        } else {
            dialog.submitPath();
        }
    }

    modal: true
    dimBackground: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: parent
    width: Math.min(60, Math.max(42, parent ? parent.width - 8 : 54))
    height: 9

    onOpened: {
        dialog.setFocusedAction(0);
    }

    onClosed: {
        dialog.closedAndRestoreFocus();
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            dialog.close();
            event.accepted = true;
        } else if ((event.modifiers & (Qt.ControlModifier | Qt.AltModifier)) && event.key === Qt.Key_D) {
            dialog.loadDemo();
            event.accepted = true;
        } else if ((event.modifiers & (Qt.ControlModifier | Qt.AltModifier)) && event.key === Qt.Key_M) {
            dialog.appendMode = !dialog.appendMode;
            event.accepted = true;
        } else if (event.key === Qt.Key_Tab) {
            dialog.setFocusedAction((dialog.focusedAction + 1) % 5);
            event.accepted = true;
        } else if (event.key === Qt.Key_Backtab) {
            dialog.setFocusedAction((dialog.focusedAction + 4) % 5);
            event.accepted = true;
        } else if (event.key === Qt.Key_Down && (dialog.focusedAction === 0 || dialog.focusedAction === 4)) {
            dialog.setFocusedAction(1);
            event.accepted = true;
        } else if (event.key === Qt.Key_Up && dialog.focusedAction >= 1 && dialog.focusedAction <= 3) {
            dialog.setFocusedAction(0);
            event.accepted = true;
        } else if (event.key === Qt.Key_Right && dialog.focusedAction >= 1 && dialog.focusedAction <= 3) {
            dialog.setFocusedAction(dialog.focusedAction === 3 ? 1 : dialog.focusedAction + 1);
            event.accepted = true;
        } else if (event.key === Qt.Key_Left && dialog.focusedAction >= 1 && dialog.focusedAction <= 3) {
            dialog.setFocusedAction(dialog.focusedAction === 1 ? 3 : dialog.focusedAction - 1);
            event.accepted = true;
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            dialog.activateFocusedAction();
            event.accepted = true;
        } else if (event.key === Qt.Key_Space && dialog.focusedAction !== 0) {
            dialog.activateFocusedAction();
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
                text: {
                    const label = dialog.appendMode ? "mode: append" : "mode: replace";
                    return dialog.focusedAction === 4 ? ("[" + label + "]") : label;
                }
                color: Theme.accent
                bold: dialog.focusedAction === 4
                underline: dialog.focusedAction === 4
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
                cursorVisible: dialog.focusedAction === 0
                cursorColor: Theme.accent
                onAccepted: dialog.submitPath()
            }
        }

        Row {
            spacing: 3

            Text {
                text: dialog.focusedAction === 1 ? "[load]" : "load"
                color: Theme.accent
                bold: true
                underline: dialog.focusedAction === 1
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.submitPath()
                }
            }
            Text {
                text: dialog.focusedAction === 2 ? "[demo album]" : "demo album"
                color: Theme.header
                bold: dialog.focusedAction === 2
                underline: dialog.focusedAction === 2
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.loadDemo()
                }
            }
            Text {
                text: dialog.focusedAction === 3 ? "[cancel]" : "cancel"
                color: Theme.text
                dim: dialog.focusedAction !== 3
                bold: dialog.focusedAction === 3
                underline: dialog.focusedAction === 3
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.close()
                }
            }
        }
    }
}
