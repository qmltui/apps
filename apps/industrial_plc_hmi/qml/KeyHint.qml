pragma ComponentBehavior: Bound

import QmlTui
import IndustrialPlcHmi

Item {
    id: root

    property string key: ""
    property string label: ""
    property bool active: false
    property color activeColor: HmiTheme.textHighlight

    signal activated

    implicitWidth: contentRow.implicitWidth
    implicitHeight: 1

    Row {
        id: contentRow
        spacing: 1

        Text {
            text: root.key
            bold: true
            color: root.active ? root.activeColor : HmiTheme.textPrimary
        }

        Text {
            text: root.label
            color: root.active ? root.activeColor : HmiTheme.textSecondary
            dim: !root.active
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.activated()
    }
}
