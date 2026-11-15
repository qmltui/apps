pragma ComponentBehavior: Bound
import QmlTui
import QmlTui.Controls

Panel {
    id: root

    required property PackageEngine engine

    readonly property bool isFetching: engine.phase === PackageEngine.FetchingPhase
    readonly property color phaseAccent: isFetching ? Theme.cyan : Theme.blue
    readonly property string phaseHeaderText: {
        const prefix = isFetching ? "Fetching Packages " : "Building Release Graph ";
        const counts = "(" + engine.completedPackages + "/" + engine.totalPackages + ")";
        return engine.paused ? (prefix + counts + " [PAUSED]") : (prefix + counts);
    }

    width: parent ? Math.min(parent.width, 104) : 96
    height: 8 + (engine.showVerboseLog ? 5 : 0)
    color: "transparent"
    border.style: Border.Rounded
    border.color: root.phaseAccent

    title: Row {
        width: 2 + phaseLabel.implicitWidth
        height: 1
        spacing: 1

        BusyIndicator {
            width: 1
            height: 1
            running: !root.engine.paused
            interval: 90
            color: root.phaseAccent
        }

        Text {
            id: phaseLabel
            height: 1
            text: root.phaseHeaderText
            color: Theme.ink
            bold: true
        }
    }

    titleTrailing: Row {
        width: rateLabel.implicitWidth + 13
        height: 1
        spacing: 1

        Text {
            id: rateLabel
            height: 1
            text: root.engine.throughputText
            color: root.isFetching ? Theme.cyan : Theme.amber
            bold: true
        }

        Sparkline {
            width: 12
            height: 1
            values: root.engine.throughputHistory
            from: 0
            to: root.isFetching ? 65 : 45
            color: root.isFetching ? Theme.cyan : Theme.green
        }
    }

    footer: Text {
        spans: [
            {
                text: "[v]",
                color: Theme.cyan,
                bold: true
            },
            {
                text: root.engine.showVerboseLog ? " Hide Stream  " : " Verbose Stream  ",
                color: Theme.secondary
            },
            {
                text: "[p]",
                color: Theme.amber,
                bold: true
            },
            {
                text: root.engine.paused ? " Resume  " : " Pause  ",
                color: Theme.secondary
            },
            {
                text: "[q/Esc]",
                color: Theme.muted,
                bold: true
            },
            {
                text: " Abort",
                color: Theme.muted
            }
        ]
    }

    Column {
        id: contentCol

        anchors.fill: parent
        spacing: 0

        Row {
            width: parent.width
            height: 1
            spacing: 1

            Text {
                width: 9
                height: 1
                text: " Overall"
                color: Theme.secondary
                bold: true
            }

            ProgressBar {
                width: Math.max(18, contentCol.width - 47)
                height: 1
                value: root.engine.overallProgress
                showPercentage: true
                style: "segmented"
                color: Theme.ink
                trackColor: "#1e293b"
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "#38bdf8"
                    }
                    GradientStop {
                        position: 0.55
                        color: "#60a5fa"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#34d399"
                    }
                }
            }

            Text {
                width: 36
                height: 1
                text: " " + root.engine.progressSummaryText + " · ETA " + root.engine.etaText
                color: Theme.secondary
            }
        }

        Text {
            width: parent.width
            height: 1
            text: "─".repeat(Math.max(0, parent.width))
            color: Theme.border
        }

        Repeater {
            model: root.engine.workerModel

            Row {
                id: workerRow

                required property int slotNumber
                required property bool busy
                required property string stageLabel
                required property string stageColor
                required property string packageLabel
                required property string stepDetail
                required property real progress

                readonly property string paddedStage: "[" + stageLabel + " ".repeat(Math.max(0, 6 - stageLabel.length)) + "]"
                readonly property int pkgColWidth: Math.max(16, contentCol.width - 50)

                width: contentCol.width
                height: 1
                spacing: 1

                Text {
                    width: 3
                    height: 1
                    text: " #" + workerRow.slotNumber
                    color: Theme.muted
                }

                BusyIndicator {
                    visible: workerRow.busy
                    width: 1
                    height: 1
                    running: workerRow.busy && !root.engine.paused
                    interval: 90
                    color: workerRow.stageColor
                }

                Text {
                    visible: !workerRow.busy
                    width: 1
                    height: 1
                    text: "·"
                    color: Theme.muted
                }

                Text {
                    width: 8
                    height: 1
                    text: workerRow.paddedStage
                    color: workerRow.stageColor
                    bold: workerRow.busy
                }

                Text {
                    width: workerRow.pkgColWidth
                    height: 1
                    text: workerRow.packageLabel
                    color: workerRow.busy ? Theme.ink : Theme.muted
                    bold: workerRow.busy
                }

                ProgressBar {
                    width: 16
                    height: 1
                    value: workerRow.progress
                    style: "line"
                    color: workerRow.busy ? workerRow.stageColor : Theme.border
                    trackColor: Theme.border
                }

                Text {
                    width: 17
                    height: 1
                    text: workerRow.stepDetail
                    color: workerRow.busy ? Theme.secondary : Theme.muted
                }
            }
        }

        Text {
            visible: root.engine.showVerboseLog
            width: contentCol.width
            height: 1
            text: {
                const label = "── LIVE STREAM ";
                const rem = Math.max(0, contentCol.width - label.length);
                return label + "─".repeat(rem);
            }
            color: Theme.border
        }

        Repeater {
            model: root.engine.showVerboseLog ? 4 : 0

            Text {
                required property int index

                readonly property string logLine: index < root.engine.liveLogLines.length ? root.engine.liveLogLines[index] : ""

                width: contentCol.width
                height: 1
                text: " " + logLine
                color: index === root.engine.liveLogLines.length - 1 ? Theme.secondary : Theme.muted
            }
        }
    }
}
