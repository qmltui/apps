#pragma once

#include "AgentTransport.h"

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include <vector>

struct StreamMessageEntry {
    int type = 0;
    QString title;
    QString content;
    QString extra;
    bool approved = true;
    QString hunkHeader;
    QString removedLines;
    QString addedLines;
};

class StreamFeedModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum MessageType {
        BannerMessage = 0,
        UserMessage,
        ThoughtMessage,
        ToolCallMessage,
        ToolResultMessage,
        DiffMessage,
        PermissionDecisionMessage,
        AgentMessage,
        SystemMessage,
        ErrorMessage,
        SwarmTableMessage,
        RepoTreeMessage
    };
    Q_ENUM(MessageType)

    enum Roles {
        TypeRole = Qt::UserRole + 1,
        TitleRole,
        ContentRole,
        ExtraRole,
        ApprovedRole,
        HunkHeaderRole,
        RemovedLinesRole,
        AddedLinesRole
    };

    explicit StreamFeedModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void appendBanner();
    void appendUser(const QString& content);
    void appendThought(const QString& duration, const QString& content);
    void appendToolCall(const QString& toolName, const QString& args);
    void appendToolResult(const QString& content);
    void appendDiff(const PatchPayload& patch);
    void appendPermission(const QString& content, bool approved);
    void appendAgent(const QString& title, const QString& content, const QString& extra);
    void appendSystem(const QString& title, const QString& content);
    void appendError(const QString& content);
    void appendSwarmTable(const QString& title);
    void appendRepoTree(const QString& title);
    void clear();

private:
    void appendEntry(StreamMessageEntry entry);

    std::vector<StreamMessageEntry> m_entries;
};

struct SlashCommandSpec {
    QString cmd;
    QString desc;
};

class CommandCatalogModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)
    Q_PROPERTY(int windowStart READ windowStart WRITE setWindowStart NOTIFY windowStartChanged)
    Q_PROPERTY(
        int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int matchCount READ matchCount NOTIFY filterChanged)
    Q_PROPERTY(QString currentCommand READ currentCommand NOTIFY currentCommandChanged)

public:
    enum Roles { ActualIndexRole = Qt::UserRole + 1, CmdRole, DescRole };

    static constexpr int kMaxWindowRows = 5;

    explicit CommandCatalogModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& text);

    int windowStart() const { return m_windowStart; }
    void setWindowStart(int start);

    int selectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);

    int matchCount() const { return static_cast<int>(m_matchedIndices.size()); }
    QString currentCommand() const { return commandAt(m_selectedIndex); }

    Q_INVOKABLE QString commandAt(int matchIndex) const;

signals:
    void filterChanged();
    void windowStartChanged();
    void selectedIndexChanged();
    void currentCommandChanged();

private:
    int visibleWindowCount(int totalMatches, int start) const;

    std::vector<SlashCommandSpec> m_allCommands;
    std::vector<int> m_matchedIndices;
    QString m_filterText;
    int m_windowStart = 0;
    int m_selectedIndex = 0;
};

struct NeuralModelSpec {
    QString modelId;
    QString name;
    QString provider;
    QString context;
    QString rate;
    QString badge;
    QString badgeStatus;
    QString desc;
};

class NeuralModelCatalog : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ rowCount CONSTANT)

public:
    enum Roles {
        ModelIdRole = Qt::UserRole + 1,
        NameRole,
        ProviderRole,
        ContextRole,
        RateRole,
        BadgeRole,
        BadgeStatusRole,
        DescRole
    };

    explicit NeuralModelCatalog(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int indexOfModel(const QString& modelId) const;
    Q_INVOKABLE QString modelIdAt(int row) const;

private:
    std::vector<NeuralModelSpec> m_models;
};

class DiffPreviewModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString targetFile READ targetFile NOTIFY patchChanged)
    Q_PROPERTY(QString hunkHeader READ hunkHeader NOTIFY patchChanged)
    Q_PROPERTY(QString additionsLabel READ additionsLabel NOTIFY patchChanged)
    Q_PROPERTY(QString deletionsLabel READ deletionsLabel NOTIFY patchChanged)

public:
    enum Roles {
        LeftTextRole = Qt::UserRole + 1,
        LeftColorRole,
        RightTextRole,
        RightColorRole,
        RowBgRole
    };

    explicit DiffPreviewModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString targetFile() const { return m_targetFile; }
    QString hunkHeader() const { return m_hunkHeader; }
    QString additionsLabel() const { return m_additionsLabel; }
    QString deletionsLabel() const { return m_deletionsLabel; }

    void setPatch(const PatchPayload& patch);

signals:
    void patchChanged();

private:
    QString m_targetFile;
    QString m_hunkHeader;
    QString m_additionsLabel;
    QString m_deletionsLabel;
    std::vector<PatchSideBySideLine> m_rows;
};

class SwarmAuditModel : public QAbstractTableModel {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SwarmAuditModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    struct AuditRow {
        QString subagent;
        QString scope;
        QString status;
        QString findings;
    };
    std::vector<AuditRow> m_rows;
};
