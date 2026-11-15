pragma ComponentBehavior: Bound
import QmlTui
import QmlTui.Controls

Panel {
    id: root

    required property string cardTitle
    required property string cardSubtitle
    required property string line1
    required property string line2
    required property string line3
    required property string accentColor
    property string badgeIcon: "✓"
    property int streamSpacing: 1

    readonly property bool hasLine3: line3.length > 0

    width: parent ? Math.min(parent.width, 104) : 96
    height: hasLine3 ? 5 : 4
    color: "transparent"
    border.style: Border.Rounded
    border.color: root.accentColor

    title: Text {
        spans: [
            {
                text: root.badgeIcon + " ",
                color: root.accentColor,
                bold: true
            },
            {
                text: root.cardTitle,
                color: Theme.ink,
                bold: true
            }
        ]
    }

    titleTrailing: Text {
        visible: root.cardSubtitle.length > 0
        text: root.cardSubtitle
        color: root.accentColor
        bold: true
    }

    Column {
        anchors.fill: parent
        spacing: 0

        Text {
            width: parent.width
            height: 1
            text: " " + root.line1
            color: Theme.secondary
        }

        Text {
            width: parent.width
            height: 1
            text: " " + root.line2
            color: Theme.muted
        }

        Text {
            visible: root.hasLine3
            width: parent.width
            height: 1
            text: " " + root.line3
            color: Theme.ink
            bold: true
        }
    }
}
