#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>

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

class MockFileSystem {
public:
  MockFileSystem();

  void reset();

  bool contains(const QString &path) const { return m_items.contains(path); }
  const MockItem *find(const QString &path) const;

  bool createItem(const QString &dirPath, const QString &name, bool isDirectory);
  bool copyRecursive(const QString &srcPath, const QString &destPath);
  bool removeRecursive(const QString &path);
  bool renameRecursive(const QString &oldPath, const QString &newPath,
                       const QString &newName);

  static QString parentDirectoryPath(const QString &path);
  static QString joinPath(const QString &dirPath, const QString &name);

private:
  QHash<QString, MockItem> m_items;
};

} // namespace qmltui::examples
