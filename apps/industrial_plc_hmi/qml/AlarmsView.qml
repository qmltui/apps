pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property var eventRows: []
    property int errorCount: 0
    property int machineState: PlcTypes.Idle

    readonly property bool wide: width >= 92
    readonly property bool showKpi: height >= 19

    function focusAuditTable() {
        auditTable.forceActiveFocus();
    }

    anchors.fill: parent
    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

    onVisibleChanged: {
        if (root.visible)
            auditTable.forceActiveFocus();
    }

    title: Text {
        text: "◆ ALARM HISTORY & AUDIT LOG (" + root.errorCount + " FAULTS RECORDED)"
        color: root.errorCount > 0 ? HmiTheme.textDanger : HmiTheme.textHighlight
        bold: true
    }

    titleTrailing: Text {
        visible: root.wide
        text: (root.eventRows ? root.eventRows.length : 0) + " EVENTS"
        color: HmiTheme.textSecondary
    }

    footer: Text {
        text: "↑↓ Select Audit Entry"
        color: HmiTheme.textDim
    }

    footerTrailing: Text {
        text: "Press [R] to clear fault lockout"
        color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textWarning
        bold: root.machineState === PlcTypes.Error
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // Summary KPI Strip on roomy viewports
        Row {
            id: kpiRow
            visible: root.showKpi
            width: parent.width
            height: 3
            spacing: 1

            Panel {
                width: Math.floor((parent.width - 2) / 3)
                height: 3
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle
                title: Text {
                    text: "AUDIT BUFFER"
                    color: HmiTheme.textDim
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    text: (root.eventRows ? root.eventRows.length : 0) + " / 100 RECORDS"
                    color: HmiTheme.textPrimary
                    bold: true
                }
            }

            Panel {
                width: Math.floor((parent.width - 2) / 3)
                height: 3
                color: root.errorCount > 0 ? HmiTheme.headerBgError : HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: root.errorCount > 0 ? HmiTheme.borderError : HmiTheme.borderSubtle
                title: Text {
                    text: "FAULT COUNTER"
                    color: root.errorCount > 0 ? HmiTheme.textDanger : HmiTheme.textDim
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    text: root.errorCount + (root.errorCount === 1 ? " TRIP LOGGED" : " TRIPS LOGGED")
                    color: root.errorCount > 0 ? HmiTheme.textDanger : HmiTheme.textSuccess
                    bold: true
                }
            }

            Panel {
                width: parent.width - Math.floor((parent.width - 2) / 3) * 2 - 2
                height: 3
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle
                title: Text {
                    text: "SAFETY RELAY"
                    color: HmiTheme.textDim
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    text: root.machineState === PlcTypes.Error ? "▲ LOCKED OUT" : "● ARMED & READY"
                    color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
                    bold: true
                }
            }
        }

        TableView {
            id: auditTable
            focus: true
            width: parent.width
            height: Math.max(4, parent.height - (root.showKpi ? 4 : 0))
            showColumnGrid: true
            showRowGrid: false
            showOuterBorder: false
            alternatingRows: true
            stretchLastColumn: true
            selectionKeepsCellColors: true
            selectedRow: 0
            gridColor: HmiTheme.borderSubtle
            headerColor: HmiTheme.textHighlight
            headerBackgroundColor: HmiTheme.recessedBg
            textColor: HmiTheme.textPrimary
            rowBackgroundColor: HmiTheme.panelBg
            alternatingRowBackgroundColor: HmiTheme.panelAltBg
            selectionColor: HmiTheme.selectionBg
            selectedTextColor: HmiTheme.textPrimary

            columns: [
                {
                    title: "SEQ",
                    width: 6,
                    align: "right",
                    padding: 1
                },
                {
                    title: "TIME",
                    width: root.wide ? 10 : 0,
                    align: "center",
                    padding: 1
                },
                {
                    title: "CODE",
                    width: root.wide ? 10 : 0,
                    align: "center",
                    padding: 1
                },
                {
                    title: "SEVERITY",
                    width: 12,
                    align: "center",
                    padding: 1
                },
                {
                    title: "EVENT MESSAGE / SAFETY TRIGGER",
                    width: 32,
                    align: "left",
                    padding: 1
                }
            ]

            model: root.eventRows

            cellFormat: (row, column, value) => {
                const rowData = root.eventRows && row >= 0 && row < root.eventRows.length ? root.eventRows[row] : null;
                const sev = rowData ? rowData[3] : "";
                const isCrit = sev === "[CRITICAL]";
                const isWarn = sev === "[WARNING]";

                if (column === 0 || column === 1) {
                    return {
                        text: value,
                        color: HmiTheme.textDim
                    };
                }
                if (column === 2) {
                    return {
                        text: value,
                        color: isCrit ? HmiTheme.textDanger : (isWarn ? HmiTheme.textWarning : HmiTheme.textSecondary)
                    };
                }
                if (column === 3) {
                    return {
                        text: value,
                        color: isCrit ? HmiTheme.textDanger : (isWarn ? HmiTheme.textWarning : HmiTheme.textSuccess),
                        bold: isCrit || isWarn
                    };
                }
                return {
                    text: value,
                    color: isCrit ? HmiTheme.textDanger : (isWarn ? HmiTheme.textWarning : HmiTheme.textPrimary),
                    bold: isCrit
                };
            }
        }
    }
}
