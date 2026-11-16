pragma ComponentBehavior: Bound
import Qt.labs.qmlmodels
import QmlTui

Terminal {
    id: root

    readonly property bool isPrompt: engine.phase === PackageEngine.PromptPhase
    readonly property bool isRunning: engine.phase === PackageEngine.FetchingPhase || engine.phase === PackageEngine.BuildingPhase
    readonly property bool isFinished: engine.phase === PackageEngine.FinishedPhase

    mode: Terminal.Inline
    focus: true
    mouseReporting: true
    title: {
        if (root.isFinished)
            return "qpm install (finished)";
        if (root.isPrompt)
            return "qpm install " + engine.targetName + " (confirm plan)";
        return "qpm install " + engine.targetName + " (" + Math.round(engine.overallProgress * 100) + "%)";
    }
    progress: engine.overallProgress
    progressState: root.isRunning ? (engine.paused ? Terminal.ProgressWarning : Terminal.ProgressNormal) : Terminal.ProgressNone

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || (root.isPrompt && event.key === Qt.Key_N) || event.key === Qt.Key_Escape || ((event.modifiers & Qt.ControlModifier) && (event.key === Qt.Key_C || event.key === Qt.Key_Q))) {
            Qt.quit();
            event.accepted = true;
            return;
        }

        if (root.isPrompt) {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Y) {
                engine.confirmInstall();
                event.accepted = true;
            } else if (event.key === Qt.Key_1) {
                engine.toggleFeature(0);
                event.accepted = true;
            } else if (event.key === Qt.Key_2) {
                engine.toggleFeature(1);
                event.accepted = true;
            } else if (event.key === Qt.Key_3) {
                engine.toggleFeature(2);
                event.accepted = true;
            } else if (event.key === Qt.Key_D) {
                engine.toggleTreePreview();
                event.accepted = true;
            }
            return;
        }

        if (root.isRunning) {
            if (event.key === Qt.Key_V) {
                engine.toggleVerboseLog();
                event.accepted = true;
            } else if (event.key === Qt.Key_P || event.key === Qt.Key_Space) {
                engine.togglePause();
                event.accepted = true;
            }
            return;
        }

        if (root.isFinished && event.key === Qt.Key_R) {
            engine.restartSession();
            event.accepted = true;
        }
    }

    Component.onCompleted: {
        engine.initializeFromArgs(Qt.application.arguments);
    }

    PackageEngine {
        id: engine
        onFinishedAndAutoExit: Qt.quit()
    }

    Scrollback {
        id: history
        model: engine.feedModel
        spacing: 0

        delegate: DelegateChooser {
            role: "entryKind"

            DelegateChoice {
                roleValue: ScrollbackFeedModel.BannerEntry
                BannerCard {}
            }

            DelegateChoice {
                roleValue: ScrollbackFeedModel.StepEntry
                StepLogRow {}
            }

            DelegateChoice {
                roleValue: ScrollbackFeedModel.PlanLockEntry
                SummaryPanelCard {
                    badgeIcon: "⚡"
                }
            }

            DelegateChoice {
                roleValue: ScrollbackFeedModel.DiagnosticEntry
                SummaryPanelCard {
                    badgeIcon: "ℹ"
                }
            }

            DelegateChoice {
                roleValue: ScrollbackFeedModel.ReceiptEntry
                SummaryPanelCard {
                    badgeIcon: "✓"
                }
            }
        }
    }

    TransactionPromptView {
        visible: root.isPrompt
        width: parent.width
        engine: engine
    }

    LiveExecutionPanel {
        visible: root.isRunning
        width: parent.width
        engine: engine
    }

    Text {
        visible: root.isFinished
        width: parent.width
        height: 1
        spans: [
            {
                text: "   ✓ Summary  ",
                color: Theme.green,
                bold: true
            },
            {
                text: "Successfully installed " + engine.totalPackages + " packages (+" + engine.totalInstallText + ") in " + engine.elapsedTotalText + "  ",
                color: Theme.ink
            },
            {
                text: "[r] Re-run · [q/Esc] Exit",
                color: Theme.muted
            }
        ]
    }

    Timer {
        interval: 50
        repeat: true
        running: root.isRunning && !engine.paused
        onTriggered: engine.tick(50)
    }
}
