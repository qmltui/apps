#include "MockAgentTransport.h"

MockAgentTransport::MockAgentTransport(QObject* parent) : IAgentTransport(parent) {}

PatchPayload MockAgentTransport::inlineRendererPatch() {
    using qmltui::Color;
    const Color slate = Color::fromString(QStringLiteral("#94a3b8"));
    const Color muted = Color::fromString(QStringLiteral("#64748b"));
    const Color rose = Color::fromString(QStringLiteral("#f87171"));
    const Color green = Color::fromString(QStringLiteral("#34d399"));
    const Color clearBg = Color::transparent();
    const Color ctxBg = Color::fromString(QStringLiteral("#1e1e2e"));
    const Color delBg = Color::fromString(QStringLiteral("#2a1215"));
    const Color addBg = Color::fromString(QStringLiteral("#0d2818"));
    const QString blankLeft(36, QLatin1Char(' '));

    PatchPayload patch;
    patch.filePath = QStringLiteral("src/QmlTui/engine/InlineRenderer.cpp");
    patch.summaryLabel = QStringLiteral("(+3, -1 lines)");
    patch.unifiedHunkHeader =
        QStringLiteral("@@ -72,6 +72,9 @@ void InlineRenderer::resize(...) @@");
    patch.removedLines = QStringLiteral("-    m_promptBuffer.resize(width, m_lastPromptHeight);");
    patch.addedLines =
        QStringLiteral("+    const int safeHeight = std::max(0, m_lastPromptHeight);\n"
                       "+    m_promptBuffer.resize(width, safeHeight);\n"
                       "+    markDirty();");
    patch.sideBySideHeader = QStringLiteral("@@ -72,6 +72,8 @@ ");
    patch.additionsLabel = QStringLiteral("+3 additions");
    patch.deletionsLabel = QStringLiteral("-1 deletion");
    patch.sideBySideRows = {
        {QStringLiteral(" 72: void InlineRenderer::render() {"), slate,
         QStringLiteral(" 72: void InlineRenderer::render() {"), slate, clearBg},
        {QStringLiteral(" 73:   const int h = buffer.height();"), slate,
         QStringLiteral(" 73:   const int h = buffer.height();"), slate, ctxBg},
        {QStringLiteral("-74:   device.write(buffer.data());"), rose,
         QStringLiteral("    (replaced with bounds check)"), muted, delBg},
        {blankLeft, muted, QStringLiteral("+74:   const int safeHeight = clamp(h);"), green, addBg},
        {blankLeft, muted, QStringLiteral("+75:   markDirty(safeHeight);"), green, addBg},
        {blankLeft, muted, QStringLiteral("+76:   device.write(buffer.slice(...));"), green, addBg},
        {QStringLiteral(" 77: }"), slate, QStringLiteral(" 77: }"), slate, clearBg},
    };
    return patch;
}

PatchPayload MockAgentTransport::textGridBufferPatch() {
    using qmltui::Color;
    const Color slate = Color::fromString(QStringLiteral("#94a3b8"));
    const Color muted = Color::fromString(QStringLiteral("#64748b"));
    const Color rose = Color::fromString(QStringLiteral("#f87171"));
    const Color green = Color::fromString(QStringLiteral("#34d399"));
    const Color clearBg = Color::transparent();
    const Color ctxBg = Color::fromString(QStringLiteral("#1e1e2e"));
    const Color delBg = Color::fromString(QStringLiteral("#2a1215"));
    const Color addBg = Color::fromString(QStringLiteral("#0d2818"));
    const QString blankLeft(36, QLatin1Char(' '));

    PatchPayload patch;
    patch.filePath = QStringLiteral("src/QmlTui/engine/TextGridBuffer.cpp");
    patch.summaryLabel = QStringLiteral("(+4, -1 lines)");
    patch.unifiedHunkHeader =
        QStringLiteral("@@ -118,5 +118,8 @@ void TextGridBuffer::putCluster(...) @@");
    patch.removedLines = QStringLiteral("-    writeCell(x, y, cluster);");
    patch.addedLines = QStringLiteral("+    const int cellSpan = std::clamp(cluster.width, 1, 2);\n"
                                      "+    if (x + cellSpan > m_width)\n"
                                      "+        return;\n"
                                      "+    writeCell(x, y, cluster);");
    patch.sideBySideHeader = QStringLiteral("@@ -118,5 +118,8 @@ ");
    patch.additionsLabel = QStringLiteral("+4 additions");
    patch.deletionsLabel = QStringLiteral("-1 deletion");
    patch.sideBySideRows = {
        {QStringLiteral("118: void TextGridBuffer::put() {"), slate,
         QStringLiteral("118: void TextGridBuffer::put() {"), slate, clearBg},
        {QStringLiteral("119:   if (y < 0 || y >= m_h) return;"), slate,
         QStringLiteral("119:   if (y < 0 || y >= m_h) return;"), slate, ctxBg},
        {QStringLiteral("-120:  writeCell(x, y, cluster);"), rose,
         QStringLiteral("    (replaced with width clamp)"), muted, delBg},
        {blankLeft, muted, QStringLiteral("+120:  const int span = clampW(c);"), green, addBg},
        {blankLeft, muted, QStringLiteral("+121:  if (x + span > m_width) return;"), green, addBg},
        {blankLeft, muted, QStringLiteral("+122:  writeCell(x, y, cluster);"), green, addBg},
        {QStringLiteral("121: }"), slate, QStringLiteral("123: }"), slate, clearBg},
    };
    return patch;
}

int MockAgentTransport::inferScenario(const QString& prompt) {
    const QString lower = prompt.toLower();
    if (lower.contains(QLatin1String("swarm")) || lower.contains(QLatin1String("audit")) ||
        lower.contains(QLatin1String("subagent"))) {
        return 2;
    }
    if (lower.contains(QLatin1String("sec")) || lower.contains(QLatin1String("vuln")) ||
        lower.contains(QLatin1String("patch"))) {
        return 3;
    }
    if (lower.contains(QLatin1String("tree")) || lower.contains(QLatin1String("files")) ||
        lower.contains(QLatin1String("arch"))) {
        return 4;
    }
    return 1;
}

void MockAgentTransport::startTurn(quint64 turnId, const QString& prompt, const QString& modelId,
                                   int scenarioHint) {
    Q_UNUSED(modelId);
    m_turnId = turnId;
    m_scenario = (scenarioHint >= 1 && scenarioHint <= 4) ? scenarioHint : inferScenario(prompt);
    m_phase = 0;
    m_phaseTick = 0;
    m_accumulatedMs = 0;
    m_awaitingPermission = false;
    m_pendingPermissionId = 0;
    m_running = true;

    emit usageReported(1520, 380);
    emit activityChanged(QStringLiteral("Planning"));
}

void MockAgentTransport::resolvePermission(quint64 permissionId, PermissionDecision decision) {
    if (!m_awaitingPermission || permissionId != m_pendingPermissionId) return;

    m_awaitingPermission = false;
    m_pendingPermissionId = 0;

    if (decision == PermissionDecision::Deny || decision == PermissionDecision::Cancel) {
        m_running = false;
        return;
    }

    m_phase = 1;
    m_phaseTick = 0;
    m_accumulatedMs = 0;
    m_running = true;
    emit activityChanged(m_resumeActivity);
}

void MockAgentTransport::cancelTurn() {
    m_running = false;
    m_awaitingPermission = false;
    m_pendingPermissionId = 0;
}

void MockAgentTransport::reset() {
    cancelTurn();
    m_phase = 0;
    m_phaseTick = 0;
    m_accumulatedMs = 0;
}

void MockAgentTransport::requestStepPermission(const QString& toolName, const QString& target,
                                               const QString& actionSummary,
                                               const QString& resumeActivity) {
    const quint64 permId = m_nextPermissionId++;
    m_running = false;
    m_awaitingPermission = true;
    m_pendingPermissionId = permId;
    m_resumeActivity = resumeActivity;
    // Set state before emitting so synchronous auto-approval in AgentSession
    // can immediately resolvePermission() and transition m_phase to 1.
    emit permissionRequested(permId, toolName, target, actionSummary);
}

void MockAgentTransport::tick(int deltaMs) {
    if (!m_running || m_awaitingPermission || deltaMs <= 0) return;

    m_accumulatedMs += deltaMs;
    while (m_running && !m_awaitingPermission && m_accumulatedMs >= 100) {
        m_accumulatedMs -= 100;
        ++m_phaseTick;

        switch (m_scenario) {
        case 1:
            stepScenario1();
            break;
        case 2:
            stepScenario2();
            break;
        case 3:
            stepScenario3();
            break;
        default:
            stepScenario4();
            break;
        }
    }
}

void MockAgentTransport::stepScenario1() {
    if (m_phase == 0) {
        switch (m_phaseTick) {
        case 1:
            emit activityChanged(QStringLiteral("Planner"));
            emit thoughtDelta(QStringLiteral("Analyzing InlineRenderer "));
            break;
        case 2:
            emit thoughtDelta(QStringLiteral("buffer logic to eliminate "));
            break;
        case 3:
            emit thoughtDelta(QStringLiteral("resize flicker and clamp bounds."));
            break;
        case 5:
            emit thoughtCompleted(QStringLiteral("0.8s"));
            break;
        case 7:
            emit activityChanged(QStringLiteral("Grep"));
            emit toolCallStarted(
                QStringLiteral("Grep"),
                QStringLiteral("query=\"promptRowToAnsi\", path=\"src/QmlTui/engine/\""));
            break;
        case 11:
            emit toolCallCompleted(QStringLiteral(
                "Found 3 matches across 2 files (InlineRenderer.h, InlineRenderer.cpp)"));
            break;
        case 14:
            emit activityChanged(QStringLiteral("ViewFile"));
            emit toolCallStarted(
                QStringLiteral("ViewFile"),
                QStringLiteral("path=\"src/QmlTui/engine/InlineRenderer.cpp\", lines=70-105"));
            break;
        case 18:
            emit toolCallCompleted(QStringLiteral("Read 35 lines from InlineRenderer.cpp"));
            break;
        case 22: {
            const PatchPayload patch = inlineRendererPatch();
            emit activityChanged(QStringLiteral("EditFile"));
            emit diffProposed(patch);
            requestStepPermission(
                QStringLiteral("EditFile"), patch.filePath,
                QStringLiteral("Clamp safeHeight and emit atomic buffer flush (+3, -1 lines)"),
                QStringLiteral("Bash"));
            break;
        }
        default:
            break;
        }
        return;
    }

    switch (m_phaseTick) {
    case 2:
        emit activityChanged(QStringLiteral("Bash"));
        emit toolCallStarted(QStringLiteral("Bash"),
                             QStringLiteral("command=\"ninja -C build qmltui_agent_stream\""));
        break;
    case 6:
        emit toolCallCompleted(QStringLiteral("Built target qmltui_agent_stream (exit 0, 0.28s)"));
        break;
    case 9:
        emit activityChanged(QStringLiteral("CTest"));
        emit toolCallStarted(QStringLiteral("CTest"),
                             QStringLiteral("command=\"ctest --test-dir build -R agent_stream\""));
        break;
    case 13:
        emit toolCallCompleted(QStringLiteral("Passed 2/2 tests: zero regressions (0.08s)"));
        break;
    case 14:
        emit activityChanged(QStringLiteral("Streaming"));
        emit responseDelta(QStringLiteral("InlineRenderer optimization complete:\n"));
        break;
    case 15:
        emit responseDelta(QStringLiteral(
            "• Implemented in-memory atomic double buffering in InlineRenderer.cpp\n"));
        break;
    case 16:
        emit responseDelta(
            QStringLiteral("• Replaced multi-step clears with single device.write() flushes\n"));
        break;
    case 17:
        emit responseDelta(QStringLiteral("• Rebuilt and validated: 100% tests passed"));
        break;
    case 18:
        m_running = false;
        emit responseCompleted(QStringLiteral("2.1k tokens · 7.7s"));
        emit turnSucceeded(QStringLiteral("Refactor patch applied & tested"),
                           QStringLiteral("success"));
        break;
    default:
        break;
    }
}

void MockAgentTransport::stepScenario2() {
    switch (m_phaseTick) {
    case 1:
        emit activityChanged(QStringLiteral("Swarm"));
        emit thoughtDelta(QStringLiteral("Spawning 3 parallel subagent workers "));
        break;
    case 3:
        emit thoughtDelta(QStringLiteral("(Memory, QML bindings, Concurrency)."));
        break;
    case 5:
        emit thoughtCompleted(QStringLiteral("0.6s"));
        break;
    case 8:
        emit activityChanged(QStringLiteral("Subagent(MemoryAuditor)"));
        emit toolCallStarted(QStringLiteral("Subagent"),
                             QStringLiteral("scope=\"src/QmlTui/Engine/\", check=\"leaks\""));
        break;
    case 12:
        emit toolCallCompleted(QStringLiteral(
            "MemoryAuditor: 0 heap leaks, 0 use-after-free conditions (checked 18 classes)"));
        break;
    case 15:
        emit activityChanged(QStringLiteral("Subagent(QmlProfiler)"));
        emit toolCallStarted(QStringLiteral("Subagent"),
                             QStringLiteral("scope=\"apps/agent_stream/\", check=\"bindings\""));
        break;
    case 19:
        emit toolCallCompleted(
            QStringLiteral("QmlProfiler: 100% typed delegate roles; 0 binding loops detected"));
        break;
    case 22:
        emit swarmAuditReady(QStringLiteral("Parallel Audit Results"));
        break;
    case 23:
        emit activityChanged(QStringLiteral("Streaming"));
        emit responseDelta(QStringLiteral("Swarm audit finished with 100% clean "));
        break;
    case 25:
        emit responseDelta(QStringLiteral("verification across all 3 workers."));
        break;
    case 26:
        m_running = false;
        emit responseCompleted(QStringLiteral("3.1k tokens · 1.4s"));
        emit turnSucceeded(QStringLiteral("Swarm audit complete: 100% clean"),
                           QStringLiteral("success"));
        break;
    default:
        break;
    }
}

void MockAgentTransport::stepScenario3() {
    if (m_phase == 0) {
        switch (m_phaseTick) {
        case 1:
            emit activityChanged(QStringLiteral("SecSandbox"));
            emit thoughtDelta(QStringLiteral("Scanning text buffer bounds "));
            break;
        case 3:
            emit thoughtDelta(QStringLiteral("against UTF-8 overflow vectors."));
            break;
        case 5:
            emit thoughtCompleted(QStringLiteral("0.5s"));
            break;
        case 8:
            emit activityChanged(QStringLiteral("FuzzTester"));
            emit toolCallStarted(QStringLiteral("FuzzTester"),
                                 QStringLiteral("input=\"malformed_utf8_sequences.bin\""));
            break;
        case 13:
            emit toolCallCompleted(
                QStringLiteral("Identified edge case in wide-character column clipping"));
            break;
        case 17: {
            const PatchPayload patch = textGridBufferPatch();
            emit activityChanged(QStringLiteral("SecurityPatch"));
            emit diffProposed(patch);
            requestStepPermission(QStringLiteral("SecurityPatch"), patch.filePath,
                                  QStringLiteral("Add displayWidth boundary clamp (+4, -1 lines)"),
                                  QStringLiteral("StressTester"));
            break;
        }
        default:
            break;
        }
        return;
    }

    switch (m_phaseTick) {
    case 4:
        emit toolCallCompleted(
            QStringLiteral("Passed 24/24 buffer stress tests with 0 faults (0.12s)"));
        break;
    case 6:
        emit activityChanged(QStringLiteral("Streaming"));
        emit responseDelta(QStringLiteral("Security patch applied and "));
        break;
    case 8:
        emit responseDelta(QStringLiteral("verified with stress tests."));
        break;
    case 9:
        m_running = false;
        emit responseCompleted(QStringLiteral("2.4k tokens · 2.1s"));
        emit turnSucceeded(QStringLiteral("Security patch verified"), QStringLiteral("success"));
        break;
    default:
        break;
    }
}

void MockAgentTransport::stepScenario4() {
    switch (m_phaseTick) {
    case 1:
        emit activityChanged(QStringLiteral("RepoIndexer"));
        emit thoughtDelta(QStringLiteral("Scanning repository workspace "));
        break;
    case 3:
        emit thoughtDelta(QStringLiteral("hierarchy and file layout..."));
        break;
    case 4:
        emit thoughtCompleted(QStringLiteral("0.4s"));
        break;
    case 7:
        emit repoTreeReady(QStringLiteral("~/QmlTui Workspace"));
        break;
    case 8:
        emit activityChanged(QStringLiteral("Streaming"));
        emit responseDelta(QStringLiteral("Indexed workspace tree:\n"));
        break;
    case 9:
        emit responseDelta(
            QStringLiteral("• Engine core: InlineRenderer (ANSI stream) & TextGridBuffer\n"));
        break;
    case 10:
        emit responseDelta(QStringLiteral(
            "• Stream components: TableView, TreeView, ProgressBar, Sparkline, StatusBadge\n"));
        break;
    case 11:
        emit responseDelta(
            QStringLiteral("• Inline session: agent output stays in terminal scrollback"));
        break;
    case 12:
        m_running = false;
        emit responseCompleted(QStringLiteral("1.2k tokens · 0.9s"));
        emit turnSucceeded(QStringLiteral("Workspace tree indexed"), QStringLiteral("info"));
        break;
    default:
        break;
    }
}
