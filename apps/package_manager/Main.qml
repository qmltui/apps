pragma ComponentBehavior: Bound
import QtQml
import QmlTui

Terminal {
    id: root

    property string targetName: "qpm-cli"
    property string targetVersion: "0.5.0"
    property int phase: 0 // 0: Fetching, 1: Building, 2: Finished
    property int currentIndex: 0
    property int currentPkgProgressKiB: 0
    property int completedKiB: 0
    property int elapsedMs: 0
    property int fetchFinishedMs: 0
    property bool autoExit: false

    readonly property int tickIntervalMs: 50
    readonly property int downloadRateKiBPerTick: 1800
    readonly property int compileRateKiBPerTick: 1500

    readonly property var packages: [
        {
            name: "qmltui-ptyview",
            version: "0.5.0",
            sizeKiB: 1945
        },
        {
            name: "md4qt",
            version: "2.8.0",
            sizeKiB: 2252
        },
        {
            name: "qt6-lottie",
            version: "6.8.3",
            sizeKiB: 3174
        },
        {
            name: "qcoro-core",
            version: "0.11.0",
            sizeKiB: 3686
        },
        {
            name: "qmltui-controls",
            version: "0.5.0",
            sizeKiB: 3891
        },
        {
            name: "qmltui-core",
            version: "0.5.0",
            sizeKiB: 4300
        },
        {
            name: "qt6-shadertools",
            version: "6.8.3",
            sizeKiB: 9625
        },
        {
            name: "kdab-kuesa",
            version: "2.4.1",
            sizeKiB: 14950
        }
    ]

    readonly property int totalSizeKiB: {
        let sum = 0;
        for (let i = 0; i < packages.length; ++i)
            sum += packages[i].sizeKiB;
        return sum;
    }

    readonly property real phaseFraction: {
        if (totalSizeKiB <= 0)
            return 0.0;
        return Math.max(0.0, Math.min(1.0, (completedKiB + currentPkgProgressKiB) / totalSizeKiB));
    }

    readonly property real overallProgress: {
        if (phase === 2)
            return 1.0;
        if (phase === 0)
            return phaseFraction * 0.5;
        return 0.5 + (phaseFraction * 0.5);
    }

    function formatSize(kiB) {
        if (kiB >= 1024)
            return (kiB / 1024.0).toFixed(1) + " MiB";
        return kiB + " KiB";
    }

    function formatSeconds(ms) {
        return (ms / 1000.0).toFixed(1) + "s";
    }

    function printStep(verb, detail, verbColor) {
        const paddedVerb = " ".repeat(Math.max(0, 12 - verb.length)) + verb + " ";
        history.print([
            {
                text: paddedVerb,
                color: verbColor || "green",
                bold: true
            },
            {
                text: detail
            }
        ]);
    }

    function cargoBar(fraction, width) {
        const clamped = Math.max(0.0, Math.min(1.0, fraction));
        const filled = Math.round(clamped * width);
        if (filled <= 0)
            return "[" + " ".repeat(width) + "]";
        if (filled >= width)
            return "[" + "=".repeat(width) + "]";
        return "[" + "=".repeat(filled - 1) + ">" + " ".repeat(width - filled) + "]";
    }

    function activeCratesText() {
        if (phase === 2 || packages.length === 0)
            return "";
        const idx = Math.min(packages.length - 1, currentIndex);
        const first = packages[idx].name;
        if (idx + 1 < packages.length)
            return first + ", " + packages[idx + 1].name;
        return first;
    }

    mode: Terminal.Inline
    focus: true
    title: root.phase === 2 ? "qpm install (finished)" : ("qpm install " + root.targetName)
    progress: root.overallProgress
    progressState: root.phase === 2 ? Terminal.ProgressNone : Terminal.ProgressNormal

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || event.key === Qt.Key_Escape || ((event.modifiers & Qt.ControlModifier) && (event.key === Qt.Key_C || event.key === Qt.Key_Q))) {
            Qt.quit();
            event.accepted = true;
        }
    }

    Scrollback {
        id: history
    }

    Text {
        width: parent.width
        height: 1
        spans: root.phase === 2 ? [
            {
                text: "     Summary ",
                color: "green",
                bold: true
            },
            {
                text: "Successfully installed " + root.packages.length + " packages (" + root.formatSize(root.totalSizeKiB) + ") in " + root.formatSeconds(root.elapsedMs) + " "
            },
            {
                text: "(press q or Esc to exit)",
                dim: true
            }
        ] : [
            {
                text: root.phase === 0 ? "    Fetching " : "    Building ",
                color: "cyan",
                bold: true
            },
            {
                text: root.cargoBar(root.phaseFraction, 24) + " ",
                bold: true
            },
            {
                text: (root.currentIndex + 1) + "/" + root.packages.length + " (" + root.formatSize(root.completedKiB + root.currentPkgProgressKiB) + "/" + root.formatSize(root.totalSizeKiB) + "): " + root.activeCratesText()
            }
        ]
    }

    Timer {
        id: stepTimer
        interval: root.tickIntervalMs
        repeat: true
        running: false

        onTriggered: {
            root.elapsedMs += root.tickIntervalMs;
            const count = root.packages.length;
            if (root.currentIndex >= count)
                return;

            const pkg = root.packages[root.currentIndex];
            const rate = root.phase === 0 ? root.downloadRateKiBPerTick : root.compileRateKiBPerTick;
            root.currentPkgProgressKiB = Math.min(pkg.sizeKiB, root.currentPkgProgressKiB + rate);

            if (root.currentPkgProgressKiB >= pkg.sizeKiB) {
                root.completedKiB += pkg.sizeKiB;
                root.currentPkgProgressKiB = 0;

                if (root.phase === 0) {
                    root.printStep("Downloaded", pkg.name + " v" + pkg.version + " (" + root.formatSize(pkg.sizeKiB) + ")", "green");
                } else {
                    root.printStep("Compiling", pkg.name + " v" + pkg.version, "green");
                }

                root.currentIndex += 1;

                if (root.currentIndex >= count) {
                    if (root.phase === 0) {
                        root.fetchFinishedMs = root.elapsedMs;
                        root.printStep("Downloaded", count + " packages (" + root.formatSize(root.totalSizeKiB) + ") in " + root.formatSeconds(root.fetchFinishedMs), "green");
                        root.phase = 1;
                        root.currentIndex = 0;
                        root.completedKiB = 0;
                    } else {
                        root.printStep("Compiling", root.targetName + " v" + root.targetVersion, "green");
                        root.printStep("Finished", "`release` profile [optimized] target(s) in " + root.formatSeconds(root.elapsedMs), "green");
                        root.printStep("Installed", "~/.qpm/bin/" + root.targetName + " (" + count + " packages, " + root.formatSize(root.totalSizeKiB) + ")", "green");
                        stepTimer.stop();
                        root.phase = 2;
                        if (root.autoExit)
                            Qt.quit();
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        const args = Qt.application.arguments;
        for (let i = 1; i < args.length; ++i) {
            const arg = args[i];
            if (arg === "--auto-exit")
                root.autoExit = true;
            else if (!arg.startsWith("-") && arg !== "install")
                root.targetName = arg;
        }
        root.printStep("Installing", root.targetName + " v" + root.targetVersion + " (~/.qpm/bin)", "green");
        root.printStep("Updating", "qt-registry.io index", "green");
        stepTimer.start();
    }
}
