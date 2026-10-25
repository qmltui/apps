#pragma once

#include <QCollator>
#include <QDateTime>
#include <QFileDevice>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QMimeDatabase>
#include <QObject>
#include <QQmlEngine>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace qmltui::examples {

struct MockItem {
  QString name;
  bool isDir = false;
  qint64 size = 0;
  QDateTime modified;
  QString permissions;
  QString owner;
  QString mimeType;
  QString content;        // For text files
  QStringList childNames; // For directories
};

class FileManagerBackend : public QObject {
  Q_OBJECT
  QML_ELEMENT
  Q_DISABLE_COPY_MOVE(FileManagerBackend)

  Q_PROPERTY(
      QString currentPath READ currentPath NOTIFY currentPathChanged FINAL)
  Q_PROPERTY(QString parentPath READ parentPath NOTIFY parentPathChanged FINAL)
  Q_PROPERTY(
      QString breadcrumbs READ breadcrumbs NOTIFY currentPathChanged FINAL)
  Q_PROPERTY(QVariantList parentEntries READ parentEntries NOTIFY
                 parentEntriesChanged FINAL)
  Q_PROPERTY(QVariantList currentEntries READ currentEntries NOTIFY
                 currentEntriesChanged FINAL)
  Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY
                 currentIndexChanged FINAL)
  Q_PROPERTY(int previewWidth READ previewWidth WRITE setPreviewWidth NOTIFY
                 previewWidthChanged FINAL)
  Q_PROPERTY(
      QVariantMap previewData READ previewData NOTIFY previewDataChanged FINAL)
  Q_PROPERTY(
      QStringList markedPaths READ markedPaths NOTIFY markedPathsChanged FINAL)
  Q_PROPERTY(int markedCount READ markedCount NOTIFY markedPathsChanged FINAL)
  Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY
                 filterTextChanged FINAL)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY
                 showHiddenChanged FINAL)
  Q_PROPERTY(QString sortField READ sortField WRITE setSortField NOTIFY
                 sortFieldChanged FINAL)
  Q_PROPERTY(bool sortAscending READ sortAscending WRITE setSortAscending NOTIFY
                 sortAscendingChanged FINAL)
  Q_PROPERTY(QString diskUsage READ diskUsage NOTIFY diskUsageChanged FINAL)
  Q_PROPERTY(QString userHost READ userHost CONSTANT FINAL)
  Q_PROPERTY(
      QString clipboardOp READ clipboardOp NOTIFY clipboardChanged FINAL)
  Q_PROPERTY(int clipboardCount READ clipboardCount NOTIFY clipboardChanged FINAL)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY
                 statusMessageChanged FINAL)
  Q_PROPERTY(QString editorProgram READ editorProgram WRITE setEditorProgram
                 NOTIFY editorProgramChanged FINAL)
  Q_PROPERTY(QStringList editorArguments READ editorArguments NOTIFY
                 editorProgramChanged FINAL)
  Q_PROPERTY(QString currentItemPath READ currentItemPath NOTIFY
                 currentIndexChanged FINAL)
  Q_PROPERTY(bool isCurrentItemDir READ isCurrentItemDir NOTIFY
                 currentIndexChanged FINAL)
  Q_PROPERTY(bool isMock READ isMock CONSTANT FINAL)

public:
  enum class SortField : quint8 {
    Name,
    Size,
    ModifiedTime,
    Type,
  };

  enum class ClipboardOp : quint8 {
    None,
    Copy,
    Cut,
  };

  explicit FileManagerBackend(QObject *parent = nullptr);
  ~FileManagerBackend() override = default;

  QString currentPath() const { return m_currentPath; }
  QString parentPath() const { return m_parentPath; }
  QString breadcrumbs() const;
  QVariantList parentEntries() const { return m_parentEntries; }
  QVariantList currentEntries() const { return m_filteredEntries; }
  int currentIndex() const { return m_currentIndex; }
  void setCurrentIndex(int index);

  int previewWidth() const { return m_previewWidth; }
  void setPreviewWidth(int w);

  QVariantMap previewData() const { return m_previewData; }
  QStringList markedPaths() const { return m_markedPaths; }
  int markedCount() const { return static_cast<int>(m_markedPaths.size()); }
  QString filterText() const { return m_filterText; }
  void setFilterText(const QString &filter);

  bool showHidden() const { return m_showHidden; }
  void setShowHidden(bool show);

  QString sortField() const;
  void setSortField(const QString &field);

  bool sortAscending() const { return m_sortAscending; }
  void setSortAscending(bool ascending);

  QString diskUsage() const { return m_diskUsage; }
  QString userHost() const { return m_userHost; }
  QString clipboardOp() const;
  int clipboardCount() const {
    return static_cast<int>(m_clipboardPaths.size());
  }
  QString statusMessage() const { return m_statusMessage; }
  bool isMock() const { return m_useMock; }

  QString editorProgram() const { return m_editorProgram; }
  void setEditorProgram(const QString &prog);
  QStringList editorArguments() const { return m_editorArguments; }
  Q_INVOKABLE QStringList editorArgumentsFor(const QString &filePath) const;

  QString currentItemPath() const;
  bool isCurrentItemDir() const;

  Q_INVOKABLE QString detectEditor() const;
  Q_INVOKABLE void refreshCurrentAndPreview();
  Q_INVOKABLE void navigateTo(const QString &path);
  Q_INVOKABLE void navigateUp();
  Q_INVOKABLE void navigateIntoCurrent();
  Q_INVOKABLE void toggleMarkCurrent();
  Q_INVOKABLE void toggleMark(int index);
  Q_INVOKABLE void selectAll();
  Q_INVOKABLE void clearMarks();
  Q_INVOKABLE void copyMarked();
  Q_INVOKABLE void cutMarked();
  Q_INVOKABLE void pasteClipboard();
  Q_INVOKABLE bool createItem(const QString &name, bool isDirectory);
  Q_INVOKABLE bool deleteCurrentOrMarked();
  Q_INVOKABLE bool renameCurrent(const QString &newName);
  Q_INVOKABLE void cycleSort();
  Q_INVOKABLE void toggleShowHidden();
  Q_INVOKABLE void showToast(const QString &msg);

signals:
  void currentPathChanged();
  void parentPathChanged();
  void parentEntriesChanged();
  void currentEntriesChanged();
  void currentIndexChanged();
  void previewWidthChanged();
  void previewDataChanged();
  void markedPathsChanged();
  void filterTextChanged();
  void showHiddenChanged();
  void sortFieldChanged();
  void sortAscendingChanged();
  void diskUsageChanged();
  void clipboardChanged();
  void statusMessageChanged();
  void editorProgramChanged();

private:
  void initMockFileSystem();
  void refreshAll(const QString &selectPath = QString(),
                  bool pathChanged = true);
  void refreshParent();
  void refreshCurrent(const QString &selectPath = QString());
  void applyFilterAndSort(const QString &selectPath = QString());
  void syncMarkedStateInFilteredEntries();
  void updatePreview();
  void updateDiskUsage();
  void updateWatchedDirectory();

  QString parentDirectoryPath(const QString &path) const;
  QString joinPath(const QString &dirPath, const QString &name) const;
  bool isTextFile(const QString &filePath, const QString &fileName,
                  const QString &mimeName) const;

  bool copyMockRecursive(const QString &srcPath, const QString &destPath);
  void removeMockRecursive(const QString &path);
  void renameMockRecursive(const QString &oldPath, const QString &newPath,
                           const QString &newName);

  QVariantMap formatFileEntry(const QString &name, const QString &fullPath,
                              bool isDir, qint64 size,
                              const QDateTime &modified,
                              const QString &permissions, const QString &owner,
                              const QString &mimeType) const;

  static QString formatSize(qint64 bytes);
  static QString formatPermissions(QFileDevice::Permissions perms, bool isDir,
                                   bool isSymLink = false);
  static QString formatOwner(const QFileInfo &info);
  static QString fileIconFor(const QString &name, bool isDir);
  static QString iconColorFor(const QString &name, bool isDir);
  static QString sortFieldToString(SortField field);
  static SortField sortFieldFromString(const QString &field);
  static QString clipboardOpToString(ClipboardOp op);

  bool m_useMock = false;
  QString m_currentPath;
  QString m_parentPath;
  QVariantList m_parentEntries;
  QVariantList m_rawCurrentEntries;
  QVariantList m_filteredEntries;
  int m_currentIndex = 0;
  int m_previewWidth = 40;
  QVariantMap m_previewData;
  QStringList m_markedPaths;
  QSet<QString> m_markedSet;
  QString m_filterText;
  bool m_showHidden = false;
  SortField m_sortField = SortField::Name;
  bool m_sortAscending = true;
  QString m_diskUsage;
  QString m_userHost;
  ClipboardOp m_clipboardOp = ClipboardOp::None;
  QStringList m_clipboardPaths;
  QString m_statusMessage;
  QString m_editorProgram;
  QStringList m_editorArguments;

  QMimeDatabase m_mimeDb;
  QCollator m_collator;
  QFileSystemWatcher *m_fsWatcher = nullptr;

  // Mock storage
  QHash<QString, MockItem> m_mockFs;
};

} // namespace qmltui::examples
