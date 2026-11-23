pragma ComponentBehavior: Bound

import QmlTui

// Live in-flight token stream rendered inside Terminal.Inline before the block
// completes and flushes permanently into Scrollback.
Column {
    id: liveCard

    required property int streamKind
    required property string title
    required property string content

    readonly property bool isThought: liveCard.streamKind === AgentSession.LiveThoughtStream

    width: parent ? parent.width : 80
    spacing: 0

    Item {
        width: parent.width
        height: 1
    }

    Row {
        spacing: 1

        Text {
            text: liveCard.isThought ? "▼ Thought" : "✦ Agent"
            color: liveCard.isThought ? Theme.violet : Theme.cyan
            bold: true
        }
        Text {
            text: "(" + liveCard.title + ")"
            color: Theme.muted
        }
    }

    Text {
        x: 2
        text: liveCard.content + "▍"
        color: liveCard.isThought ? Theme.slate : "white"
        italic: liveCard.isThought
    }
}
