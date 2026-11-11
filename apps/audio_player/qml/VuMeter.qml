pragma ComponentBehavior: Bound
import QmlTui
import QmlTui.Controls

// Single-row compact VU channel: "L  ━━━━━━····  -6.0 dB".
Row {
    id: meter
    property string label: "L"
    property real level: 0.0             // 0..1, full-scale sine = 1.0
    property string accentColor: Theme.vuLow
    property string labelColor: Theme.subtle
    property int meterWidth: 24

    readonly property string dbText: Theme.formatDecibels(meter.level).padStart(7, " ")
    readonly property int barCols: Math.max(4, meterWidth - dbText.length - 3)

    width: meterWidth
    height: 1
    spacing: 1

    Text {
        text: meter.label
        bold: true
        color: meter.labelColor
    }
    ProgressBar {
        width: meter.barCols
        value: meter.level * 100
        from: 0
        to: 100
        fillCharacter: "━"
        emptyCharacter: "·"
        color: Theme.muted
        thresholds: [
            { value: 65, color: meter.accentColor },
            { value: 85, color: Theme.warning },
            { value: 100, color: Theme.hot }
        ]
    }
    Text {
        text: meter.dbText
        color: meter.level > 0.85 ? Theme.hot : Theme.text
        dim: meter.level <= 0.85
    }
}
