pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: thoughtCard

    required property string content
    required property string title

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "▼ Thought"
            color: Theme.violet
            bold: true
        }
        Text {
            text: " (" + (thoughtCard.title || "0.8s") + ")"
            color: Theme.muted
        }
    }

    Text {
        x: 2
        text: thoughtCard.content
        color: Theme.slate
        italic: true
    }
}
