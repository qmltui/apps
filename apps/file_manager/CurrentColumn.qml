pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: root

    required property var currentEntries
    required property int currentIndex
    required property string filterText

    signal itemClicked(int index)
    signal requestFileManagerFocus()

    color: Theme.bgColumn
    border.style: Border.None

    ListView {
        id: entryList
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.bottom: parent.bottom
        clip: true
        focusPolicy: Qt.NoFocus
        model: root.currentEntries
        currentIndex: root.currentIndex

        onFocusChanged: {
            if (focus) {
                focus = false;
                root.requestFileManagerFocus();
            }
        }

        onItemActivated: index => {
            root.itemClicked(index);
        }

        delegate: Rectangle {
            id: entryDelegate

            required property int index
            required property var modelData

            width: entryList.width
            height: 1

            readonly property bool isCurrent: entryDelegate.index === root.currentIndex
            readonly property bool isMarked: entryDelegate.modelData.isMarked || false

            color: {
                if (entryDelegate.isCurrent && entryDelegate.isMarked) return Theme.bgSelected;
                if (entryDelegate.isCurrent) return Theme.bgCursor;
                if (entryDelegate.isMarked) return Theme.bgMarked;
                return QmlTui.transparent;
            }

            // Left side: Cursor, Mark checkbox, Icon, File Name
            Row {
                x: 1
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1

                // Cursor pointer
                Text {
                    text: entryDelegate.isCurrent ? "▸" : " "
                    color: Theme.accentYellow
                    bold: true
                }

                // Selection checkmark (only shown when marked)
                Text {
                    visible: entryDelegate.isMarked
                    text: "[✓]"
                    color: Theme.accentPeach
                    bold: true
                }

                // File type icon
                Text {
                    text: entryDelegate.modelData.icon || "📄"
                    color: entryDelegate.modelData.iconColor || Theme.textPrimary
                }

                // File / Directory Name
                Text {
                    text: entryDelegate.modelData.name || ""
                    color: {
                        if (entryDelegate.isCurrent) return Theme.textHighlight;
                        if (entryDelegate.isMarked) return Theme.accentPeach;
                        if (entryDelegate.modelData.isDir) return Theme.accentBlue;
                        return Theme.textPrimary;
                    }
                    bold: entryDelegate.isCurrent || entryDelegate.modelData.isDir
                }
            }

            // Right side: File size or item count
            Text {
                anchors.right: parent.right
                anchors.rightMargin: 1
                anchors.verticalCenter: parent.verticalCenter
                text: entryDelegate.modelData.sizeFormatted || ""
                color: entryDelegate.isCurrent ? Theme.textSecondary : Theme.textDim
            }
        }
    }

    Text {
        anchors.centerIn: entryList
        visible: !root.currentEntries || root.currentEntries.length === 0
        text: root.filterText !== "" ? "No matching files" : "(Empty directory)"
        color: Theme.textDim
    }
}
