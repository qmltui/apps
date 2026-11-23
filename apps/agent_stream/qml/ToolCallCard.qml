pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: toolCallCard

    required property string title
    required property string content

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "⚡"
            color: Theme.amber
            bold: true
        }
        Text {
            text: " " + toolCallCard.title
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " " + toolCallCard.content
            color: Theme.slate
        }
    }
}
