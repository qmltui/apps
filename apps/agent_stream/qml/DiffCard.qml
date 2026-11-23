pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: diffCard

    required property string title
    required property string extra
    required property string hunkHeader
    required property string removedLines
    required property string addedLines

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "▼ Patch:"
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " " + diffCard.title
            color: "white"
            bold: true
        }
        Text {
            text: " · " + (diffCard.extra || "(+3, -1 lines)") + " [F3 for side-by-side]"
            color: Theme.muted
        }
    }

    Text {
        x: 2
        text: diffCard.hunkHeader
        color: Theme.violet
    }
    Text {
        visible: diffCard.removedLines.length > 0
        x: 2
        text: diffCard.removedLines
        color: Theme.rose
    }
    Text {
        visible: diffCard.addedLines.length > 0
        x: 2
        text: diffCard.addedLines
        color: Theme.green
    }
}
