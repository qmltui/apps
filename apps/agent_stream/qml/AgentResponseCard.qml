pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: agentCard

    required property string content
    required property string title
    required property string extra

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "✦ " + (agentCard.title || "Agent")
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " (" + (agentCard.extra || "2.1k tokens · 0.8s") + ")"
            color: Theme.muted
        }
    }

    Text {
        x: 2
        text: agentCard.content
        color: "white"
    }
}
