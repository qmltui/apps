pragma ComponentBehavior: Bound
import QmlTui

Row {
    id: root

    required property string verb
    required property string verbColor
    required property string detail
    required property string trailing
    property int streamSpacing: 0

    readonly property string paddedVerb: " ".repeat(Math.max(0, 11 - verb.length)) + verb + "  "

    width: parent ? parent.width : 96
    height: 1
    spacing: 0

    Text {
        width: 13
        height: 1
        text: root.paddedVerb
        color: root.verbColor
        bold: true
    }

    Text {
        width: Math.max(10, root.width - 13 - (root.trailing.length > 0 ? root.trailing.length + 2 : 0))
        height: 1
        text: root.detail
        color: Theme.ink
    }

    Text {
        visible: root.trailing.length > 0
        width: root.trailing.length > 0 ? root.trailing.length + 1 : 0
        height: 1
        text: root.trailing.length > 0 ? (" " + root.trailing) : ""
        color: Theme.muted
    }
}
