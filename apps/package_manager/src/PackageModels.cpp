#include "PackageModels.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// TransactionPlanModel
// ---------------------------------------------------------------------------

TransactionPlanModel::TransactionPlanModel(QObject *parent)
    : QAbstractListModel(parent) {}

int TransactionPlanModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_packages.size());
}

QVariant TransactionPlanModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0
        || index.row() >= static_cast<int>(m_packages.size())) {
        return {};
    }

    const PackageSpec &pkg = m_packages[static_cast<size_t>(index.row())];
    switch (role) {
    case NameRole:
        return pkg.name;
    case VersionRole:
        return pkg.version;
    case FeatureLabelRole:
        return pkg.featureLabel;
    case DownloadTextRole:
        return pkg.cached ? QStringLiteral("0 KiB") : formatSize(pkg.downloadKiB);
    case InstallTextRole:
        return formatSize(pkg.installKiB);
    case SourceLabelRole:
        return pkg.cached ? QStringLiteral("cache hit") : QStringLiteral("registry");
    case CachedRole:
        return pkg.cached;
    default:
        return {};
    }
}

QHash<int, QByteArray> TransactionPlanModel::roleNames() const {
    return {
        {NameRole, "name"},
        {VersionRole, "version"},
        {FeatureLabelRole, "featureLabel"},
        {DownloadTextRole, "downloadText"},
        {InstallTextRole, "installText"},
        {SourceLabelRole, "sourceLabel"},
        {CachedRole, "cached"},
    };
}

void TransactionPlanModel::setPackages(std::vector<PackageSpec> packages) {
    beginResetModel();
    m_packages = std::move(packages);
    endResetModel();
}

QString TransactionPlanModel::formatSize(int kiB) {
    if (kiB >= 1024)
        return QString::number(static_cast<double>(kiB) / 1024.0, 'f', 1) + QStringLiteral(" MiB");
    return QString::number(kiB) + QStringLiteral(" KiB");
}

// ---------------------------------------------------------------------------
// WorkerSlotModel
// ---------------------------------------------------------------------------

WorkerSlotModel::WorkerSlotModel(QObject *parent)
    : QAbstractListModel(parent), m_slots(kWorkerCount) {}

int WorkerSlotModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_slots.size());
}

QVariant WorkerSlotModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0
        || index.row() >= static_cast<int>(m_slots.size())) {
        return {};
    }

    const WorkerSlotState &s = m_slots[static_cast<size_t>(index.row())];
    switch (role) {
    case SlotNumberRole:
        return index.row() + 1;
    case BusyRole:
        return s.busy;
    case StageLabelRole:
        return s.stageLabel;
    case StageColorRole:
        return s.stageColor;
    case PackageLabelRole:
        return s.packageLabel;
    case StepDetailRole:
        return s.stepDetail;
    case ProgressRole:
        return s.progress;
    default:
        return {};
    }
}

QHash<int, QByteArray> WorkerSlotModel::roleNames() const {
    return {
        {SlotNumberRole, "slotNumber"},
        {BusyRole, "busy"},
        {StageLabelRole, "stageLabel"},
        {StageColorRole, "stageColor"},
        {PackageLabelRole, "packageLabel"},
        {StepDetailRole, "stepDetail"},
        {ProgressRole, "progress"},
    };
}

void WorkerSlotModel::setSlot(int slot, const WorkerSlotState &state) {
    if (slot < 0 || slot >= static_cast<int>(m_slots.size()))
        return;
    m_slots[static_cast<size_t>(slot)] = state;
    const QModelIndex idx = index(slot, 0);
    emit dataChanged(idx, idx);
}

void WorkerSlotModel::resetSlots(const QString &idleText) {
    beginResetModel();
    for (auto &s : m_slots) {
        s = WorkerSlotState{};
        s.packageLabel = idleText;
    }
    endResetModel();
}

// ---------------------------------------------------------------------------
// ScrollbackFeedModel
// ---------------------------------------------------------------------------

ScrollbackFeedModel::ScrollbackFeedModel(QObject *parent)
    : QAbstractListModel(parent) {}

int ScrollbackFeedModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_entries.size());
}

QVariant ScrollbackFeedModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0
        || index.row() >= static_cast<int>(m_entries.size())) {
        return {};
    }

    const ScrollbackEntry &e = m_entries[static_cast<size_t>(index.row())];
    switch (role) {
    case EntryKindRole:
        return e.entryKind;
    case VerbRole:
        return e.verb;
    case VerbColorRole:
        return e.verbColor;
    case DetailRole:
        return e.detail;
    case TrailingRole:
        return e.trailing;
    case TitleRole:
        return e.title;
    case SubtitleRole:
        return e.subtitle;
    case Line1Role:
        return e.line1;
    case Line2Role:
        return e.line2;
    case Line3Role:
        return e.line3;
    case AccentColorRole:
        return e.accentColor;
    default:
        return {};
    }
}

QHash<int, QByteArray> ScrollbackFeedModel::roleNames() const {
    return {
        {EntryKindRole, "entryKind"},
        {VerbRole, "verb"},
        {VerbColorRole, "verbColor"},
        {DetailRole, "detail"},
        {TrailingRole, "trailing"},
        {TitleRole, "cardTitle"},
        {SubtitleRole, "cardSubtitle"},
        {Line1Role, "line1"},
        {Line2Role, "line2"},
        {Line3Role, "line3"},
        {AccentColorRole, "accentColor"},
    };
}

void ScrollbackFeedModel::appendEntry(const ScrollbackEntry &entry) {
    const int row = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), row, row);
    m_entries.push_back(entry);
    endInsertRows();
}

void ScrollbackFeedModel::clearEntries() {
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

// ---------------------------------------------------------------------------
// PackageEngine
// ---------------------------------------------------------------------------

PackageEngine::PackageEngine(QObject *parent)
    : QObject(parent)
    , m_planModel(new TransactionPlanModel(this))
    , m_workerModel(new WorkerSlotModel(this))
    , m_feedModel(new ScrollbackFeedModel(this)) {
    m_catalog = {
        {QStringLiteral("utf8proc"), QStringLiteral("2.9.0"), QString(), QStringLiteral("core"),
         640, 1420, 4, true, QStringLiteral("9f4c..e2a1")},
        {QStringLiteral("md4qt"), QStringLiteral("2.8.0"), QString(), QStringLiteral("core"),
         1840, 4200, 6, true, QStringLiteral("3a81..7b09")},
        {QStringLiteral("qcoro-core"), QStringLiteral("0.11.0"), QString(), QStringLiteral("core"),
         2860, 6800, 9, false, QStringLiteral("71de..44c0")},
        {QStringLiteral("qmltui-core"), QStringLiteral("0.6.0"), QString(), QStringLiteral("core"),
         4920, 12400, 18, false, QStringLiteral("e502..19fa")},
        {QStringLiteral("qmltui-controls"), QStringLiteral("0.6.0"), QString(),
         QStringLiteral("core"), 4100, 9800, 14, false, QStringLiteral("b83c..6d12")},
        {QStringLiteral("qt6-shadertools"), QStringLiteral("6.8.3"), QString(),
         QStringLiteral("core"), 8640, 21200, 16, false, QStringLiteral("4c90..88ef")},
        {QStringLiteral("qmltui-ptyview"), QStringLiteral("0.6.0"), QStringLiteral("ptyview"),
         QStringLiteral("+ptyview"), 2150, 5300, 8, false, QStringLiteral("d17a..03b4")},
        {QStringLiteral("qmltui-image"), QStringLiteral("0.6.0"), QStringLiteral("sixel"),
         QStringLiteral("+sixel"), 4520, 10800, 12, false, QStringLiteral("62fa..c518")},
        {QStringLiteral("qmltui-wasm-shell"), QStringLiteral("0.6.0"), QStringLiteral("wasm"),
         QStringLiteral("+wasm-sdk"), 3680, 8400, 10, false, QStringLiteral("10be..9a77")},
        {QStringLiteral("qt6-wasm-runtime"), QStringLiteral("6.8.3"), QStringLiteral("wasm"),
         QStringLiteral("+wasm-sdk"), 7960, 19400, 15, false, QStringLiteral("88d4..51ce")},
    };

    for (int i = 0; i < 16; ++i)
        m_throughputHistory.append(0.0);

    rebuildPlan();
    appendBanner();
}

void PackageEngine::initializeFromArgs(const QStringList &args) {
    bool changed = false;
    for (int i = 1; i < args.size(); ++i) {
        const QString &arg = args[i];
        if (arg == QLatin1String("--auto-exit")) {
            m_autoExit = true;
            changed = true;
        } else if (arg == QLatin1String("-y") || arg == QLatin1String("--yes")) {
            m_autoConfirm = true;
            changed = true;
        } else if (arg == QLatin1String("--with-wasm")) {
            m_wasmEnabled = true;
            rebuildPlan();
        } else if (!arg.startsWith(u'-') && arg != QLatin1String("install")) {
            m_targetName = arg;
            rebuildPlan();
        }
    }
    if (changed)
        emit configChanged();
    if (m_autoConfirm && m_phase == PromptPhase)
        confirmInstall();
}

void PackageEngine::setPtyviewEnabled(bool enabled) {
    if (m_phase != PromptPhase || m_ptyviewEnabled == enabled)
        return;
    m_ptyviewEnabled = enabled;
    rebuildPlan();
}

void PackageEngine::setSixelEnabled(bool enabled) {
    if (m_phase != PromptPhase || m_sixelEnabled == enabled)
        return;
    m_sixelEnabled = enabled;
    rebuildPlan();
}

void PackageEngine::setWasmEnabled(bool enabled) {
    if (m_phase != PromptPhase || m_wasmEnabled == enabled)
        return;
    m_wasmEnabled = enabled;
    rebuildPlan();
}

void PackageEngine::setShowTreePreview(bool show) {
    if (m_showTreePreview == show)
        return;
    m_showTreePreview = show;
    emit uiStateChanged();
}

void PackageEngine::setShowVerboseLog(bool show) {
    if (m_showVerboseLog == show)
        return;
    m_showVerboseLog = show;
    emit uiStateChanged();
}

void PackageEngine::setPaused(bool paused) {
    if (m_paused == paused)
        return;
    m_paused = paused;
    emit uiStateChanged();
}

int PackageEngine::cachedPackages() const {
    int count = 0;
    for (const auto &pkg : m_planModel->packages()) {
        if (pkg.cached)
            ++count;
    }
    return count;
}

QString PackageEngine::totalDownloadText() const {
    return TransactionPlanModel::formatSize(m_totalDownloadKiB);
}

QString PackageEngine::totalInstallText() const {
    return TransactionPlanModel::formatSize(m_totalInstallKiB);
}

QString PackageEngine::activeFeaturesText() const {
    QStringList list;
    if (m_ptyviewEnabled)
        list.append(QStringLiteral("ptyview"));
    if (m_sixelEnabled)
        list.append(QStringLiteral("sixel"));
    if (m_wasmEnabled)
        list.append(QStringLiteral("wasm-sdk"));
    return list.isEmpty() ? QStringLiteral("default (minimal)") : list.join(QStringLiteral(", "));
}

QString PackageEngine::progressSummaryText() const {
    const auto &pkgs = m_planModel->packages();
    if (m_phase == FetchingPhase) {
        int fetchedKiB = 0;
        for (size_t i = 0; i < pkgs.size(); ++i) {
            if (pkgs[i].cached)
                continue;
            bool activeOrQueued = false;
            for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s) {
                const auto &ws = m_workerModel->slot(s);
                if (ws.busy && ws.packageIndex == static_cast<int>(i)) {
                    fetchedKiB += ws.progressKiB;
                    activeOrQueued = true;
                    break;
                }
            }
            if (!activeOrQueued && static_cast<int>(i) < m_nextQueueIndex)
                fetchedKiB += pkgs[i].downloadKiB;
        }
        fetchedKiB = std::clamp(fetchedKiB, 0, m_totalDownloadKiB);
        return TransactionPlanModel::formatSize(fetchedKiB) + QStringLiteral(" / ")
               + TransactionPlanModel::formatSize(m_totalDownloadKiB);
    }

    if (m_phase == BuildingPhase) {
        return QString::number(m_completedInPhase) + QStringLiteral(" / ")
               + QString::number(totalPackages()) + QStringLiteral(" targets compiled");
    }

    return TransactionPlanModel::formatSize(m_totalInstallKiB);
}

QString PackageEngine::elapsedTotalText() const {
    const double secs = static_cast<double>(m_fetchElapsedMs + m_buildElapsedMs) / 1000.0;
    return QString::number(secs, 'f', 1) + QStringLiteral("s");
}

double PackageEngine::overallProgress() const {
    if (m_phase == PromptPhase)
        return 0.0;
    if (m_phase == FinishedPhase)
        return 1.0;
    if (m_phase == FetchingPhase)
        return m_phaseProgress * 0.45;
    return 0.45 + (m_phaseProgress * 0.55);
}

void PackageEngine::toggleFeature(int featureIndex) {
    if (m_phase != PromptPhase)
        return;
    if (featureIndex == 0)
        setPtyviewEnabled(!m_ptyviewEnabled);
    else if (featureIndex == 1)
        setSixelEnabled(!m_sixelEnabled);
    else if (featureIndex == 2)
        setWasmEnabled(!m_wasmEnabled);
}

void PackageEngine::toggleTreePreview() {
    if (m_phase == PromptPhase)
        setShowTreePreview(!m_showTreePreview);
}

void PackageEngine::toggleVerboseLog() {
    if (m_phase == FetchingPhase || m_phase == BuildingPhase)
        setShowVerboseLog(!m_showVerboseLog);
}

void PackageEngine::togglePause() {
    if (m_phase == FetchingPhase || m_phase == BuildingPhase)
        setPaused(!m_paused);
}

void PackageEngine::confirmInstall() {
    if (m_phase != PromptPhase)
        return;

    ScrollbackEntry lockCard;
    lockCard.entryKind = ScrollbackFeedModel::PlanLockEntry;
    lockCard.title = QStringLiteral("Locked Transaction Plan");
    lockCard.subtitle = QString::number(totalPackages()) + QStringLiteral(" packages · ")
                        + totalDownloadText() + QStringLiteral(" fetch");
    lockCard.line1 = QStringLiteral("Features   ") + activeFeaturesText()
                     + QStringLiteral("  ·  Profile: release [lto + qmlcachegen]");
    lockCard.line2 = QStringLiteral("Footprint  Download ") + totalDownloadText()
                     + QStringLiteral(" (") + QString::number(cachedPackages())
                     + QStringLiteral(" cached)  ·  Install +") + totalInstallText()
                     + QStringLiteral("  ->  ~/.qpm/bin/") + m_targetName;
    lockCard.accentColor = QStringLiteral("#38bdf8");
    m_feedModel->appendEntry(lockCard);

    appendStep(QStringLiteral("Resolved"), QStringLiteral("#38bdf8"),
               QString::number(totalPackages()) + QStringLiteral(" packages from qt-registry.io"),
               QStringLiteral("[lockfile v3 ✓]"));

    startFetchingPhase();
}

void PackageEngine::restartSession() {
    m_phase = PromptPhase;
    m_paused = false;
    m_showVerboseLog = false;
    m_showTreePreview = false;
    m_diagnosticEmitted = false;
    m_fetchElapsedMs = 0;
    m_buildElapsedMs = 0;
    m_completedInPhase = 0;
    m_nextQueueIndex = 0;
    m_phaseProgress = 0.0;
    m_liveLogLines.clear();
    m_throughputHistory.clear();
    for (int i = 0; i < 16; ++i)
        m_throughputHistory.append(0.0);

    m_feedModel->clearEntries();
    rebuildPlan();
    appendBanner();
    emit phaseChanged();
    emit uiStateChanged();
    emit telemetryChanged();
}

void PackageEngine::rebuildPlan() {
    std::vector<PackageSpec> selected;
    m_totalDownloadKiB = 0;
    m_totalInstallKiB = 0;

    for (const auto &pkg : m_catalog) {
        if (pkg.featureKey == QLatin1String("ptyview") && !m_ptyviewEnabled)
            continue;
        if (pkg.featureKey == QLatin1String("sixel") && !m_sixelEnabled)
            continue;
        if (pkg.featureKey == QLatin1String("wasm") && !m_wasmEnabled)
            continue;
        selected.push_back(pkg);
        if (!pkg.cached)
            m_totalDownloadKiB += pkg.downloadKiB;
        m_totalInstallKiB += pkg.installKiB;
    }

    m_planModel->setPackages(std::move(selected));

    // Build hierarchical dependency tree for TreeView
    QVariantList coreChildren;
    coreChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("utf8proc v2.9.0 (cached)")},
                                    {QStringLiteral("expanded"), false}});
    coreChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("qcoro-core v0.11.0")},
                                    {QStringLiteral("expanded"), false}});

    QVariantList controlsChildren;
    controlsChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("md4qt v2.8.0 (cached)")},
                                        {QStringLiteral("expanded"), false}});
    controlsChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("qt6-shadertools v6.8.3")},
                                        {QStringLiteral("expanded"), false}});

    QVariantList topChildren;
    topChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("qmltui-core v0.6.0")},
                                   {QStringLiteral("expanded"), true},
                                   {QStringLiteral("children"), coreChildren}});
    topChildren.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("qmltui-controls v0.6.0")},
                                   {QStringLiteral("expanded"), true},
                                   {QStringLiteral("children"), controlsChildren}});

    int rows = 1 + 2 + 2 + 2; // root + core(2) + controls(2)
    if (m_ptyviewEnabled) {
        topChildren.append(QVariantMap{
            {QStringLiteral("text"), QStringLiteral("qmltui-ptyview v0.6.0 [+ptyview]")},
            {QStringLiteral("expanded"), false}});
        ++rows;
    }
    if (m_sixelEnabled) {
        topChildren.append(QVariantMap{
            {QStringLiteral("text"), QStringLiteral("qmltui-image v0.6.0 [+sixel]")},
            {QStringLiteral("expanded"), false}});
        ++rows;
    }
    if (m_wasmEnabled) {
        QVariantList wasmChildren;
        wasmChildren.append(QVariantMap{
            {QStringLiteral("text"), QStringLiteral("qt6-wasm-runtime v6.8.3")},
            {QStringLiteral("expanded"), false}});
        topChildren.append(QVariantMap{
            {QStringLiteral("text"), QStringLiteral("qmltui-wasm-shell v0.6.0 [+wasm-sdk]")},
            {QStringLiteral("expanded"), true},
            {QStringLiteral("children"), wasmChildren}});
        rows += 2;
    }

    m_treeRowCount = rows;
    m_dependencyTree = QVariantMap{
        {QStringLiteral("text"), m_targetName + QStringLiteral(" v") + m_targetVersion},
        {QStringLiteral("expanded"), true},
        {QStringLiteral("children"), topChildren},
    };

    emit planChanged();
    emit telemetryChanged();
}

void PackageEngine::appendBanner() {
    ScrollbackEntry banner;
    banner.entryKind = ScrollbackFeedModel::BannerEntry;
    banner.title = QStringLiteral("qpm install ") + m_targetName + QStringLiteral(" --release");
    banner.subtitle =
        QStringLiteral("qt-registry.io · linux-x86_64 · Qt 6.8+ · 4 parallel workers");
    m_feedModel->appendEntry(banner);
}

void PackageEngine::appendStep(const QString &verb, const QString &verbColor,
                               const QString &detail, const QString &trailing) {
    ScrollbackEntry step;
    step.entryKind = ScrollbackFeedModel::StepEntry;
    step.verb = verb;
    step.verbColor = verbColor;
    step.detail = detail;
    step.trailing = trailing;
    m_feedModel->appendEntry(step);
}

void PackageEngine::pushLiveLog(const QString &line) {
    m_liveLogLines.append(line);
    while (m_liveLogLines.size() > 4)
        m_liveLogLines.removeFirst();
}

void PackageEngine::pushThroughputSample(double sample) {
    m_throughputHistory.append(sample);
    while (m_throughputHistory.size() > 16)
        m_throughputHistory.removeFirst();
}

void PackageEngine::startFetchingPhase() {
    m_phase = FetchingPhase;
    m_nextQueueIndex = 0;
    m_completedInPhase = 0;
    m_phaseProgress = 0.0;
    m_tickCounter = 0;
    m_liveLogLines.clear();
    pushLiveLog(QStringLiteral("[http/2] connected to cdn.qt-registry.io:443 (TLS 1.3, 4 streams)"));
    pushLiveLog(QStringLiteral("[keyring] loaded 3 trusted Ed25519 signing keys"));

    m_workerModel->resetSlots(QStringLiteral("idle · awaiting mirror stream"));
    for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s)
        assignNextFetchJob(s);

    recomputePhaseTelemetry();
    emit phaseChanged();
    emit telemetryChanged();
}

void PackageEngine::assignNextFetchJob(int slot) {
    const auto &pkgs = m_planModel->packages();
    while (m_nextQueueIndex < static_cast<int>(pkgs.size())) {
        const int idx = m_nextQueueIndex++;
        const PackageSpec &pkg = pkgs[static_cast<size_t>(idx)];
        if (pkg.cached) {
            ++m_completedInPhase;
            appendStep(QStringLiteral("Cached"), QStringLiteral("#38bdf8"),
                       pkg.name + QStringLiteral(" v") + pkg.version,
                       QStringLiteral("[sha256:") + pkg.sha256Short + QStringLiteral(" ✓]"));
            pushLiveLog(QStringLiteral("[cache] hit ") + pkg.name + QStringLiteral("-")
                        + pkg.version + QStringLiteral(".qpkg (sha256:") + pkg.sha256Short
                        + QStringLiteral(")"));
            continue;
        }

        WorkerSlotState ws;
        ws.busy = true;
        ws.packageIndex = idx;
        ws.subStep = 0;
        ws.progressKiB = 0;
        ws.targetKiB = pkg.downloadKiB;
        ws.elapsedSlotMs = 0;
        ws.stageLabel = QStringLiteral("FETCH");
        ws.stageColor = QStringLiteral("#38bdf8");
        ws.packageLabel = pkg.name + QStringLiteral(" v") + pkg.version;
        ws.stepDetail = QStringLiteral("0 KiB / ") + TransactionPlanModel::formatSize(pkg.downloadKiB);
        ws.progress = 0.0;
        m_workerModel->setSlot(slot, ws);
        pushLiveLog(QStringLiteral("[fetch#") + QString::number(slot + 1)
                    + QStringLiteral("] GET /packages/") + pkg.name + QStringLiteral("/")
                    + pkg.version + QStringLiteral(".tar.zst"));
        return;
    }

    WorkerSlotState idle;
    idle.busy = false;
    idle.stageLabel = QStringLiteral("IDLE");
    idle.stageColor = QStringLiteral("#64748b");
    idle.packageLabel = QStringLiteral("mirror stream drained");
    idle.stepDetail = QStringLiteral("ready");
    idle.progress = 1.0;
    m_workerModel->setSlot(slot, idle);
}

void PackageEngine::advanceFetchSlot(int slot, int deltaMs) {
    WorkerSlotState ws = m_workerModel->slot(slot);
    if (!ws.busy || ws.packageIndex < 0)
        return;

    const auto &pkgs = m_planModel->packages();
    const PackageSpec &pkg = pkgs[static_cast<size_t>(ws.packageIndex)];
    ws.elapsedSlotMs += deltaMs;

    if (ws.subStep == 0) {
        // Staggered per-slot download rate (~270-380 KiB per 50ms tick -> ~22-30 MiB/s total)
        const int baseRate = 270 + (slot * 34) + ((m_tickCounter + slot) % 3) * 22;
        ws.progressKiB = std::min(pkg.downloadKiB, ws.progressKiB + baseRate);
        ws.progress = static_cast<double>(ws.progressKiB) / std::max(1, pkg.downloadKiB) * 0.88;
        ws.stepDetail = TransactionPlanModel::formatSize(ws.progressKiB) + QStringLiteral(" / ")
                        + TransactionPlanModel::formatSize(pkg.downloadKiB);

        if (ws.progressKiB >= pkg.downloadKiB) {
            ws.subStep = 1;
            ws.progressKiB = 0;
            ws.targetKiB = 3; // 3 ticks of signature & sha256 verification
            ws.stageLabel = QStringLiteral("VERIFY");
            ws.stageColor = QStringLiteral("#a78bfa");
            ws.stepDetail = QStringLiteral("ed25519 + sha256");
            ws.progress = 0.92;
            pushLiveLog(QStringLiteral("[verify] checking Ed25519 sig for ") + pkg.name
                        + QStringLiteral(" (") + pkg.sha256Short + QStringLiteral(")"));
        }
        m_workerModel->setSlot(slot, ws);
    } else {
        ws.progressKiB += 1;
        ws.progress = std::min(1.0, 0.90 + 0.04 * ws.progressKiB);
        if (ws.progressKiB >= ws.targetKiB) {
            ++m_completedInPhase;
            const QString secs =
                QString::number(static_cast<double>(ws.elapsedSlotMs) / 1000.0, 'f', 1)
                + QStringLiteral("s");
            appendStep(QStringLiteral("Fetched"), QStringLiteral("#34d399"),
                       pkg.name + QStringLiteral(" v") + pkg.version + QStringLiteral(" (")
                           + TransactionPlanModel::formatSize(pkg.downloadKiB) + QStringLiteral(")"),
                       QStringLiteral("[ed25519 ✓ · ") + secs + QStringLiteral("]"));
            assignNextFetchJob(slot);
            return;
        }
        m_workerModel->setSlot(slot, ws);
    }
}

void PackageEngine::startBuildingPhase() {
    appendStep(QStringLiteral("Verified"), QStringLiteral("#a78bfa"),
               QString::number(totalPackages()) + QStringLiteral("/")
                   + QString::number(totalPackages())
                   + QStringLiteral(" Ed25519 package signatures & SHA-256 digests"),
               QStringLiteral("[0 warnings]"));

    m_phase = BuildingPhase;
    m_nextQueueIndex = 0;
    m_completedInPhase = 0;
    m_phaseProgress = 0.0;
    pushLiveLog(QStringLiteral("[cmake] configuring Ninja build graph (-DCMAKE_BUILD_TYPE=Release)"));

    m_workerModel->resetSlots(QStringLiteral("idle · awaiting build graph"));
    for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s)
        assignNextBuildJob(s);

    recomputePhaseTelemetry();
    emit phaseChanged();
    emit telemetryChanged();
}

void PackageEngine::assignNextBuildJob(int slot) {
    const auto &pkgs = m_planModel->packages();
    if (m_nextQueueIndex < static_cast<int>(pkgs.size())) {
        const int idx = m_nextQueueIndex++;
        const PackageSpec &pkg = pkgs[static_cast<size_t>(idx)];
        const bool hasQml = pkg.name.startsWith(QLatin1String("qmltui"));

        WorkerSlotState ws;
        ws.busy = true;
        ws.packageIndex = idx;
        ws.subStep = hasQml ? 0 : 1;
        ws.progressKiB = 0;
        ws.targetKiB = hasQml ? 4 : pkg.objectCount;
        ws.elapsedSlotMs = 0;
        ws.stageLabel = hasQml ? QStringLiteral("QMLGEN") : QStringLiteral("CXX");
        ws.stageColor = hasQml ? QStringLiteral("#fbbf24") : QStringLiteral("#60a5fa");
        ws.packageLabel = pkg.name + QStringLiteral(" v") + pkg.version;
        ws.stepDetail = hasQml ? QStringLiteral("qmlcachegen aot")
                               : (QStringLiteral("0/") + QString::number(pkg.objectCount)
                                  + QStringLiteral(" obj"));
        ws.progress = 0.05;
        m_workerModel->setSlot(slot, ws);

        if (hasQml) {
            pushLiveLog(QStringLiteral("[qmlcachegen] --only-bytecode=false ") + pkg.name
                        + QStringLiteral("/Main.qml -> .rcc/qmlcache.cpp"));
        } else {
            pushLiveLog(QStringLiteral("[clang++] -O3 -std=c++20 -flto -c ") + pkg.name
                        + QStringLiteral("/src/module.cpp"));
        }
        return;
    }

    WorkerSlotState idle;
    idle.busy = false;
    idle.stageLabel = QStringLiteral("IDLE");
    idle.stageColor = QStringLiteral("#64748b");
    idle.packageLabel = QStringLiteral("worker queue complete");
    idle.stepDetail = QStringLiteral("done");
    idle.progress = 1.0;
    m_workerModel->setSlot(slot, idle);
}

void PackageEngine::advanceBuildSlot(int slot, int deltaMs) {
    WorkerSlotState ws = m_workerModel->slot(slot);
    if (!ws.busy || ws.packageIndex < 0)
        return;

    const auto &pkgs = m_planModel->packages();
    const PackageSpec &pkg = pkgs[static_cast<size_t>(ws.packageIndex)];
    ws.elapsedSlotMs += deltaMs;

    if (ws.subStep == 0) {
        // QMLGEN stage (4 ticks)
        ws.progressKiB += 1;
        ws.progress = 0.22 * (static_cast<double>(ws.progressKiB) / std::max(1, ws.targetKiB));
        ws.stepDetail = QStringLiteral("aot ") + QString::number(ws.progressKiB * 3)
                        + QStringLiteral(" qml files");
        if (ws.progressKiB >= ws.targetKiB) {
            ws.subStep = 1;
            ws.progressKiB = 0;
            ws.targetKiB = pkg.objectCount;
            ws.stageLabel = QStringLiteral("CXX");
            ws.stageColor = QStringLiteral("#60a5fa");
            ws.stepDetail = QStringLiteral("0/") + QString::number(pkg.objectCount)
                            + QStringLiteral(" obj");
            pushLiveLog(QStringLiteral("[clang++] -O3 -std=c++20 -flto -c ") + pkg.name
                        + QStringLiteral("/ItemPaintTraversal.cpp"));
        }
        m_workerModel->setSlot(slot, ws);
        return;
    }

    if (ws.subStep == 1) {
        // CXX compile stage (~14-18 ticks per package)
        const int stepObjs = ((m_tickCounter + slot) % 2 == 0) ? 1 : 1;
        ws.progressKiB = std::min(ws.targetKiB, ws.progressKiB + stepObjs);
        ws.progress = 0.22 + 0.63 * (static_cast<double>(ws.progressKiB) / std::max(1, ws.targetKiB));
        ws.stepDetail = QString::number(ws.progressKiB) + QStringLiteral("/")
                        + QString::number(ws.targetKiB) + QStringLiteral(" obj");
        if (ws.progressKiB == ws.targetKiB / 2) {
            pushLiveLog(QStringLiteral("[clang++] -O3 -std=c++20 -DNDEBUG ") + pkg.name
                        + QStringLiteral("/unit_") + QString::number(ws.progressKiB)
                        + QStringLiteral(".cpp.o"));
        }
        if (ws.progressKiB >= ws.targetKiB) {
            ws.subStep = 2;
            ws.progressKiB = 0;
            ws.targetKiB = 3;
            ws.stageLabel = QStringLiteral("LINK");
            ws.stageColor = QStringLiteral("#34d399");
            ws.stepDetail = QStringLiteral("lld --lto-O3");
            ws.progress = 0.90;
            pushLiveLog(QStringLiteral("[ld.lld] --icf=all --lto-O3 lib")
                        + QString(pkg.name).replace(u'-', u'_') + QStringLiteral(".so"));
        }
        m_workerModel->setSlot(slot, ws);
        return;
    }

    // LINK stage (3 ticks)
    ws.progressKiB += 1;
    ws.progress = std::min(1.0, 0.88 + 0.04 * ws.progressKiB);
    if (ws.progressKiB >= ws.targetKiB) {
        ++m_completedInPhase;
        const QString secs =
            QString::number(static_cast<double>(ws.elapsedSlotMs) / 1000.0, 'f', 1)
            + QStringLiteral("s");
        appendStep(QStringLiteral("Compiled"), QStringLiteral("#34d399"),
                   pkg.name + QStringLiteral(" v") + pkg.version,
                   QStringLiteral("[") + QString::number(pkg.objectCount)
                       + QStringLiteral(" obj · ") + secs + QStringLiteral("]"));

        if (!m_diagnosticEmitted && pkg.name == QLatin1String("qmltui-controls")) {
            m_diagnosticEmitted = true;
            ScrollbackEntry diag;
            diag.entryKind = ScrollbackFeedModel::DiagnosticEntry;
            diag.title = QStringLiteral("qmlcachegen · AOT Binding Report");
            diag.subtitle = QStringLiteral("qmltui-controls v0.6.0");
            diag.line1 = QStringLiteral(
                "Compiled 42 QML property bindings & 11 required-property delegates to C++17");
            diag.line2 = QStringLiteral(
                "pragma ComponentBehavior: Bound verified across all modules (0 dynamic lookups)");
            diag.accentColor = QStringLiteral("#fbbf24");
            m_feedModel->appendEntry(diag);
        }

        assignNextBuildJob(slot);
        return;
    }
    m_workerModel->setSlot(slot, ws);
}

void PackageEngine::finishInstallation() {
    appendStep(QStringLiteral("Linked"), QStringLiteral("#34d399"),
               m_targetName + QStringLiteral(" v") + m_targetVersion
                   + QStringLiteral(" -> ~/.qpm/bin/") + m_targetName,
               QStringLiteral("[release + lto]"));

    const QString fetchSec =
        QString::number(static_cast<double>(m_fetchElapsedMs) / 1000.0, 'f', 1)
        + QStringLiteral("s");
    const QString buildSec =
        QString::number(static_cast<double>(m_buildElapsedMs) / 1000.0, 'f', 1)
        + QStringLiteral("s");

    ScrollbackEntry receipt;
    receipt.entryKind = ScrollbackFeedModel::ReceiptEntry;
    receipt.title = QStringLiteral("Installed ") + m_targetName + QStringLiteral(" v")
                    + m_targetVersion;
    receipt.subtitle = QStringLiteral("Ed25519 ") + QString::number(totalPackages())
                       + QStringLiteral("/") + QString::number(totalPackages())
                       + QStringLiteral(" ✓");
    receipt.line1 = QStringLiteral("Binary     ~/.qpm/bin/") + m_targetName
                    + QStringLiteral("  ·  Profile: release [optimized + lto + qmlcachegen]");
    receipt.line2 = QStringLiteral("Features   ") + activeFeaturesText() + QStringLiteral("  ·  ")
                    + QString::number(totalPackages()) + QStringLiteral(" packages (")
                    + QString::number(cachedPackages()) + QStringLiteral(" cached)");
    receipt.line3 = QStringLiteral("Timing     Fetch ") + fetchSec + QStringLiteral("  ·  Build ")
                    + buildSec + QStringLiteral("  ·  Total ") + elapsedTotalText()
                    + QStringLiteral("  ·  Disk +") + totalInstallText();
    receipt.accentColor = QStringLiteral("#34d399");
    m_feedModel->appendEntry(receipt);

    m_phase = FinishedPhase;
    m_phaseProgress = 1.0;
    m_etaText = QStringLiteral("0.0s");
    emit phaseChanged();
    emit telemetryChanged();

    if (m_autoExit)
        emit finishedAndAutoExit();
}

void PackageEngine::recomputePhaseTelemetry() {
    const auto &pkgs = m_planModel->packages();
    const int total = static_cast<int>(pkgs.size());
    if (total <= 0) {
        m_phaseProgress = 1.0;
        return;
    }

    int activeSlots = 0;
    double partialSum = 0.0;
    for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s) {
        const auto &ws = m_workerModel->slot(s);
        if (ws.busy) {
            ++activeSlots;
            partialSum += ws.progress;
        }
    }

    m_phaseProgress =
        std::clamp((static_cast<double>(m_completedInPhase) + partialSum) / total, 0.0, 1.0);

    if (m_phase == FetchingPhase) {
        const double wave = std::sin(static_cast<double>(m_tickCounter) * 0.45) * 4.2;
        const double rate = activeSlots * 13.8 + std::max(0.0, wave);
        m_throughputText = QString::number(rate, 'f', 1) + QStringLiteral(" MiB/s");
        pushThroughputSample(rate);
        const double remSecs = std::max(0.1, (1.0 - m_phaseProgress) * 2.1);
        m_etaText = QString::number(remSecs, 'f', 1) + QStringLiteral("s");
    } else if (m_phase == BuildingPhase) {
        const double wave = std::cos(static_cast<double>(m_tickCounter) * 0.35) * 2.5;
        const double objPerSec = activeSlots * 9.5 + std::max(0.0, wave);
        m_throughputText = QString::number(activeSlots * 3) + QStringLiteral(" jobs · ")
                           + QString::number(objPerSec, 'f', 0) + QStringLiteral(" tu/s");
        pushThroughputSample(objPerSec);
        const double remSecs = std::max(0.1, (1.0 - m_phaseProgress) * 2.8);
        m_etaText = QString::number(remSecs, 'f', 1) + QStringLiteral("s");
    }
}

void PackageEngine::tick(int deltaMs) {
    if (m_paused || (m_phase != FetchingPhase && m_phase != BuildingPhase))
        return;

    ++m_tickCounter;
    if (m_phase == FetchingPhase) {
        m_fetchElapsedMs += deltaMs;
        for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s)
            advanceFetchSlot(s, deltaMs);

        recomputePhaseTelemetry();
        if (m_completedInPhase >= totalPackages()) {
            startBuildingPhase();
            return;
        }
        emit telemetryChanged();
        return;
    }

    if (m_phase == BuildingPhase) {
        m_buildElapsedMs += deltaMs;
        for (int s = 0; s < WorkerSlotModel::kWorkerCount; ++s)
            advanceBuildSlot(s, deltaMs);

        recomputePhaseTelemetry();
        if (m_completedInPhase >= totalPackages()) {
            finishInstallation();
            return;
        }
        emit telemetryChanged();
    }
}
