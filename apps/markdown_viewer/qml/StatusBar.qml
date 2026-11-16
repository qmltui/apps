pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: status

    required property var theme
    property string statusMessage: ""
    property string activeSearchInfo: ""

    height: 3
    color: status.theme.surface
    border {
        style: Border.Single
        color: status.theme.border
    }

    Row {
        anchors {
            fill: parent
            leftMargin: 1
            rightMargin: 1
        }
        spacing: 2
        clip: true

        Text {
            text: status.statusMessage.length > 0 ? status.statusMessage : (status.activeSearchInfo.length > 0 ? status.activeSearchInfo : "Ready")
            color: status.statusMessage.length > 0 ? status.theme.success : (status.activeSearchInfo.length > 0 ? status.theme.warning : status.theme.textPrimary)
            bold: true
        }

        Text {
            text: "│"
            color: status.theme.border
        }

        Text {
            text: "[Tab] Section  [j/k] Scroll  [s] Style  [e] Editor  [l] Lines  [/] Find  [t] Theme  [o] Sample  [q] Quit"
            color: status.theme.textMuted
        }
    }
}
