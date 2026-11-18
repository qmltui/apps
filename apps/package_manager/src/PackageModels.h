#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include <vector>

struct PackageSpec {
    QString name;
    QString version;
    QString featureKey; // "", "ptyview", "sixel", "wasm"
    QString featureLabel; // "core", "+ptyview", "+sixel", "+wasm-sdk"
    int downloadKiB = 0;
    int installKiB = 0;
    int objectCount = 8;
    bool cached = false;
    QString sha256Short;
};

class TransactionPlanModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by PackageEngine")

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        VersionRole,
        FeatureLabelRole,
        DownloadTextRole,
        InstallTextRole,
        SourceLabelRole,
        CachedRole
    };

    explicit TransactionPlanModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setPackages(std::vector<PackageSpec> packages);
    const std::vector<PackageSpec> &packages() const { return m_packages; }

    static QString formatSize(int kiB);

private:
    std::vector<PackageSpec> m_packages;
};

struct WorkerSlotState {
    bool busy = false;
    int packageIndex = -1;
    int subStep = 0; // Fetch: 0=FETCH, 1=SHA256; Build: 0=QMLGEN, 1=CXX, 2=LINK
    int progressKiB = 0;
    int targetKiB = 1;
    int elapsedSlotMs = 0;
    QString stageLabel = QStringLiteral("IDLE");
    QString stageColor = QStringLiteral("#64748b");
    QString packageLabel = QStringLiteral("waiting for runnable target");
    QString stepDetail;
    double progress = 0.0;
};

class WorkerSlotModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by PackageEngine")

public:
    enum Roles {
        SlotNumberRole = Qt::UserRole + 1,
        BusyRole,
        StageLabelRole,
        StageColorRole,
        PackageLabelRole,
        StepDetailRole,
        ProgressRole
    };

    static constexpr int kWorkerCount = 4;

    explicit WorkerSlotModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSlot(int slot, const WorkerSlotState &state);
    void resetSlots(const QString &idleText);
    const WorkerSlotState &slot(int slot) const { return m_slots[static_cast<size_t>(slot)]; }

private:
    std::vector<WorkerSlotState> m_slots;
};

struct ScrollbackEntry {
    int entryKind = 0;
    QString verb;
    QString verbColor;
    QString detail;
    QString trailing;
    QString title;
    QString subtitle;
    QString line1;
    QString line2;
    QString line3;
    QString accentColor;
};

class ScrollbackFeedModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by PackageEngine")

public:
    enum EntryKind {
        BannerEntry = 0,
        StepEntry = 1,
        PlanLockEntry = 2,
        DiagnosticEntry = 3,
        ReceiptEntry = 4
    };
    Q_ENUM(EntryKind)

    enum Roles {
        EntryKindRole = Qt::UserRole + 1,
        VerbRole,
        VerbColorRole,
        DetailRole,
        TrailingRole,
        TitleRole,
        SubtitleRole,
        Line1Role,
        Line2Role,
        Line3Role,
        AccentColorRole
    };

    explicit ScrollbackFeedModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void appendEntry(const ScrollbackEntry &entry);
    void clearEntries();

private:
    std::vector<ScrollbackEntry> m_entries;
};

class PackageEngine : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString targetName READ targetName NOTIFY planChanged)
    Q_PROPERTY(QString targetVersion READ targetVersion CONSTANT)
    Q_PROPERTY(int phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(bool autoExit READ autoExit NOTIFY configChanged)
    Q_PROPERTY(bool ptyviewEnabled READ ptyviewEnabled WRITE setPtyviewEnabled NOTIFY planChanged)
    Q_PROPERTY(bool sixelEnabled READ sixelEnabled WRITE setSixelEnabled NOTIFY planChanged)
    Q_PROPERTY(bool wasmEnabled READ wasmEnabled WRITE setWasmEnabled NOTIFY planChanged)
    Q_PROPERTY(bool showTreePreview READ showTreePreview WRITE setShowTreePreview NOTIFY uiStateChanged)
    Q_PROPERTY(bool showVerboseLog READ showVerboseLog WRITE setShowVerboseLog NOTIFY uiStateChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY uiStateChanged)

    Q_PROPERTY(int totalPackages READ totalPackages NOTIFY planChanged)
    Q_PROPERTY(int cachedPackages READ cachedPackages NOTIFY planChanged)
    Q_PROPERTY(int completedPackages READ completedPackages NOTIFY telemetryChanged)
    Q_PROPERTY(int treeRowCount READ treeRowCount NOTIFY planChanged)

    Q_PROPERTY(QString totalDownloadText READ totalDownloadText NOTIFY planChanged)
    Q_PROPERTY(QString totalInstallText READ totalInstallText NOTIFY planChanged)
    Q_PROPERTY(QString activeFeaturesText READ activeFeaturesText NOTIFY planChanged)
    Q_PROPERTY(QString progressSummaryText READ progressSummaryText NOTIFY telemetryChanged)
    Q_PROPERTY(QString throughputText READ throughputText NOTIFY telemetryChanged)
    Q_PROPERTY(QString etaText READ etaText NOTIFY telemetryChanged)
    Q_PROPERTY(QString elapsedTotalText READ elapsedTotalText NOTIFY telemetryChanged)

    Q_PROPERTY(double overallProgress READ overallProgress NOTIFY telemetryChanged)
    Q_PROPERTY(double phaseProgress READ phaseProgress NOTIFY telemetryChanged)
    Q_PROPERTY(QVariantList throughputHistory READ throughputHistory NOTIFY telemetryChanged)
    Q_PROPERTY(QVariantMap dependencyTree READ dependencyTree NOTIFY planChanged)
    Q_PROPERTY(QStringList liveLogLines READ liveLogLines NOTIFY telemetryChanged)

    Q_PROPERTY(TransactionPlanModel *planModel READ planModel CONSTANT)
    Q_PROPERTY(WorkerSlotModel *workerModel READ workerModel CONSTANT)
    Q_PROPERTY(ScrollbackFeedModel *feedModel READ feedModel CONSTANT)

public:
    enum Phase {
        PromptPhase = 0,
        FetchingPhase = 1,
        BuildingPhase = 2,
        FinishedPhase = 3
    };
    Q_ENUM(Phase)

    explicit PackageEngine(QObject *parent = nullptr);

    QString targetName() const { return m_targetName; }
    QString targetVersion() const { return m_targetVersion; }
    int phase() const { return m_phase; }
    bool autoExit() const { return m_autoExit; }

    bool ptyviewEnabled() const { return m_ptyviewEnabled; }
    void setPtyviewEnabled(bool enabled);

    bool sixelEnabled() const { return m_sixelEnabled; }
    void setSixelEnabled(bool enabled);

    bool wasmEnabled() const { return m_wasmEnabled; }
    void setWasmEnabled(bool enabled);

    bool showTreePreview() const { return m_showTreePreview; }
    void setShowTreePreview(bool show);

    bool showVerboseLog() const { return m_showVerboseLog; }
    void setShowVerboseLog(bool show);

    bool paused() const { return m_paused; }
    void setPaused(bool paused);

    int totalPackages() const { return static_cast<int>(m_planModel->packages().size()); }
    int cachedPackages() const;
    int completedPackages() const { return m_completedInPhase; }
    int treeRowCount() const { return m_treeRowCount; }

    QString totalDownloadText() const;
    QString totalInstallText() const;
    QString activeFeaturesText() const;
    QString progressSummaryText() const;
    QString throughputText() const { return m_throughputText; }
    QString etaText() const { return m_etaText; }
    QString elapsedTotalText() const;

    double overallProgress() const;
    double phaseProgress() const { return m_phaseProgress; }
    QVariantList throughputHistory() const { return m_throughputHistory; }
    QVariantMap dependencyTree() const { return m_dependencyTree; }
    QStringList liveLogLines() const { return m_liveLogLines; }

    TransactionPlanModel *planModel() const { return m_planModel; }
    WorkerSlotModel *workerModel() const { return m_workerModel; }
    ScrollbackFeedModel *feedModel() const { return m_feedModel; }

    Q_INVOKABLE void initializeFromArgs(const QStringList &args);
    Q_INVOKABLE void toggleFeature(int featureIndex);
    Q_INVOKABLE void toggleTreePreview();
    Q_INVOKABLE void toggleVerboseLog();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void confirmInstall();
    Q_INVOKABLE void restartSession();
    Q_INVOKABLE void tick(int deltaMs);

signals:
    void phaseChanged();
    void planChanged();
    void uiStateChanged();
    void telemetryChanged();
    void configChanged();
    void finishedAndAutoExit();

private:
    void rebuildPlan();
    void appendBanner();
    void appendStep(const QString &verb, const QString &verbColor, const QString &detail,
                    const QString &trailing = QString());
    void pushLiveLog(const QString &line);
    void pushThroughputSample(double sample);
    void startFetchingPhase();
    void startBuildingPhase();
    void finishInstallation();
    void assignNextFetchJob(int slot);
    void assignNextBuildJob(int slot);
    void advanceFetchSlot(int slot, int deltaMs);
    void advanceBuildSlot(int slot, int deltaMs);
    void recomputePhaseTelemetry();

    QString m_targetName = QStringLiteral("qpm-studio");
    QString m_targetVersion = QStringLiteral("0.6.0");
    int m_phase = PromptPhase;
    bool m_autoConfirm = false;
    bool m_autoExit = false;

    bool m_ptyviewEnabled = true;
    bool m_sixelEnabled = true;
    bool m_wasmEnabled = false;
    bool m_showTreePreview = false;
    bool m_showVerboseLog = false;
    bool m_paused = false;
    bool m_diagnosticEmitted = false;

    int m_totalDownloadKiB = 0;
    int m_totalInstallKiB = 0;
    int m_treeRowCount = 8;

    int m_nextQueueIndex = 0;
    int m_completedInPhase = 0;
    int m_fetchElapsedMs = 0;
    int m_buildElapsedMs = 0;
    int m_tickCounter = 0;
    double m_phaseProgress = 0.0;

    QString m_throughputText = QStringLiteral("0.0 MiB/s");
    QString m_etaText = QStringLiteral("--");
    QVariantList m_throughputHistory;
    QVariantMap m_dependencyTree;
    QStringList m_liveLogLines;

    std::vector<PackageSpec> m_catalog;
    TransactionPlanModel *m_planModel = nullptr;
    WorkerSlotModel *m_workerModel = nullptr;
    ScrollbackFeedModel *m_feedModel = nullptr;
};
