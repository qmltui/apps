pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: root

    required property var parentEntries
    required property string currentPath
    required property string parentPath

    signal itemClicked(int index)
    signal requestFileManagerFocus()

    color: Theme.bgColumnDim
    border.style: Border.None

    ListView {
        id: parentList
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.bottom: parent.bottom
        clip: true
        focusPolicy: Qt.NoFocus
        model: root.parentEntries

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
            id: parentDelegate

            required property int index
            required property var modelData

            width: parentList.width
            height: 1

            readonly property bool isCurrentFolder: parentDelegate.modelData.path === root.currentPath
            color: isCurrentFolder ? Theme.bgCursor : QmlTui.transparent

            Row {
                x: 1
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1

                Text {
                    text: parentDelegate.modelData.icon || "📄"
                    color: parentDelegate.isCurrentFolder ? Theme.accentCyan : Theme.textDim
                }

                Text {
                    text: parentDelegate.modelData.name || ""
                    color: parentDelegate.isCurrentFolder ? Theme.textPrimary : Theme.textMuted
                    bold: parentDelegate.isCurrentFolder
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: !root.parentEntries || root.parentEntries.length === 0
        text: root.parentPath === "" ? "(root)" : "(empty)"
        color: Theme.textDim
    }
}
