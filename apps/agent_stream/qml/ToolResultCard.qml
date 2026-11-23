pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: toolResultCard

    required property string content
    property int streamSpacing: 0

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "  →"
            color: Theme.muted
        }
        Text {
            text: " " + toolResultCard.content
            color: Theme.green
        }
    }
}
