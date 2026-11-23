pragma ComponentBehavior: Bound

import QmlTui

Popup {
    id: diffModal

    required property DiffPreviewModel diffModel

    readonly property string targetFile: diffModal.diffModel.targetFile

    signal switchToModelRequested

    width: parent ? parent.width : 78
    height: 13
    clip: true
    modal: true
    dimBackground: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnOutsideClick

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape || event.key === Qt.Key_F3) {
            diffModal.close();
            event.accepted = true;
        } else if (event.key === Qt.Key_F2) {
            diffModal.switchToModelRequested();
            event.accepted = true;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.surface
    }

    Column {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0

        // Header Title
        Row {
            width: parent.width
            height: 1
            spacing: 1

            StatusBadge {
                text: "Diff"
                status: "warning"
                variant: "subtle"
                showIcon: false
            }

            Text {
                text: diffModal.diffModel.targetFile
                color: Theme.amber
                bold: true
            }

            Text {
                text: " · "
                color: Theme.muted
            }

            Rectangle {
                width: closeText.implicitWidth + 2
                height: 1
                color: closeMouse.containsMouse ? Theme.hoverBg : "transparent"

                Text {
                    id: closeText
                    anchors.centerIn: parent
                    text: "[Esc/F3] close"
                    color: closeMouse.containsMouse ? "white" : Theme.muted
                    bold: closeMouse.containsMouse
                    underline: closeMouse.containsMouse
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: diffModal.close()
                }
            }
        }

        // Subheader stats
        Row {
            width: parent.width
            height: 1
            spacing: 1

            Text {
                text: diffModal.diffModel.hunkHeader
                color: Theme.cyan
                bold: true
            }
            Text {
                text: diffModal.diffModel.additionsLabel
                color: Theme.green
            }
            Text {
                text: "·"
                color: Theme.muted
            }
            Text {
                text: diffModal.diffModel.deletionsLabel
                color: Theme.rose
            }
        }

        // Column Titles (Before / After Side-by-Side)
        Row {
            width: parent.width
            height: 1
            spacing: 0

            Text {
                width: 36
                text: "  Original (HEAD)"
                color: Theme.muted
                bold: true
            }
            Text {
                text: "  "
                color: Theme.muted
            }
            Text {
                text: "Modified (proposed patch)"
                color: Theme.muted
                bold: true
            }
        }

        // Side-by-side diff rows backed by C++ DiffPreviewModel
        Repeater {
            model: diffModal.diffModel
            delegate: Rectangle {
                id: diffRow

                required property string leftText
                required property color leftColor
                required property string rightText
                required property color rightColor
                required property color rowBg

                width: parent.width
                height: 1
                color: diffRow.rowBg

                Row {
                    width: parent.width
                    height: 1
                    spacing: 0

                    Text {
                        width: 36
                        text: diffRow.leftText
                        color: diffRow.leftColor
                    }
                    Text {
                        text: "  "
                        color: Theme.muted
                    }
                    Text {
                        text: diffRow.rightText
                        color: diffRow.rightColor
                    }
                }
            }
        }
    }
}
