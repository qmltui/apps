#pragma once

#include "AgentTransport.h"

class MockAgentTransport : public IAgentTransport {
    Q_OBJECT

public:
    explicit MockAgentTransport(QObject* parent = nullptr);

    QString transportName() const override { return QStringLiteral("mock-stream (stub harness)"); }

    void startTurn(quint64 turnId, const QString& prompt, const QString& modelId,
                   int scenarioHint = 0) override;
    void resolvePermission(quint64 permissionId, PermissionDecision decision) override;
    void cancelTurn() override;
    void reset() override;
    void tick(int deltaMs) override;

    static PatchPayload inlineRendererPatch();
    static PatchPayload textGridBufferPatch();

private:
    static int inferScenario(const QString& prompt);
    void stepScenario1();
    void stepScenario2();
    void stepScenario3();
    void stepScenario4();
    void requestStepPermission(const QString& toolName, const QString& target,
                               const QString& actionSummary, const QString& resumeActivity);

    bool m_running = false;
    bool m_awaitingPermission = false;
    quint64 m_turnId = 0;
    quint64 m_nextPermissionId = 1;
    quint64 m_pendingPermissionId = 0;
    int m_scenario = 1;
    int m_phase = 0; // 0 = pre-permission / main phase, 1 = post-permission phase
    int m_phaseTick = 0;
    int m_accumulatedMs = 0;
    QString m_resumeActivity;
};
