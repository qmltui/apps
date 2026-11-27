pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import QmlTui.Charts as Charts
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

    Charts.Gauge {
        width: root.width
        height: 2
        thresholdMode: Charts.Gauge.Solid
        label: root.label
        unit: root.unit.length > 0 ? (" " + root.unit) : ""
        decimals: root.decimals
        min: root.from
        max: root.to
        value: root.value
        warningThreshold: root.effectiveWarn
        dangerThreshold: root.effectiveDanger
        color: HmiTheme.statusProcessing
        warningColor: HmiTheme.textWarning
        dangerColor: HmiTheme.statusError
        trackColor: HmiTheme.textDim
        labelColor: HmiTheme.textSecondary
        valueColor: root.readoutColor
        backgroundColor: HmiTheme.recessedBg
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
