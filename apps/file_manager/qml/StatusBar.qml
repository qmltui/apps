pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: root

    required property var previewData
    required property int currentIndex
    required property int totalCount
    required property int markedCount
    required property string sortField
    required property bool sortAscending
    required property string statusMessage
    required property string modeName
    required property color modeColor

    height: 1
    color: Theme.bgHeader
    border.style: Border.None

    Row {
        x: 2
        y: 0
        width: parent.width - 4
        spacing: 2

        Text {
            text: root.modeName
            color: root.modeColor
            bold: true
        }

        Text {
            text: {
                if (root.previewData && root.previewData.permissions) {
                    return root.previewData.permissions + "  " + (root.previewData.owner || "");
                }
                return "-";
            }
            color: Theme.textMuted
        }

        Text {
            visible: root.statusMessage !== ""
            text: " " + root.statusMessage + " "
            color: Theme.accentYellow
            bold: true
        }

        // Right side info: Marked items, Sort mode, Index/percentage, Hints
        Row {
            anchors.right: parent.right
            spacing: 2

            Text {
                visible: root.markedCount > 0
                text: "[" + root.markedCount + " marked]"
                color: Theme.accentPeach
                bold: true
            }

            Text {
                text: "Sort: " + root.sortField + (root.sortAscending ? " ▲" : " ▼")
                color: Theme.textSecondary
            }

            Text {
                text: {
                    if (root.totalCount === 0) return "0/0 (0%)";
                    const pos = (root.currentIndex + 1);
                    const pct = Math.round((pos / root.totalCount) * 100);
                    return pos + "/" + root.totalCount + " (" + pct + "%)";
                }
                color: Theme.accentCyan
                bold: true
            }

            Text {
                text: "[?]Help  [!]Shell  [q]Quit"
                color: Theme.textDim
            }
        }
    }
}
