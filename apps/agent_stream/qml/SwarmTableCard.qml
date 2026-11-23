pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: swarmCard

    required property string title

    width: parent ? parent.width : 76
    spacing: 0

    SwarmAuditModel {
        id: swarmAuditModel
    }

    Row {
        spacing: 1

        Text {
            text: "✦ Subagent Swarm Mesh:"
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " " + (swarmCard.title || "Audit Results")
            color: "white"
            bold: true
        }
    }

    TableView {
        width: parent.width > 0 ? Math.min(parent.width, 76) : 76
        height: 5
        focus: false
        focusPolicy: Qt.NoFocus
        headerColor: Theme.cyan
        headerBackgroundColor: "transparent"
        showColumnGrid: false
        alternatingRows: false

        columns: [
            {
                title: "Subagent",
                width: 18,
                align: "left"
            },
            {
                title: "Scope",
                width: 20,
                align: "left"
            },
            {
                title: "Status",
                width: 10,
                align: "center"
            },
            {
                title: "Findings",
                width: 26,
                align: "left"
            }
        ]

        model: swarmAuditModel

        cellFormat: (row, col, value) => {
            if (col === 0)
                return {
                    text: value,
                    color: "#a78bfa",
                    bold: true
                };
            if (col === 1)
                return {
                    text: value,
                    color: "#94a3b8"
                };
            if (col === 2)
                return {
                    text: "✔ " + value,
                    color: "#34d399",
                    bold: true
                };
            if (col === 3)
                return {
                    text: value,
                    color: "#e2e8f0"
                };
            return {
                text: value
            };
        }
    }
}
