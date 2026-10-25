pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import ContainerFleetApp

Terminal {
    id: root
    color: "#0d0e15" // Deep dark pastel canvas background

    // Instantiate C++ QAbstractListModel with tableData extension
    FleetListModel {
        id: fleetModel
    }

    property bool showLogs: false

    readonly property var selectedNode: fleetModel.tableData.length > 0 ? fleetModel.getNodeAt(Math.max(0, tableView.selectedRow)) : ({})
    readonly property var selectedCpuHistory: fleetModel.tableData.length > 0 ? fleetModel.getCpuHistory(Math.max(0, tableView.selectedRow)) : []
    readonly property var selectedLogs: fleetModel.tableData.length > 0 ? fleetModel.getLogs(Math.max(0, tableView.selectedRow)) : []

    // Soft Pastel Palette (Catppuccin Mocha / Nord Inspired)
    readonly property string colorHeaderBg: "#181825"
    readonly property string colorCardBg: "#14141f"
    readonly property string colorCardAltBg: "#11111b"
    readonly property string colorCardSelBg: "#272738"
    readonly property string colorTextPrimary: "#cdd6f4"
    readonly property string colorTextMuted: "#6c7086"
    readonly property string colorAccentCyan: "#89dceb"
    readonly property string colorAccentGreen: "#a6e3a1"
    readonly property string colorAccentRed: "#f38ba8"
    readonly property string colorAccentYellow: "#f9e2af"
    readonly property string colorAccentPurple: "#cba6f7"

    // MANDATORY QMLTUI RULE: Explicit key handling for Q/Escape to quit
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || (event.key === Qt.Key_Escape && searchInput.text === "")) {
            Qt.quit();
            event.accepted = true;
        } else if (event.key === Qt.Key_Escape) {
            searchInput.text = "";
            fleetModel.searchQuery = "";
            tableView.focus = true;
            event.accepted = true;
        } else if (event.key === Qt.Key_Slash) {
            searchInput.focus = true;
            event.accepted = true;
        } else if (event.key === Qt.Key_1) {
            fleetModel.activeFilter = 0;
            tableView.selectedRow = 0;
            event.accepted = true;
        } else if (event.key === Qt.Key_2) {
            fleetModel.activeFilter = 1;
            tableView.selectedRow = 0;
            event.accepted = true;
        } else if (event.key === Qt.Key_3) {
            fleetModel.activeFilter = 2;
            tableView.selectedRow = 0;
            event.accepted = true;
        } else if (event.key === Qt.Key_R) {
            fleetModel.toggleNodeStatus(tableView.selectedRow);
            event.accepted = true;
        } else if (event.key === Qt.Key_A) {
            fleetModel.restartAllCritical();
            event.accepted = true;
        } else if (event.key === Qt.Key_L) {
            root.showLogs = !root.showLogs;
            event.accepted = true;
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        // --- 1. UNIFIED SLEEK TOP CONTROL HEADER ---
        Panel {
            width: parent.width
            height: 3
            color: root.colorHeaderBg
            border.color: "#272738"
            title: Text {
                text: "⚡ DOCKER FLEET MONITOR  |  Total: " + fleetModel.totalNodes + "  Running: " + fleetModel.runningNodes + "  Critical: " + fleetModel.alertNodes
                color: root.colorAccentCyan
            }

            Row {
                x: 1
                y: 1
                width: parent.width - 2
                spacing: 2

                // Left: Borderless Filter Tabs
                TabBar {
                    id: filterTabs
                    width: Math.max(46, parent.width - 32)
                    currentIndex: fleetModel.activeFilter
                    tabs: ["1:ALL(" + fleetModel.totalNodes + ")", "2:RUN(" + fleetModel.runningNodes + ")", "3:CRIT(" + fleetModel.alertNodes + ")"]
                    onCurrentIndexChanged: {
                        if (fleetModel.activeFilter !== filterTabs.currentIndex) {
                            fleetModel.activeFilter = filterTabs.currentIndex;
                            tableView.selectedRow = 0;
                        }
                    }
                }

                // Right: Clean Flat Search Input
                Row {
                    spacing: 1

                    Text {
                        text: "[/] Search:"
                        color: searchInput.focus ? root.colorAccentYellow : root.colorTextMuted
                        bold: searchInput.focus
                    }

                    Rectangle {
                        width: 16
                        height: 1
                        border.style: Border.None
                        color: searchInput.focus ? "#2a2b3d" : "#14141f"

                        TextInput {
                            id: searchInput
                            width: 14
                            text: ""
                            color: root.colorTextPrimary
                            onTextChanged: fleetModel.searchQuery = searchInput.text
                            onAccepted: tableView.focus = true
                        }
                    }
                }
            }
        }

        // --- 2. NATIVE TABLEVIEW WITH HEADER SORTING ---
        Rectangle {
            width: parent.width
            height: Math.max(4, parent.height - (root.showLogs ? 16 : 10))
            border.style: Border.Single
            border.color: "#272738"

            TableView {
                id: tableView
                focus: true
                selectedRow: 0
                anchors.fill: parent
                anchors.margins: 1
                showColumnGrid: true
                headerColor: root.colorAccentCyan
                headerBackgroundColor: "#181825"
                selectionColor: root.colorAccentYellow
                selectionBackgroundColor: root.colorCardSelBg
                alternatingRows: true
                alternatingRowBackgroundColor: root.colorCardAltBg
                rowBackgroundColor: root.colorCardBg

                columns: [
                    {
                        title: "NODE NAME",
                        width: 19,
                        align: "left"
                    },
                    {
                        title: "IP ADDRESS",
                        width: 14,
                        align: "left"
                    },
                    {
                        title: "STATUS",
                        width: 12,
                        align: "center"
                    },
                    {
                        title: "CPU LOAD",
                        width: 17,
                        align: "left"
                    },
                    {
                        title: "MEMORY",
                        width: 17,
                        align: "left"
                    },
                    {
                        title: "TAGS & SERVICES",
                        width: 22,
                        align: "left"
                    }
                ]

                model: fleetModel.tableData

                cellFormat: (row, col, value) => {
                    if (col === 2) {
                        if (value === "RUNNING")
                            return {
                                text: "✔ RUNNING",
                                color: root.colorAccentGreen,
                                bold: true
                            };
                        if (value === "CRITICAL")
                            return {
                                text: "✖ CRITICAL",
                                color: root.colorAccentRed,
                                bold: true
                            };
                        if (value === "STANDBY")
                            return {
                                text: "⏸ STANDBY",
                                color: root.colorAccentYellow,
                                bold: true
                            };
                    } else if (col === 3) {
                        let isHigh = value.indexOf("80%") !== -1 || value.indexOf("90%") !== -1 || value.indexOf("100%") !== -1;
                        return {
                            text: value,
                            color: isHigh ? root.colorAccentRed : root.colorAccentCyan,
                            bold: isHigh
                        };
                    } else if (col === 4) {
                        let isHigh = value.indexOf("80%") !== -1 || value.indexOf("90%") !== -1 || value.indexOf("100%") !== -1;
                        return {
                            text: value,
                            color: isHigh ? root.colorAccentRed : root.colorAccentGreen,
                            bold: isHigh
                        };
                    } else if (col === 5) {
                        return {
                            text: value,
                            color: root.colorAccentPurple
                        };
                    }
                    return {
                        text: value
                    };
                }
            }
        }

        // --- 3. BORDERED INSPECTOR DRAWER PANEL WITH SPARKLINE ---
        Panel {
            width: parent.width
            height: 4
            color: "#181825"
            border.color: "#272738"
            title: Text {
                text: "🔍 NODE INSPECTOR & REAL-TIME TELEMETRY"
                color: root.colorAccentCyan
            }

            Column {
                x: 1
                y: 1
                spacing: 0

                Row {
                    spacing: 1
                    Text {
                        text: root.selectedNode && root.selectedNode.name ? root.selectedNode.name + " (" + root.selectedNode.id + ")" : "Node Selection Active"
                        color: root.colorTextPrimary
                        bold: true
                    }
                    Text {
                        text: " | CPU Sparkline: ["
                        color: root.colorAccentYellow
                        bold: true
                    }
                    Sparkline {
                        width: 16
                        height: 1
                        from: 1
                        to: 10
                        values: root.selectedCpuHistory
                        color: root.colorAccentYellow
                    }
                    Text {
                        text: "]"
                        color: root.colorAccentYellow
                        bold: true
                    }
                }
                Text {
                    text: root.selectedNode && root.selectedNode.name ? "Status: " + root.selectedNode.status + " | IP: " + root.selectedNode.ip + " | CPU Load: " + (root.selectedNode.cpu * 10) + "%" + " | Memory: " + (root.selectedNode.mem * 10) + "%" : ""
                    color: root.colorTextMuted
                }
            }
        }

        // --- 4. OPTIONAL LIVE DOCKER LOG DRAWER ---
        Panel {
            visible: root.showLogs
            width: parent.width
            height: 6
            color: "#11111b"
            border.color: "#272738"
            title: Text {
                text: "📜 LIVE CONTAINER LOG STREAM (" + (root.selectedNode && root.selectedNode.name ? root.selectedNode.name : "") + ")"
                color: root.colorAccentPurple
            }

            Column {
                x: 1
                y: 1
                spacing: 0

                Repeater {
                    model: root.selectedLogs
                    delegate: Text {
                        required property string modelData
                        text: "  > " + modelData
                        color: modelData.indexOf("ALERT") !== -1 ? root.colorAccentRed : modelData.indexOf("WARN") !== -1 ? root.colorAccentYellow : root.colorTextMuted
                    }
                }
            }
        }

        // --- 5. COMPLETE FOOTER SHORTCUT LEGEND ---
        Panel {
            width: parent.width
            height: 3
            color: "#0d1c2d"
            border.color: "#273647"
            title: Text {
                text: "SHORTCUTS & COMMANDS"
                color: root.colorAccentGreen
            }

            Row {
                x: 1
                y: 1
                spacing: 2
                Text {
                    text: "[1-3]Filter [/]Search [Headers]Sort [R]Restart [A]FixAll [L]Logs [Q/Esc]Quit"
                    color: root.colorAccentGreen
                    bold: true
                }
            }
        }
    }
}
