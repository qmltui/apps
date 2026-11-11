#pragma once

#include <QDateTime>
#include <QFileDevice>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QString>
#include <QVariantMap>

namespace qmltui::examples {

class FileEntryFormatter {
public:
  static QVariantMap formatFileEntry(const QString &name,
                                     const QString &fullPath, bool isDir,
                                     qint64 size, const QDateTime &modified,
                                     const QString &permissions,
                                     const QString &owner,
                                     const QString &mimeType, bool isMarked);

  static QString formatSize(qint64 bytes);
  static QString formatPermissions(QFileDevice::Permissions perms, bool isDir,
                                   bool isSymLink = false);
  static QString formatOwner(const QFileInfo &info);
  static QString fileIconFor(const QString &name, bool isDir);
  static QString iconColorFor(const QString &name, bool isDir);

  static bool isVisualPreviewFile(const QString &fileName,
                                  const QString &mimeName);
  static bool isTextFile(const QMimeDatabase &mimeDb, bool useMock,
                         const QString &filePath, const QString &fileName,
                         const QString &mimeName);

  static bool copyPathRecursively(const QString &srcPath,
                                  const QString &destPath);
};

} // namespace qmltui::examples
