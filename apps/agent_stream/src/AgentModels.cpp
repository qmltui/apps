#include "AgentModels.h"

#include <algorithm>

StreamFeedModel::StreamFeedModel(QObject* parent) : QAbstractListModel(parent) {}

int StreamFeedModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_entries.size());
}

QVariant StreamFeedModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_entries.size())) {
        return {};
    }

    const StreamMessageEntry& entry = m_entries[static_cast<size_t>(index.row())];
    switch (role) {
    case TypeRole:
        return entry.type;
    case TitleRole:
        return entry.title;
    case ContentRole:
        return entry.content;
    case ExtraRole:
        return entry.extra;
    case ApprovedRole:
        return entry.approved;
    case HunkHeaderRole:
        return entry.hunkHeader;
    case RemovedLinesRole:
        return entry.removedLines;
    case AddedLinesRole:
        return entry.addedLines;
    default:
        return {};
    }
}

QHash<int, QByteArray> StreamFeedModel::roleNames() const {
    return {
        {TypeRole, QByteArrayLiteral("type")},
        {TitleRole, QByteArrayLiteral("title")},
        {ContentRole, QByteArrayLiteral("content")},
        {ExtraRole, QByteArrayLiteral("extra")},
        {ApprovedRole, QByteArrayLiteral("approved")},
        {HunkHeaderRole, QByteArrayLiteral("hunkHeader")},
        {RemovedLinesRole, QByteArrayLiteral("removedLines")},
        {AddedLinesRole, QByteArrayLiteral("addedLines")},
    };
}

void StreamFeedModel::appendEntry(StreamMessageEntry entry) {
    const int row = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), row, row);
    m_entries.push_back(std::move(entry));
    endInsertRows();
}

void StreamFeedModel::appendBanner() {
    StreamMessageEntry entry;
    entry.type = BannerMessage;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendUser(const QString& content) {
    StreamMessageEntry entry;
    entry.type = UserMessage;
    entry.content = content;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendThought(const QString& duration, const QString& content) {
    StreamMessageEntry entry;
    entry.type = ThoughtMessage;
    entry.title = duration;
    entry.content = content;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendToolCall(const QString& toolName, const QString& args) {
    StreamMessageEntry entry;
    entry.type = ToolCallMessage;
    entry.title = toolName;
    entry.content = args;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendToolResult(const QString& content) {
    StreamMessageEntry entry;
    entry.type = ToolResultMessage;
    entry.content = content;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendDiff(const PatchPayload& patch) {
    StreamMessageEntry entry;
    entry.type = DiffMessage;
    entry.title = patch.filePath;
    entry.extra = patch.summaryLabel;
    entry.hunkHeader = patch.unifiedHunkHeader;
    entry.removedLines = patch.removedLines;
    entry.addedLines = patch.addedLines;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendPermission(const QString& content, bool approved) {
    StreamMessageEntry entry;
    entry.type = PermissionDecisionMessage;
    entry.content = content;
    entry.approved = approved;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendAgent(const QString& title, const QString& content,
                                  const QString& extra) {
    StreamMessageEntry entry;
    entry.type = AgentMessage;
    entry.title = title;
    entry.content = content;
    entry.extra = extra;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendSystem(const QString& title, const QString& content) {
    StreamMessageEntry entry;
    entry.type = SystemMessage;
    entry.title = title;
    entry.content = content;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendError(const QString& content) {
    StreamMessageEntry entry;
    entry.type = ErrorMessage;
    entry.content = content;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendSwarmTable(const QString& title) {
    StreamMessageEntry entry;
    entry.type = SwarmTableMessage;
    entry.title = title;
    appendEntry(std::move(entry));
}

void StreamFeedModel::appendRepoTree(const QString& title) {
    StreamMessageEntry entry;
    entry.type = RepoTreeMessage;
    entry.title = title;
    appendEntry(std::move(entry));
}

void StreamFeedModel::clear() {
    if (m_entries.empty()) return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

CommandCatalogModel::CommandCatalogModel(QObject* parent)
    : QAbstractListModel(parent),
      m_allCommands({
          {QStringLiteral("/help"), QStringLiteral("Show all workflows & shortcuts")},
          {QStringLiteral("/demo 1"), QStringLiteral("Refactor InlineRenderer & clamp bounds")},
          {QStringLiteral("/demo 2"),
           QStringLiteral("Dispatch parallel subagent swarm (TableView)")},
          {QStringLiteral("/demo 3"), QStringLiteral("Security sandbox exploit audit")},
          {QStringLiteral("/demo 4"), QStringLiteral("Explore workspace hierarchy (TreeView)")},
          {QStringLiteral("/model"), QStringLiteral("Switch active neural model (F2)")},
          {QStringLiteral("/swarm"), QStringLiteral("Show subagent swarm status (TableView)")},
          {QStringLiteral("/tree"), QStringLiteral("Inspect workspace hierarchy (TreeView)")},
          {QStringLiteral("/diff"), QStringLiteral("Inspect side-by-side patch diff (F3)")},
          {QStringLiteral("/cost"), QStringLiteral("Session token metrics & capacity")},
          {QStringLiteral("/doctor"), QStringLiteral("Run system environment diagnostics")},
          {QStringLiteral("/clear"), QStringLiteral("Clear terminal history")},
          {QStringLiteral("/quit"), QStringLiteral("Exit studio session")},
      }) {
    m_matchedIndices.resize(m_allCommands.size());
    for (size_t i = 0; i < m_allCommands.size(); ++i)
        m_matchedIndices[i] = static_cast<int>(i);
}

int CommandCatalogModel::visibleWindowCount(int totalMatches, int start) const {
    return std::clamp(totalMatches - start, 0, kMaxWindowRows);
}

int CommandCatalogModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return visibleWindowCount(matchCount(), m_windowStart);
}

QVariant CommandCatalogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};

    const int actualIdx = m_windowStart + index.row();
    if (actualIdx < 0 || actualIdx >= matchCount()) return {};

    const SlashCommandSpec& spec =
        m_allCommands[static_cast<size_t>(m_matchedIndices[static_cast<size_t>(actualIdx)])];
    switch (role) {
    case ActualIndexRole:
        return actualIdx;
    case CmdRole:
        return spec.cmd;
    case DescRole:
        return spec.desc;
    default:
        return {};
    }
}

QHash<int, QByteArray> CommandCatalogModel::roleNames() const {
    return {
        {ActualIndexRole, QByteArrayLiteral("actualIndex")},
        {CmdRole, QByteArrayLiteral("cmd")},
        {DescRole, QByteArrayLiteral("desc")},
    };
}

void CommandCatalogModel::setFilterText(const QString& text) {
    if (m_filterText == text) return;

    const int oldRows = rowCount();
    const bool windowWasNonZero = m_windowStart != 0;
    const bool selectionWasNonZero = m_selectedIndex != 0;
    const QString previousCommand = currentCommand();
    m_filterText = text;

    const QString query = text.trimmed().toLower();
    std::vector<int> nextMatches;
    nextMatches.reserve(m_allCommands.size());
    if (query.isEmpty() || query == QLatin1String("/")) {
        for (size_t i = 0; i < m_allCommands.size(); ++i)
            nextMatches.push_back(static_cast<int>(i));
    } else {
        for (size_t i = 0; i < m_allCommands.size(); ++i) {
            const SlashCommandSpec& item = m_allCommands[i];
            if (item.cmd.toLower().contains(query) || item.desc.toLower().contains(query))
                nextMatches.push_back(static_cast<int>(i));
        }
    }

    const int newRows = visibleWindowCount(static_cast<int>(nextMatches.size()), 0);
    if (newRows < oldRows) {
        beginRemoveRows(QModelIndex(), newRows, oldRows - 1);
        m_matchedIndices = std::move(nextMatches);
        m_windowStart = 0;
        m_selectedIndex = 0;
        endRemoveRows();
    } else if (newRows > oldRows) {
        beginInsertRows(QModelIndex(), oldRows, newRows - 1);
        m_matchedIndices = std::move(nextMatches);
        m_windowStart = 0;
        m_selectedIndex = 0;
        endInsertRows();
    } else {
        m_matchedIndices = std::move(nextMatches);
        m_windowStart = 0;
        m_selectedIndex = 0;
    }

    const int sharedRows = std::min(oldRows, newRows);
    if (sharedRows > 0) emit dataChanged(index(0), index(sharedRows - 1));

    if (windowWasNonZero) emit windowStartChanged();
    if (selectionWasNonZero) emit selectedIndexChanged();
    emit filterChanged();
    if (currentCommand() != previousCommand) emit currentCommandChanged();
}

void CommandCatalogModel::setSelectedIndex(int idx) {
    const int clamped = matchCount() > 0 ? std::clamp(idx, 0, matchCount() - 1) : 0;
    if (m_selectedIndex == clamped) return;
    const QString previousCommand = currentCommand();
    m_selectedIndex = clamped;
    emit selectedIndexChanged();
    if (currentCommand() != previousCommand) emit currentCommandChanged();
}

void CommandCatalogModel::setWindowStart(int start) {
    const int clamped = std::max(0, std::min(start, matchCount() - kMaxWindowRows));
    if (m_windowStart == clamped) return;

    const int oldRows = rowCount();
    const int newRows = visibleWindowCount(matchCount(), clamped);
    if (newRows < oldRows) {
        beginRemoveRows(QModelIndex(), newRows, oldRows - 1);
        m_windowStart = clamped;
        endRemoveRows();
    } else if (newRows > oldRows) {
        beginInsertRows(QModelIndex(), oldRows, newRows - 1);
        m_windowStart = clamped;
        endInsertRows();
    } else {
        m_windowStart = clamped;
    }

    const int sharedRows = std::min(oldRows, newRows);
    if (sharedRows > 0) emit dataChanged(index(0), index(sharedRows - 1));
    emit windowStartChanged();
}

QString CommandCatalogModel::commandAt(int matchIndex) const {
    if (matchIndex < 0 || matchIndex >= matchCount()) return {};
    return m_allCommands[static_cast<size_t>(m_matchedIndices[static_cast<size_t>(matchIndex)])]
        .cmd;
}

NeuralModelCatalog::NeuralModelCatalog(QObject* parent)
    : QAbstractListModel(parent),
      m_models({
          {
              QStringLiteral("claude-3-7-sonnet"),
              QStringLiteral("Claude 3.7 Sonnet"),
              QStringLiteral("Anthropic"),
              QStringLiteral("128k / 64k thinking"),
              QStringLiteral("$3.00/M in · $15.00/M out"),
              QStringLiteral("default"),
              QStringLiteral("success"),
              QStringLiteral("Hybrid reasoning with adaptive token budget & code synthesis"),
          },
          {
              QStringLiteral("gpt-4.5-preview"),
              QStringLiteral("GPT-4.5 Preview"),
              QStringLiteral("OpenAI"),
              QStringLiteral("128k / 32k thinking"),
              QStringLiteral("$7.50/M in · $37.50/M out"),
              QStringLiteral("frontier"),
              QStringLiteral("info"),
              QStringLiteral("Deep reasoning & architecture analysis with high fidelity"),
          },
          {
              QStringLiteral("gemini-2.0-flash"),
              QStringLiteral("Gemini 2.0 Flash"),
              QStringLiteral("Google"),
              QStringLiteral("1.0M context window"),
              QStringLiteral("$0.10/M in · $0.40/M out"),
              QStringLiteral("fastest"),
              QStringLiteral("warning"),
              QStringLiteral("Ultra-low latency streaming with 1M token context window"),
          },
          {
              QStringLiteral("deepseek-r1"),
              QStringLiteral("DeepSeek R1"),
              QStringLiteral("DeepSeek"),
              QStringLiteral("64k context window"),
              QStringLiteral("$0.55/M in · $2.19/M out"),
              QStringLiteral("reasoning"),
              QStringLiteral("subtle"),
              QStringLiteral("Open-weights reinforced reasoning for math & logic proofs"),
          },
      }) {}

int NeuralModelCatalog::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_models.size());
}

QVariant NeuralModelCatalog::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};

    const NeuralModelSpec& spec = m_models[static_cast<size_t>(index.row())];
    switch (role) {
    case ModelIdRole:
        return spec.modelId;
    case NameRole:
        return spec.name;
    case ProviderRole:
        return spec.provider;
    case ContextRole:
        return spec.context;
    case RateRole:
        return spec.rate;
    case BadgeRole:
        return spec.badge;
    case BadgeStatusRole:
        return spec.badgeStatus;
    case DescRole:
        return spec.desc;
    default:
        return {};
    }
}

QHash<int, QByteArray> NeuralModelCatalog::roleNames() const {
    return {
        {ModelIdRole, QByteArrayLiteral("modelId")},
        {NameRole, QByteArrayLiteral("name")},
        {ProviderRole, QByteArrayLiteral("provider")},
        {ContextRole, QByteArrayLiteral("context")},
        {RateRole, QByteArrayLiteral("rate")},
        {BadgeRole, QByteArrayLiteral("badge")},
        {BadgeStatusRole, QByteArrayLiteral("badgeStatus")},
        {DescRole, QByteArrayLiteral("desc")},
    };
}

int NeuralModelCatalog::indexOfModel(const QString& modelId) const {
    for (size_t i = 0; i < m_models.size(); ++i) {
        if (m_models[i].modelId == modelId) return static_cast<int>(i);
    }
    return -1;
}

QString NeuralModelCatalog::modelIdAt(int row) const {
    if (row < 0 || row >= rowCount()) return {};
    return m_models[static_cast<size_t>(row)].modelId;
}

DiffPreviewModel::DiffPreviewModel(QObject* parent) : QAbstractListModel(parent) {}

int DiffPreviewModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

QVariant DiffPreviewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};

    const PatchSideBySideLine& row = m_rows[static_cast<size_t>(index.row())];
    switch (role) {
    case LeftTextRole:
        return row.leftText;
    case LeftColorRole:
        return QVariant::fromValue(row.leftColor);
    case RightTextRole:
        return row.rightText;
    case RightColorRole:
        return QVariant::fromValue(row.rightColor);
    case RowBgRole:
        return QVariant::fromValue(row.rowBg);
    default:
        return {};
    }
}

QHash<int, QByteArray> DiffPreviewModel::roleNames() const {
    return {
        {LeftTextRole, QByteArrayLiteral("leftText")},
        {LeftColorRole, QByteArrayLiteral("leftColor")},
        {RightTextRole, QByteArrayLiteral("rightText")},
        {RightColorRole, QByteArrayLiteral("rightColor")},
        {RowBgRole, QByteArrayLiteral("rowBg")},
    };
}

void DiffPreviewModel::setPatch(const PatchPayload& patch) {
    const int oldRows = rowCount();
    const int newRows = static_cast<int>(patch.sideBySideRows.size());

    m_targetFile = patch.filePath;
    m_hunkHeader = patch.sideBySideHeader;
    m_additionsLabel = patch.additionsLabel;
    m_deletionsLabel = patch.deletionsLabel;

    if (newRows < oldRows) {
        beginRemoveRows(QModelIndex(), newRows, oldRows - 1);
        m_rows = patch.sideBySideRows;
        endRemoveRows();
    } else if (newRows > oldRows) {
        beginInsertRows(QModelIndex(), oldRows, newRows - 1);
        m_rows = patch.sideBySideRows;
        endInsertRows();
    } else {
        m_rows = patch.sideBySideRows;
    }

    const int sharedRows = std::min(oldRows, newRows);
    if (sharedRows > 0) emit dataChanged(index(0), index(sharedRows - 1));
    emit patchChanged();
}

SwarmAuditModel::SwarmAuditModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_rows({
          {QStringLiteral("MemoryAuditor"), QStringLiteral("src/QmlTui/Engine/"),
           QStringLiteral("Pass"), QStringLiteral("0 leaks (checked 18)")},
          {QStringLiteral("QmlProfiler"), QStringLiteral("apps/agent_stream/"),
           QStringLiteral("Pass"), QStringLiteral("0 binding loops")},
          {QStringLiteral("RaceVerifier"), QStringLiteral("Terminal.cpp"), QStringLiteral("Pass"),
           QStringLiteral("Thread-affinity safe")},
      }) {}

int SwarmAuditModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

int SwarmAuditModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 4;
}

QVariant SwarmAuditModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole || index.row() < 0 ||
        index.row() >= rowCount()) {
        return {};
    }

    const AuditRow& row = m_rows[static_cast<size_t>(index.row())];
    switch (index.column()) {
    case 0:
        return row.subagent;
    case 1:
        return row.scope;
    case 2:
        return row.status;
    case 3:
        return row.findings;
    default:
        return {};
    }
}
