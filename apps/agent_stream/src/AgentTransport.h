#pragma once

#include <QmlTui/Core/Primitives/Color.h>

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <vector>

struct PatchSideBySideLine {
    QString leftText;
    qmltui::Color leftColor;
    QString rightText;
    qmltui::Color rightColor;
    qmltui::Color rowBg;
};

struct PatchPayload {
    QString filePath;
    QString summaryLabel;
    QString unifiedHunkHeader;
    QString removedLines;
    QString addedLines;
    QString sideBySideHeader;
    QString additionsLabel;
    QString deletionsLabel;
    std::vector<PatchSideBySideLine> sideBySideRows;
};

enum class PermissionDecision { AllowOnce = 0, AlwaysAllow = 1, Deny = 2, Cancel = 3 };

// Protocol-agnostic transport contract between AgentSession and an agent runtime
// (built-in MockAgentTransport today; ACP stdio, CLI stream-json, or HTTP SSE later).
class IAgentTransport : public QObject {
    Q_OBJECT

public:
    explicit IAgentTransport(QObject* parent = nullptr) : QObject(parent) {}
    ~IAgentTransport() override = default;

    virtual QString transportName() const = 0;

    virtual void startTurn(quint64 turnId, const QString& prompt, const QString& modelId,
                           int scenarioHint = 0) = 0;
    virtual void resolvePermission(quint64 permissionId, PermissionDecision decision) = 0;
    virtual void cancelTurn() = 0;
    virtual void reset() = 0;

    // Optional deterministic clock hook for simulated transports under CLOCK VIRTUAL.
    // Real asynchronous I/O transports (QProcess / QNetworkReply) leave this as a no-op.
    virtual void tick(int deltaMs) { Q_UNUSED(deltaMs); }

signals:
    void activityChanged(const QString& activityLabel);
    void thoughtDelta(const QString& chunk);
    void thoughtCompleted(const QString& durationLabel);
    void responseDelta(const QString& chunk);
    void responseCompleted(const QString& summaryLabel);
    void toolCallStarted(const QString& toolName, const QString& argsSummary);
    void toolCallCompleted(const QString& resultSummary);
    void diffProposed(const PatchPayload& patch);
    void permissionRequested(quint64 permissionId, const QString& toolName, const QString& target,
                             const QString& actionSummary);
    void swarmAuditReady(const QString& title);
    void repoTreeReady(const QString& title);
    void usageReported(int inputTokens, int outputTokens);
    void turnSucceeded(const QString& toastMessage, const QString& toastStatus);
    void transportError(const QString& message);
};
