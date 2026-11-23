pragma ComponentBehavior: Bound

import Qt.labs.qmlmodels
import QmlTui

Terminal {
    id: root

    property int historyIndex: -1

    readonly property bool modalActive: (modelModal && modelModal.visible) || (diffInspector && diffInspector.visible)
    readonly property list<string> spinnerFrames: ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]

    function openModelPicker(): void {
        if (diffInspector.visible)
            diffInspector.close();
        if (modelModal.visible)
            modelModal.close();
        else
            modelModal.open();
    }

    function openDiffInspector(): void {
        if (modelModal.visible)
            modelModal.close();
        if (diffInspector.visible)
            diffInspector.close();
        else
            diffInspector.open();
    }

    function submitPromptText(text: string): void {
        const query = text.length > 0 ? text.trim() : promptBar.inputText.trim();
        if (query.length === 0)
            return;
        if (!query.startsWith("/") && !session.ready)
            return;
        promptBar.inputText = "";
        session.submitRequest(query);
        root.historyIndex = session.promptHistoryCount;
    }

    mode: Terminal.Inline
    title: {
        if (session.awaitingPermission) {
            return "⚠️ [ACTION REQUIRED] " + session.currentToolName + "(" + session.currentToolTarget + ") — Qt AI Studio";
        }
        if (modelModal && modelModal.visible) {
            return "⚙️ Select Neural Model — Qt AI Studio (" + session.activeModel + ")";
        }
        if (diffInspector && diffInspector.visible) {
            return "📝 Diff Inspector: " + diffInspector.targetFile + " — Qt AI Studio";
        }
        if (session.working) {
            return root.spinnerFrames[session.spinnerIndex] + " " + session.workingLabel + " — Qt AI Studio (" + session.activeModel + " · " + session.sessionCostStr + ")";
        }
        return "● Ready — Qt AI Studio (" + session.activeModel + " · " + session.sessionCostStr + ")";
    }
    mouseReporting: true

    Keys.onPressed: event => {
        if (event.key === Qt.Key_F2) {
            root.openModelPicker();
            event.accepted = true;
            return;
        }
        if (event.key === Qt.Key_F3) {
            root.openDiffInspector();
            event.accepted = true;
            return;
        }

        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Q) {
            Qt.quit();
            event.accepted = true;
            return;
        }

        if (session.ready && event.key === Qt.Key_Escape && !promptBar.popupVisible && !root.modalActive) {
            if (promptBar.inputText.length > 0) {
                promptBar.inputText = "";
                root.historyIndex = session.promptHistoryCount;
            } else {
                Qt.quit();
            }
            event.accepted = true;
            return;
        }

        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_C) {
            if (session.working) {
                session.interrupt();
            } else if (session.awaitingPermission) {
                session.choosePermission(3);
            } else if (promptBar.inputText.length > 0) {
                promptBar.inputText = "";
            } else {
                Qt.quit();
            }
            event.accepted = true;
            return;
        }

        if (session.awaitingPermission) {
            if (event.key === Qt.Key_1 || event.key === Qt.Key_Y) {
                session.choosePermission(0);
                event.accepted = true;
            } else if (event.key === Qt.Key_2) {
                session.choosePermission(1);
                event.accepted = true;
            } else if (event.key === Qt.Key_3 || event.key === Qt.Key_N) {
                session.choosePermission(2);
                event.accepted = true;
            } else if (event.key === Qt.Key_4 || event.key === Qt.Key_Escape) {
                session.choosePermission(3);
                event.accepted = true;
            } else if (event.key === Qt.Key_Up) {
                session.permissionIndex = Math.max(0, session.permissionIndex - 1);
                event.accepted = true;
            } else if (event.key === Qt.Key_Down) {
                session.permissionIndex = Math.min(3, session.permissionIndex + 1);
                event.accepted = true;
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                session.choosePermission(session.permissionIndex);
                event.accepted = true;
            }
            return;
        }

        if (session.ready) {
            if (event.key === Qt.Key_Up) {
                if (session.promptHistoryCount > 0) {
                    if (root.historyIndex > 0) {
                        root.historyIndex--;
                        promptBar.inputText = session.promptHistoryAt(root.historyIndex);
                        promptBar.inputCursorPosition = promptBar.inputText.length;
                    } else if (root.historyIndex === -1) {
                        root.historyIndex = session.promptHistoryCount - 1;
                        promptBar.inputText = session.promptHistoryAt(root.historyIndex);
                        promptBar.inputCursorPosition = promptBar.inputText.length;
                    }
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_Down) {
                if (root.historyIndex >= 0 && root.historyIndex < session.promptHistoryCount - 1) {
                    root.historyIndex++;
                    promptBar.inputText = session.promptHistoryAt(root.historyIndex);
                    promptBar.inputCursorPosition = promptBar.inputText.length;
                } else if (root.historyIndex >= session.promptHistoryCount - 1) {
                    root.historyIndex = session.promptHistoryCount;
                    promptBar.inputText = "";
                }
                event.accepted = true;
            }
        }
    }

    Component.onCompleted: {
        session.start();
    }

    AgentSession {
        id: session
        onOpenModelPickerRequested: () => root.openModelPicker()
        onOpenDiffInspectorRequested: () => root.openDiffInspector()
        onClearScrollbackRequested: () => history.clear()
        onQuitRequested: () => Qt.quit()
        onToastRequested: (message, status) => toast.notify(message, status)
    }

    Scrollback {
        id: history
        model: session.feedModel
        spacing: 1

        delegate: DelegateChooser {
            role: "type"

            DelegateChoice {
                roleValue: StreamFeedModel.BannerMessage
                BannerCard {
                    activeModel: session.activeModel
                }
            }
            DelegateChoice {
                roleValue: StreamFeedModel.UserMessage
                UserPromptCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.ThoughtMessage
                ThoughtCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.ToolCallMessage
                ToolCallCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.ToolResultMessage
                ToolResultCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.DiffMessage
                DiffCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.PermissionDecisionMessage
                PermissionCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.AgentMessage
                AgentResponseCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.SystemMessage
                SystemMessageCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.ErrorMessage
                ErrorMessageCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.SwarmTableMessage
                SwarmTableCard {}
            }
            DelegateChoice {
                roleValue: StreamFeedModel.RepoTreeMessage
                RepoTreeCard {}
            }
        }
    }

    // Live inline region: in-flight streaming deltas above the pinned prompt bar
    Column {
        width: root.width
        spacing: 0

        LiveStreamCard {
            visible: session.hasLiveStream && !root.modalActive
            width: parent.width
            streamKind: session.liveStreamKind
            title: session.liveStreamTitle
            content: session.liveStreamText
        }

        PinnedPromptBar {
            id: promptBar
            width: parent.width
            ready: session.ready
            working: session.working
            awaitingPermission: session.awaitingPermission
            modalActive: root.modalActive
            activeModel: session.activeModel
            sessionCostStr: session.sessionCostStr
            currentToolName: session.currentToolName
            currentToolTarget: session.currentToolTarget
            currentToolAction: session.currentToolAction
            permissionIndex: session.permissionIndex
            spinnerFrames: root.spinnerFrames
            spinnerIndex: session.spinnerIndex
            workingLabel: session.workingLabel

            onRequestSubmitted: text => root.submitPromptText(text)
            onPermissionChosen: index => session.choosePermission(index)
            onPermissionHovered: index => session.permissionIndex = index
            onOpenModelPickerRequested: () => root.openModelPicker()
            onOpenDiffInspectorRequested: () => root.openDiffInspector()
            onInterruptRequested: () => session.interrupt()
        }
    }

    ModelPickerModal {
        id: modelModal
        x: 0
        y: 0
        width: root.width
        currentModel: session.activeModel
        onSwitchToDiffRequested: () => root.openDiffInspector()
        onModelSelected: modelId => session.selectModel(modelId)
    }

    DiffInspectorModal {
        id: diffInspector
        x: 0
        y: 0
        width: root.width
        diffModel: session.diffModel
        onSwitchToModelRequested: () => root.openModelPicker()
    }

    ToastNotification {
        id: toast
        x: Math.max(0, root.width - width - 2)
        y: 1
    }

    // Drives spinner/elapsed telemetry and steps simulated transports under CLOCK VIRTUAL.
    Timer {
        interval: 100
        running: session.working
        repeat: true
        onTriggered: session.tick(100)
    }
}
