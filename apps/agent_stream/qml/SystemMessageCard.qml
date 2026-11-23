pragma ComponentBehavior: Bound

import QmlTui
import QmlTui.Controls

Column {
    id: sysCard

    required property string title
    required property string content

    readonly property bool showMetrics: {
        const lower = sysCard.title.toLowerCase();
        return lower.indexOf("cost") !== -1 || lower.indexOf("usage") !== -1;
    }

    width: parent ? parent.width : 80
    spacing: 0

    Text {
        visible: sysCard.title.length > 0
        text: "✦ " + sysCard.title
        color: Theme.cyan
        bold: true
    }

    Text {
        x: 2
        text: sysCard.content
        color: Theme.slateLight
    }

    // Context capacity bar if it's a cost/metrics card
    Row {
        visible: sysCard.showMetrics
        spacing: 1

        Text {
            text: "  • Context Window:  "
            color: Theme.slateLight
        }
        ProgressBar {
            value: 0.18
            from: 0.0
            to: 1.0
            style: "segmented"
            color: Theme.green
            trackColor: Theme.pressedBg
            width: 14
        }
        Text {
            text: " 23.4k / 128k (18%)"
            color: Theme.slate
        }
    }

    // Live latency sparkline if it's a cost/metrics card
    Row {
        visible: sysCard.showMetrics
        spacing: 1

        Text {
            text: "  • Peak Rate:       "
            color: Theme.slateLight
        }
        Sparkline {
            values: [14, 22, 28, 35, 42, 58, 62, 54, 70, 85, 92, 88]
            color: Theme.cyan
            width: 12
            height: 1
        }
        Text {
            text: " 92 tok/s (avg: 64 tok/s)"
            color: Theme.slate
        }
    }

    // Multi-line Throughput Stream Waveform
    Column {
        visible: sysCard.showMetrics
        width: parent.width
        spacing: 0

        Text {
            text: "  • Throughput Stream Waveform (10s window):"
            color: Theme.slateLight
        }
        Text {
            text: "    120 ┤          ⡠⠔⠊⠉⠒⢄        ⡠⠊⠑⢄\n     80 ┤     ⡠⠒⠁        ⠑⢄    ⡠⠊   ⠑⢄\n     40 ┤⢀⠤⠊⠁             ⠑⠢⠔⠊      ⠑⠤⢄"
            color: Theme.cyan
        }
        Text {
            text: "      0 └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴─ (t)"
            color: Theme.muted
        }
    }
}
