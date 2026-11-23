pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: userCard

    required property string content

    width: parent ? parent.width : 80
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "❯"
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " " + userCard.content
            color: "white"
            bold: true
        }
    }
}
