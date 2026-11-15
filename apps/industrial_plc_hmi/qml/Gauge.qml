pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Column {
    id: root

    property string label: ""
    property double value: 0
    property string unit: ""
    property double from: 0
    property double to: 100
    property double warningVal: 75
    property double dangerVal: 85
    property int decimals: 1
    property var history: []
    property int sparkHeight: 1
    property color traceColor: HmiTheme.statusProcessing

    readonly property double effectiveDanger: Math.max(root.from + 0.01, Math.min(root.to, root.dangerVal))
    readonly property double effectiveWarn: Math.max(root.from + 0.005, Math.min(root.effectiveDanger - 0.005, root.warningVal))
    readonly property color readoutColor: root.value >= root.dangerVal ? HmiTheme.textDanger : (root.value >= root.effectiveWarn ? HmiTheme.textWarning : HmiTheme.textPrimary)

    width: parent ? parent.width : 22
    spacing: 0

    Item {
        width: root.width
        height: 1

        Text {
            anchors.left: parent.left
            text: root.label
            color: HmiTheme.textSecondary
            bold: true
        }

        Text {
            anchors.right: parent.right
            text: (root.decimals > 0 ? root.value.toFixed(root.decimals) : Math.round(root.value).toString()) + " " + root.unit
            color: root.readoutColor
            bold: true
        }
    }

    ProgressBar {
        width: root.width
        height: 1
        from: root.from
        to: root.to
        value: root.value
        style: "segmented"
        trackColor: HmiTheme.recessedBg
        thresholds: [
            {
                value: root.effectiveWarn,
                color: HmiTheme.statusProcessing
            },
            {
                value: root.effectiveDanger,
                color: HmiTheme.textWarning
            },
            {
                value: root.to,
                color: HmiTheme.statusError
            }
        ]
    }

    Sparkline {
        visible: root.history && root.history.length > 0
        width: root.width
        height: Math.max(1, root.sparkHeight)
        filled: root.sparkHeight > 1
        values: root.history
        from: root.from
        to: root.to
        color: root.value >= root.dangerVal ? HmiTheme.statusError : root.traceColor
        backgroundColor: HmiTheme.recessedBg
    }
}
