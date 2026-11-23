#pragma once

#include "AgentModels.h"
#include "AgentTransport.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>
#include <memory>

class AgentSession : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(StreamFeedModel* feedModel READ feedModel CONSTANT)
    Q_PROPERTY(DiffPreviewModel* diffModel READ diffModel CONSTANT)

    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool ready READ isReady NOTIFY stateChanged)
    Q_PROPERTY(bool working READ isWorking NOTIFY stateChanged)
    Q_PROPERTY(bool awaitingPermission READ isAwaitingPermission NOTIFY stateChanged)

    Q_PROPERTY(QString transportName READ transportName NOTIFY transportChanged)
    Q_PROPERTY(QString activeModel READ activeModel WRITE setActiveModel NOTIFY activeModelChanged)
    Q_PROPERTY(QString sessionCostStr READ sessionCostStr NOTIFY usageChanged)
    Q_PROPERTY(int totalTokens READ totalTokens NOTIFY usageChanged)

    Q_PROPERTY(QString currentToolName READ currentToolName NOTIFY activityChanged)
    Q_PROPERTY(QString currentToolTarget READ currentToolTarget NOTIFY permissionChanged)
    Q_PROPERTY(QString currentToolAction READ currentToolAction NOTIFY permissionChanged)
    Q_PROPERTY(QString currentDiffFile READ currentDiffFile NOTIFY diffChanged)
    Q_PROPERTY(int permissionIndex READ permissionIndex WRITE setPermissionIndex NOTIFY
                   permissionIndexChanged)

    Q_PROPERTY(int spinnerIndex READ spinnerIndex NOTIFY tickUpdated)
    Q_PROPERTY(QString workingLabel READ workingLabel NOTIFY workingLabelChanged)

    Q_PROPERTY(int liveStreamKind READ liveStreamKind NOTIFY liveStreamChanged)
    Q_PROPERTY(bool hasLiveStream READ hasLiveStream NOTIFY liveStreamChanged)
    Q_PROPERTY(QString liveStreamTitle READ liveStreamTitle NOTIFY liveStreamChanged)
    Q_PROPERTY(QString liveStreamText READ liveStreamText NOTIFY liveStreamChanged)

    Q_PROPERTY(int promptHistoryCount READ promptHistoryCount NOTIFY promptHistoryChanged)

public:
    enum AgentState { ReadyState = 0, WorkingState, PermissionState };
    Q_ENUM(AgentState)

    enum LiveStreamKind { NoLiveStream = 0, LiveThoughtStream, LiveResponseStream };
    Q_ENUM(LiveStreamKind)

    explicit AgentSession(QObject* parent = nullptr);
    ~AgentSession() override = default;

    // Pluggable harness seam: inject any IAgentTransport implementation from C++.
    void setTransport(std::unique_ptr<IAgentTransport> transport);
    IAgentTransport* transport() const { return m_transport.get(); }
    QString transportName() const;

    StreamFeedModel* feedModel() { return &m_feedModel; }
    DiffPreviewModel* diffModel() { return &m_diffModel; }

    int state() const { return m_state; }
    bool isReady() const { return m_state == ReadyState; }
    bool isWorking() const { return m_state == WorkingState; }
    bool isAwaitingPermission() const { return m_state == PermissionState; }

    QString activeModel() const { return m_activeModel; }
    void setActiveModel(const QString& modelId);

    QString sessionCostStr() const { return m_sessionCostStr; }
    int totalTokens() const { return m_totalTokens; }

    QString currentToolName() const { return m_currentToolName; }
    QString currentToolTarget() const { return m_currentToolTarget; }
    QString currentToolAction() const { return m_currentToolAction; }
    QString currentDiffFile() const { return m_diffModel.targetFile(); }

    int permissionIndex() const { return m_permissionIndex; }
    void setPermissionIndex(int index);

    int spinnerIndex() const { return m_spinnerIndex; }
    QString workingLabel() const;

    int liveStreamKind() const { return m_liveStreamKind; }
    bool hasLiveStream() const { return m_liveStreamKind != NoLiveStream; }
    QString liveStreamTitle() const { return m_liveStreamTitle; }
    QString liveStreamText() const { return m_liveStreamText; }

    int promptHistoryCount() const { return static_cast<int>(m_promptHistory.size()); }
    Q_INVOKABLE QString promptHistoryAt(int index) const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void submitRequest(const QString& text);
    Q_INVOKABLE void choosePermission(int index);
    Q_INVOKABLE void interrupt();
    Q_INVOKABLE void selectModel(const QString& modelId);
    Q_INVOKABLE void tick(int deltaMs);

signals:
    void stateChanged();
    void transportChanged();
    void activeModelChanged();
    void usageChanged();
    void activityChanged();
    void permissionChanged();
    void diffChanged();
    void permissionIndexChanged();
    void tickUpdated();
    void workingLabelChanged();
    void liveStreamChanged();
    void promptHistoryChanged();

    void openModelPickerRequested();
    void openDiffInspectorRequested();
    void clearScrollbackRequested();
    void quitRequested();
    void toastRequested(const QString& message, const QString& status);

private:
    void wireTransport();
    void handleSlashCommand(const QString& cmd);
    void dispatchTurn(const QString& prompt, int scenarioHint = 0);
    void setState(AgentState next);
    void clearLiveStream();
    void recordPrompt(const QString& prompt);
    void addUsage(int inputTokens, int outputTokens);

    StreamFeedModel m_feedModel;
    DiffPreviewModel m_diffModel;
    std::unique_ptr<IAgentTransport> m_transport;

    AgentState m_state = ReadyState;
    quint64 m_nextTurnId = 1;
    quint64 m_pendingPermissionId = 0;
    bool m_sessionAlwaysAllow = false;

    QString m_activeModel = QStringLiteral("claude-3-7-sonnet");
    int m_totalTokens = 1420;
    QString m_sessionCostStr = QStringLiteral("$0.014");

    QString m_currentToolName;
    QString m_currentToolTarget;
    QString m_currentToolAction;
    int m_permissionIndex = 0;

    int m_spinnerIndex = 0;
    int m_elapsedDeciseconds = 0;
    int m_subDecisecondMs = 0;

    LiveStreamKind m_liveStreamKind = NoLiveStream;
    QString m_liveStreamTitle;
    QString m_liveStreamText;

    QStringList m_promptHistory;
};
