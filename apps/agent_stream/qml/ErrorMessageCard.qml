pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: errCard

    required property string content

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "✖"
            color: Theme.rose
            bold: true
        }
        Text {
            text: " " + errCard.content
            color: Theme.roseLight
        }
    }
}
