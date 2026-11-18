pragma ComponentBehavior: Bound
import QmlTui

Column {
    id: root

    required property string cardTitle
    required property string cardSubtitle
    property int streamSpacing: 0

    width: parent ? parent.width : 96
    height: 2
    spacing: 0

    Text {
        width: root.width
        height: 1
        spans: [
            {
                text: "❯ ",
                color: Theme.cyan,
                bold: true
            },
            {
                text: root.cardTitle,
                color: Theme.ink,
                bold: true
            },
            {
                text: "  [qpm v0.6.0]",
                color: Theme.green,
                bold: true
            }
        ]
    }

    Text {
        width: root.width
        height: 1
        text: "  " + root.cardSubtitle
        color: Theme.muted
    }
}
