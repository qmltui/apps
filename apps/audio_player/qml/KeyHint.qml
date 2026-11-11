pragma ComponentBehavior: Bound
import QmlTui

// Minimal footer key hint: bold key followed by dimmed label.
Row {
    id: hint
    property string key: ""
    property string label: ""
    property bool active: false
    property string activeColor: Theme.accent
    spacing: 1

    Text {
        text: hint.key
        bold: true
        color: hint.active ? hint.activeColor : Theme.text
        dim: !hint.active
    }
    Text {
        text: hint.label
        color: hint.active ? hint.activeColor : Theme.text
        dim: !hint.active
    }
}
