pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: header

    required property var theme
    property string title: "Document"
    property int currentLine: 1
    property int totalLines: 1
    property int headingCount: 0
    property int codeBlockCount: 0
    property int tableCount: 0
    property string activeThemeName: "aurora"
    property string activeStyleName: "modern"

    height: 3
    color: header.theme.surface
    border {
        style: Border.Single
        color: header.theme.border
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
            property int maxTitleLen: Math.max(10, header.width - 55)

            text: "📖 " + (header.title.length > maxTitleLen ? header.title.substring(0, maxTitleLen - 1) + "…" : header.title)
            color: header.theme.accent
            bold: true
        }

        Text {
            text: "│"
            color: header.theme.border
        }

        Text {
            text: "[H:" + header.headingCount + " C:" + header.codeBlockCount + " T:" + header.tableCount + "]"
            color: header.theme.textMuted
        }

        Text {
            text: "│"
            color: header.theme.border
        }

        Text {
            property int pct: (header.totalLines > 0) ? Math.min(100, Math.round((header.currentLine / header.totalLines) * 100)) : 0

            text: "Ln " + header.currentLine + "/" + header.totalLines + " (" + pct + "%)"
            color: header.theme.warning
            bold: true
        }

        Text {
            text: "│"
            color: header.theme.border
        }

        Text {
            text: "[" + header.activeThemeName + " : " + header.activeStyleName + "]"
            color: header.theme.accentSecondary
        }
    }
}
