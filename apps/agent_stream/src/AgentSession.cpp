#include "AgentSession.h"

#include "MockAgentTransport.h"

#include <QRegularExpression>
#include <algorithm>

namespace {

constexpr int kSpinnerFrameCount = 10;

QString defaultScenarioPrompt(int scenarioId) {
    switch (scenarioId) {
    case 1:
        return QStringLiteral(
            "Optimize InlineRenderer buffer resizing and verify zero-copy safety");
    case 2:
        return QStringLiteral(
            "Dispatch parallel subagent swarm to audit memory leaks and concurrency locks");
    case 3:
        return QStringLiteral("Run security sandbox exploit audit and generate regression patch");
    default:
        return QStringLiteral("Explore repository workspace hierarchy and file structure");
    }
}

} // namespace

AgentSession::AgentSession(QObject* parent) : QObject(parent) {
    m_diffModel.setPatch(MockAgentTransport::inlineRendererPatch());
    setTransport(std::make_unique<MockAgentTransport>());
}

void AgentSession::setTransport(std::unique_ptr<IAgentTransport> transport) {
    if (!transport) return;
    if (m_transport) disconnect(m_transport.get(), nullptr, this, nullptr);
    m_transport = std::move(transport);
    wireTransport();
    emit transportChanged();
}

QString AgentSession::transportName() const {
    return m_transport ? m_transport->transportName() : QStringLiteral("none");
}

void AgentSession::wireTransport() {
    IAgentTransport* t = m_transport.get();
    if (!t) return;

    connect(t, &IAgentTransport::activityChanged, this, [this](const QString& label) {
        if (m_currentToolName == label) return;
        m_currentToolName = label;
        emit activityChanged();
        emit workingLabelChanged();
    });

    connect(t, &IAgentTransport::thoughtDelta, this, [this](const QString& chunk) {
        m_liveStreamKind = LiveThoughtStream;
        m_liveStreamTitle = QStringLiteral("streaming...");
        m_liveStreamText += chunk;
        emit liveStreamChanged();
    });

    connect(t, &IAgentTransport::thoughtCompleted, this, [this](const QString& durationLabel) {
        const QString committed = m_liveStreamText;
        clearLiveStream();
        m_feedModel.appendThought(durationLabel, committed);
    });

    connect(t, &IAgentTransport::responseDelta, this, [this](const QString& chunk) {
        m_liveStreamKind = LiveResponseStream;
        m_liveStreamTitle = QStringLiteral("streaming...");
        m_liveStreamText += chunk;
        emit liveStreamChanged();
    });

    connect(t, &IAgentTransport::responseCompleted, this, [this](const QString& summaryLabel) {
        const QString committed = m_liveStreamText;
        clearLiveStream();
        m_feedModel.appendAgent(QStringLiteral("Agent"), committed, summaryLabel);
    });

    connect(t, &IAgentTransport::toolCallStarted, this,
            [this](const QString& toolName, const QString& argsSummary) {
                clearLiveStream();
                m_feedModel.appendToolCall(toolName, argsSummary);
            });

    connect(t, &IAgentTransport::toolCallCompleted, this,
            [this](const QString& resultSummary) { m_feedModel.appendToolResult(resultSummary); });

    connect(t, &IAgentTransport::diffProposed, this, [this](const PatchPayload& patch) {
        clearLiveStream();
        m_diffModel.setPatch(patch);
        m_feedModel.appendDiff(patch);
        emit diffChanged();
    });

    connect(t, &IAgentTransport::permissionRequested, this,
            [this](quint64 permissionId, const QString& toolName, const QString& target,
                   const QString& actionSummary) {
                clearLiveStream();
                m_pendingPermissionId = permissionId;
                m_currentToolName = toolName;
                m_currentToolTarget = target;
                m_currentToolAction = actionSummary;
                emit activityChanged();
                emit workingLabelChanged();
                emit permissionChanged();

                if (m_sessionAlwaysAllow) {
                    m_feedModel.appendPermission(
                        QStringLiteral("Auto-approved: %1(%2)").arg(toolName, target), true);
                    m_pendingPermissionId = 0;
                    if (m_transport)
                        m_transport->resolvePermission(permissionId, PermissionDecision::AllowOnce);
                    return;
                }

                setPermissionIndex(0);
                setState(PermissionState);
            });

    connect(t, &IAgentTransport::swarmAuditReady, this, [this](const QString& title) {
        clearLiveStream();
        m_feedModel.appendSwarmTable(title);
    });

    connect(t, &IAgentTransport::repoTreeReady, this, [this](const QString& title) {
        clearLiveStream();
        m_feedModel.appendRepoTree(title);
    });

    connect(t, &IAgentTransport::usageReported, this,
            [this](int inputTokens, int outputTokens) { addUsage(inputTokens, outputTokens); });

    connect(t, &IAgentTransport::turnSucceeded, this,
            [this](const QString& toastMessage, const QString& toastStatus) {
                clearLiveStream();
                setState(ReadyState);
                if (!toastMessage.isEmpty()) emit toastRequested(toastMessage, toastStatus);
            });

    connect(t, &IAgentTransport::transportError, this, [this](const QString& message) {
        clearLiveStream();
        m_feedModel.appendError(message);
        setState(ReadyState);
    });
}

void AgentSession::setActiveModel(const QString& modelId) {
    if (modelId.isEmpty() || m_activeModel == modelId) return;
    m_activeModel = modelId;
    emit activeModelChanged();
}

void AgentSession::selectModel(const QString& modelId) {
    if (modelId.isEmpty()) return;
    setActiveModel(modelId);
    emit toastRequested(QStringLiteral("Model: ") + modelId, QStringLiteral("info"));
    m_feedModel.appendSystem(QStringLiteral("Model"),
                             QStringLiteral("Switched neural model to: ") + modelId);
}

void AgentSession::setPermissionIndex(int index) {
    const int clamped = std::clamp(index, 0, 3);
    if (m_permissionIndex == clamped) return;
    m_permissionIndex = clamped;
    emit permissionIndexChanged();
}

QString AgentSession::workingLabel() const {
    const QString elapsed = QString::number(m_elapsedDeciseconds / 10.0, 'f', 1) + QLatin1Char('s');
    if (!m_currentToolName.isEmpty())
        return QStringLiteral("%1 (%2)...").arg(m_currentToolName, elapsed);
    return QStringLiteral("Reasoning (%1)...").arg(elapsed);
}

QString AgentSession::promptHistoryAt(int index) const {
    if (index < 0 || index >= m_promptHistory.size()) return {};
    return m_promptHistory.at(index);
}

void AgentSession::start() {
    m_feedModel.appendBanner();
}

void AgentSession::submitRequest(const QString& text) {
    const QString query = text.trimmed();
    if (query.isEmpty()) return;

    if (query.startsWith(QLatin1Char('/'))) {
        recordPrompt(query);
        handleSlashCommand(query);
        return;
    }

    if (!isReady()) return;

    recordPrompt(query);
    dispatchTurn(query, 0);
}

void AgentSession::handleSlashCommand(const QString& cmd) {
    const QString trimmed = cmd.trimmed();
    const QString mainCmd = trimmed.section(QLatin1Char(' '), 0, 0).toLower();

    if (mainCmd == QLatin1String("/help")) {
        m_feedModel.appendSystem(
            QStringLiteral("Help & Workflows"),
            QStringLiteral(
                "Commands & Workflows:\n"
                "  /demo [1-4]   Execute agent workflows (1: Refactor, 2: Swarm, 3: Patch, 4: "
                "Tree)\n"
                "  /swarm        Show subagent swarm status (TableView)\n"
                "  /tree         Inspect repository workspace hierarchy (TreeView)\n"
                "  /cost         Show session token metrics and capacity (ProgressBar & "
                "Sparkline)\n"
                "  /model        Toggle active neural model (Claude 3.7 / GPT-4.5 / Gemini 2.0)\n"
                "  /doctor       Run system environment diagnostics\n"
                "  /clear        Clear terminal history\n"
                "  /exit, /quit  Exit studio session\n\n"
                "Keys: Enter (submit), 1-4 (select), Ctrl+C (abort), Ctrl+Q (quit), Esc (close or "
                "quit)"));
        return;
    }

    if (mainCmd == QLatin1String("/swarm")) {
        m_feedModel.appendSwarmTable(QStringLiteral("Active Mesh Status"));
        return;
    }

    if (mainCmd == QLatin1String("/tree")) {
        m_feedModel.appendRepoTree(QStringLiteral("~/QmlTui Workspace"));
        return;
    }

    if (mainCmd == QLatin1String("/cost") || mainCmd == QLatin1String("/usage")) {
        m_feedModel.appendSystem(QStringLiteral("Usage & Cost"),
                                 QStringLiteral("Session Metrics:\n"
                                                "  • Input Tokens:   %1\n"
                                                "  • Output Tokens:  %2\n"
                                                "  • Total Cost:     %3\n"
                                                "  • Active Model:   %4")
                                     .arg(m_totalTokens * 4)
                                     .arg(m_totalTokens)
                                     .arg(m_sessionCostStr, m_activeModel));
        return;
    }

    if (mainCmd == QLatin1String("/diff") || mainCmd == QLatin1String("/patch")) {
        emit openDiffInspectorRequested();
        return;
    }

    if (mainCmd == QLatin1String("/model")) {
        emit openModelPickerRequested();
        return;
    }

    if (mainCmd == QLatin1String("/doctor")) {
        m_feedModel.appendSystem(
            QStringLiteral("Diagnostics"),
            QStringLiteral(
                "System Status:\n"
                "  ✓ Repository:   ~/QmlTui (branch: main*)\n"
                "  ✓ Harness:      %1\n"
                "  ✓ Stream:       Live Delta -> Scrollback Commit\n"
                "  ✓ Components:   TableView, TreeView, ProgressBar, Sparkline, StatusBadge\n"
                "  ✓ Status:       All systems nominal")
                .arg(transportName()));
        return;
    }

    if (mainCmd == QLatin1String("/clear")) {
        if (m_transport) m_transport->reset();
        clearLiveStream();
        setState(ReadyState);
        emit clearScrollbackRequested();
        m_feedModel.clear();
        m_feedModel.appendBanner();
        return;
    }

    if (mainCmd == QLatin1String("/exit") || mainCmd == QLatin1String("/quit")) {
        emit quitRequested();
        return;
    }

    if (mainCmd.startsWith(QLatin1String("/demo"))) {
        int demoId = 1;
        static const QRegularExpression demoRegex(QStringLiteral("^/demo\\s*(\\d+)"),
                                                  QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = demoRegex.match(trimmed);
        if (match.hasMatch()) {
            const int parsed = match.captured(1).toInt();
            if (parsed >= 1 && parsed <= 4) demoId = parsed;
        }
        dispatchTurn(defaultScenarioPrompt(demoId), demoId);
        return;
    }

    m_feedModel.appendError(
        QStringLiteral("Unknown command: %1. Type /help for available workflows.").arg(cmd));
}

void AgentSession::dispatchTurn(const QString& prompt, int scenarioHint) {
    if (m_transport) m_transport->cancelTurn();
    clearLiveStream();

    m_elapsedDeciseconds = 0;
    m_subDecisecondMs = 0;
    m_pendingPermissionId = 0;
    m_currentToolName = QStringLiteral("Planning");
    emit activityChanged();
    emit workingLabelChanged();

    m_feedModel.appendUser(prompt);
    setState(WorkingState);

    if (m_transport) m_transport->startTurn(m_nextTurnId++, prompt, m_activeModel, scenarioHint);
}

void AgentSession::choosePermission(int index) {
    if (!isAwaitingPermission()) return;

    const int clamped = std::clamp(index, 0, 3);
    setPermissionIndex(clamped);
    const quint64 permId = m_pendingPermissionId;
    m_pendingPermissionId = 0;

    if (clamped == 2) {
        m_feedModel.appendPermission(
            QStringLiteral("Execution denied: %1(%2)").arg(m_currentToolName, m_currentToolTarget),
            false);
        m_feedModel.appendError(QStringLiteral("Agent task halted by user."));
        if (m_transport) m_transport->resolvePermission(permId, PermissionDecision::Deny);
        setState(ReadyState);
        return;
    }

    if (clamped == 3) {
        m_feedModel.appendError(QStringLiteral("Cancelled request."));
        if (m_transport) m_transport->resolvePermission(permId, PermissionDecision::Cancel);
        setState(ReadyState);
        return;
    }

    if (clamped == 1) {
        m_sessionAlwaysAllow = true;
        m_feedModel.appendPermission(
            QStringLiteral("Always allow: %1(%2)").arg(m_currentToolName, m_currentToolTarget),
            true);
    } else {
        m_feedModel.appendPermission(
            QStringLiteral("Approved: %1(%2)").arg(m_currentToolName, m_currentToolTarget), true);
    }

    setState(WorkingState);
    if (m_transport) {
        m_transport->resolvePermission(permId, clamped == 1 ? PermissionDecision::AlwaysAllow
                                                            : PermissionDecision::AllowOnce);
    }
}

void AgentSession::interrupt() {
    if (m_transport) m_transport->cancelTurn();
    clearLiveStream();
    m_pendingPermissionId = 0;
    m_feedModel.appendError(QStringLiteral("Execution interrupted by user (Ctrl+C)."));
    setState(ReadyState);
}

void AgentSession::tick(int deltaMs) {
    if (!isWorking() || deltaMs <= 0) return;

    m_subDecisecondMs += deltaMs;
    bool clockAdvanced = false;
    while (m_subDecisecondMs >= 100) {
        m_subDecisecondMs -= 100;
        ++m_elapsedDeciseconds;
        m_spinnerIndex = (m_spinnerIndex + 1) % kSpinnerFrameCount;
        clockAdvanced = true;
    }

    if (clockAdvanced) {
        emit tickUpdated();
        emit workingLabelChanged();
    }

    if (m_transport) m_transport->tick(deltaMs);
}

void AgentSession::setState(AgentState next) {
    if (m_state == next) return;
    m_state = next;
    emit stateChanged();
}

void AgentSession::clearLiveStream() {
    if (m_liveStreamKind == NoLiveStream && m_liveStreamText.isEmpty() &&
        m_liveStreamTitle.isEmpty()) {
        return;
    }
    m_liveStreamKind = NoLiveStream;
    m_liveStreamTitle.clear();
    m_liveStreamText.clear();
    emit liveStreamChanged();
}

void AgentSession::recordPrompt(const QString& prompt) {
    m_promptHistory.append(prompt);
    emit promptHistoryChanged();
}

void AgentSession::addUsage(int inputTokens, int outputTokens) {
    Q_UNUSED(inputTokens);
    if (outputTokens <= 0) return;
    m_totalTokens += outputTokens;
    const double cost = (static_cast<double>(m_totalTokens) * 0.000008) + 0.012;
    m_sessionCostStr = QLatin1Char('$') + QString::number(cost, 'f', 3);
    emit usageChanged();
}
