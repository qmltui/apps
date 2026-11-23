#include "AgentModels.h"
#include "AgentSession.h"
#include "AgentTransport.h"
#include "MockAgentTransport.h"

#include <QSignalSpy>
#include <QtTest>

namespace {

// Asynchronous-style custom harness stub (never uses tick()) verifying that
// external LLM / ACP adapters can drive AgentSession directly via signals.
class StubExternalTransport : public IAgentTransport {
    Q_OBJECT

public:
    explicit StubExternalTransport(QObject* parent = nullptr) : IAgentTransport(parent) {}

    QString transportName() const override { return QStringLiteral("stub-acp-stdio"); }

    void startTurn(quint64 turnId, const QString& prompt, const QString& modelId,
                   int scenarioHint) override {
        lastTurnId = turnId;
        lastPrompt = prompt;
        lastModel = modelId;
        lastScenarioHint = scenarioHint;
    }

    void resolvePermission(quint64 permissionId, PermissionDecision decision) override {
        resolvedPermissionId = permissionId;
        lastDecision = decision;
    }

    void cancelTurn() override { ++cancelCount; }
    void reset() override { ++resetCount; }

    using IAgentTransport::activityChanged;
    using IAgentTransport::diffProposed;
    using IAgentTransport::permissionRequested;
    using IAgentTransport::responseCompleted;
    using IAgentTransport::responseDelta;
    using IAgentTransport::thoughtCompleted;
    using IAgentTransport::thoughtDelta;
    using IAgentTransport::toolCallCompleted;
    using IAgentTransport::toolCallStarted;
    using IAgentTransport::turnSucceeded;
    using IAgentTransport::usageReported;

    quint64 lastTurnId = 0;
    QString lastPrompt;
    QString lastModel;
    int lastScenarioHint = 0;
    quint64 resolvedPermissionId = 0;
    PermissionDecision lastDecision = PermissionDecision::Cancel;
    int cancelCount = 0;
    int resetCount = 0;
};

} // namespace

class TestAgentModels : public QObject {
    Q_OBJECT

private slots:
    void commandCatalogShiftsWindowWithoutModelReset() {
        CommandCatalogModel model;
        QCOMPARE(model.matchCount(), 13);
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.data(model.index(0, 0), CommandCatalogModel::CmdRole).toString(),
                 QStringLiteral("/help"));

        QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
        QSignalSpy dataSpy(&model, &QAbstractItemModel::dataChanged);

        model.setWindowStart(3);
        QCOMPARE(resetSpy.count(), 0);
        QCOMPARE(dataSpy.count(), 1);
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.data(model.index(0, 0), CommandCatalogModel::ActualIndexRole).toInt(), 3);
        QCOMPARE(model.data(model.index(0, 0), CommandCatalogModel::CmdRole).toString(),
                 QStringLiteral("/demo 3"));
        QCOMPARE(model.data(model.index(4, 0), CommandCatalogModel::CmdRole).toString(),
                 QStringLiteral("/tree"));

        model.setFilterText(QStringLiteral("/dem"));
        QCOMPARE(resetSpy.count(), 0);
        QCOMPARE(model.windowStart(), 0);
        QCOMPARE(model.selectedIndex(), 0);
        QCOMPARE(model.matchCount(), 4);
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.currentCommand(), QStringLiteral("/demo 1"));

        QSignalSpy currentCmdSpy(&model, &CommandCatalogModel::currentCommandChanged);
        model.setFilterText(QStringLiteral("/mod"));
        QCOMPARE(model.selectedIndex(), 0);
        QCOMPARE(currentCmdSpy.count(), 1);
        QCOMPARE(model.currentCommand(), QStringLiteral("/model"));
    }

    void diffModelsReflectPatchPayload() {
        StreamFeedModel feed;
        feed.appendDiff(MockAgentTransport::inlineRendererPatch());
        feed.appendDiff(MockAgentTransport::textGridBufferPatch());
        QCOMPARE(feed.rowCount(), 2);
        QCOMPARE(feed.data(feed.index(0, 0), StreamFeedModel::ExtraRole).toString(),
                 QStringLiteral("(+3, -1 lines)"));
        QCOMPARE(feed.data(feed.index(1, 0), StreamFeedModel::ExtraRole).toString(),
                 QStringLiteral("(+4, -1 lines)"));
        QVERIFY(feed.data(feed.index(1, 0), StreamFeedModel::HunkHeaderRole)
                    .toString()
                    .contains(QStringLiteral("TextGridBuffer::putCluster")));

        DiffPreviewModel preview;
        preview.setPatch(MockAgentTransport::inlineRendererPatch());
        QCOMPARE(preview.rowCount(), 7);
        QCOMPARE(preview.additionsLabel(), QStringLiteral("+3 additions"));

        QSignalSpy resetSpy(&preview, &QAbstractItemModel::modelReset);
        preview.setPatch(MockAgentTransport::textGridBufferPatch());
        QCOMPARE(resetSpy.count(), 0);
        QCOMPARE(preview.additionsLabel(), QStringLiteral("+4 additions"));
        QVERIFY(preview.data(preview.index(0, 0), DiffPreviewModel::LeftTextRole)
                    .toString()
                    .contains(QStringLiteral("TextGridBuffer")));
    }

    void customExternalTransportDrivesLiveDeltasAndCommitsToScrollback() {
        AgentSession session;
        auto* rawTransport = new StubExternalTransport();
        session.setTransport(std::unique_ptr<IAgentTransport>(rawTransport));
        QCOMPARE(session.transportName(), QStringLiteral("stub-acp-stdio"));

        session.submitRequest(QStringLiteral("Fix parser bug in CustomParser.cpp"));
        QVERIFY(session.isWorking());
        QCOMPARE(rawTransport->lastPrompt, QStringLiteral("Fix parser bug in CustomParser.cpp"));
        QCOMPARE(session.feedModel()->rowCount(), 1); // UserPromptCard committed

        // Stream live thought chunks before committing to Scrollback
        emit rawTransport->thoughtDelta(QStringLiteral("Inspecting "));
        emit rawTransport->thoughtDelta(QStringLiteral("CustomParser..."));
        QVERIFY(session.hasLiveStream());
        QCOMPARE(session.liveStreamKind(), int(AgentSession::LiveThoughtStream));
        QCOMPARE(session.liveStreamText(), QStringLiteral("Inspecting CustomParser..."));
        QCOMPARE(session.feedModel()->rowCount(), 1); // Not yet flushed to Scrollback

        emit rawTransport->thoughtCompleted(QStringLiteral("0.3s"));
        QVERIFY(!session.hasLiveStream());
        QCOMPARE(session.feedModel()->rowCount(), 2); // Flushed to Scrollback!
        QCOMPARE(session.feedModel()
                     ->data(session.feedModel()->index(1, 0), StreamFeedModel::ContentRole)
                     .toString(),
                 QStringLiteral("Inspecting CustomParser..."));

        // Propose arbitrary patch payload from external harness (no hardcoded file switch)
        PatchPayload customPatch;
        customPatch.filePath = QStringLiteral("src/CustomParser.cpp");
        customPatch.summaryLabel = QStringLiteral("(+1, -1 lines)");
        customPatch.unifiedHunkHeader = QStringLiteral("@@ -10,1 +10,1 @@");
        customPatch.removedLines = QStringLiteral("-return false;");
        customPatch.addedLines = QStringLiteral("+return true;");
        customPatch.sideBySideHeader = QStringLiteral("@@ -10,1 +10,1 @@ ");
        customPatch.additionsLabel = QStringLiteral("+1 addition");
        customPatch.deletionsLabel = QStringLiteral("-1 deletion");
        customPatch.sideBySideRows = {
            {QStringLiteral("-10: return false;"),
             qmltui::Color::fromString(QStringLiteral("#f87171")),
             QStringLiteral("+10: return true;"),
             qmltui::Color::fromString(QStringLiteral("#34d399")), qmltui::Color::transparent()},
        };

        emit rawTransport->diffProposed(customPatch);
        QCOMPARE(session.currentDiffFile(), QStringLiteral("src/CustomParser.cpp"));
        QCOMPARE(session.diffModel()->rowCount(), 1);
        QCOMPARE(session.diffModel()->additionsLabel(), QStringLiteral("+1 addition"));

        // Permission round-trip
        emit rawTransport->permissionRequested(42, QStringLiteral("WriteFile"),
                                               QStringLiteral("src/CustomParser.cpp"),
                                               QStringLiteral("Apply parser fix"));
        QVERIFY(session.isAwaitingPermission());
        session.choosePermission(0);
        QCOMPARE(rawTransport->resolvedPermissionId, quint64(42));
        QCOMPARE(rawTransport->lastDecision, PermissionDecision::AllowOnce);
        QVERIFY(session.isWorking());

        // Stream live assistant response chunks, then commit on completion
        emit rawTransport->responseDelta(QStringLiteral("Applied fix "));
        emit rawTransport->responseDelta(QStringLiteral("and verified."));
        QVERIFY(session.hasLiveStream());
        QCOMPARE(session.liveStreamKind(), int(AgentSession::LiveResponseStream));
        QCOMPARE(session.liveStreamText(), QStringLiteral("Applied fix and verified."));

        emit rawTransport->responseCompleted(QStringLiteral("512 tokens · 0.4s"));
        emit rawTransport->turnSucceeded(QStringLiteral("Done"), QStringLiteral("success"));
        QVERIFY(!session.hasLiveStream());
        QVERIFY(session.isReady());
        const int lastRow = session.feedModel()->rowCount() - 1;
        QCOMPARE(session.feedModel()
                     ->data(session.feedModel()->index(lastRow, 0), StreamFeedModel::ContentRole)
                     .toString(),
                 QStringLiteral("Applied fix and verified."));
    }

    void neuralModelsAndSwarmTable() {
        NeuralModelCatalog models;
        QCOMPARE(models.rowCount(), 4);
        QCOMPARE(models.indexOfModel(QStringLiteral("gpt-4.5-preview")), 1);
        QCOMPARE(models.modelIdAt(3), QStringLiteral("deepseek-r1"));

        SwarmAuditModel swarm;
        QCOMPARE(swarm.rowCount(), 3);
        QCOMPARE(swarm.columnCount(), 4);
        QCOMPARE(swarm.data(swarm.index(0, 0), Qt::DisplayRole).toString(),
                 QStringLiteral("MemoryAuditor"));
        QCOMPARE(swarm.data(swarm.index(1, 1), Qt::DisplayRole).toString(),
                 QStringLiteral("apps/agent_stream/"));
    }
};

QTEST_MAIN(TestAgentModels)
#include "test_agent_models.moc"
