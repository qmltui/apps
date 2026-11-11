#include "FileEntryFormatter.h"

#include <QDir>
#include <QDirListing>
#include <QFile>
#include <QLocale>
#include <QMimeType>
#include <QStringList>
#include <algorithm>

namespace qmltui::examples {

QVariantMap FileEntryFormatter::formatFileEntry(
    const QString &name, const QString &fullPath, bool isDir, qint64 size,
    const QDateTime &modified, const QString &permissions, const QString &owner,
    const QString &mimeType, bool isMarked) {
  QVariantMap map;
  map[QStringLiteral("name")] = name;
  map[QStringLiteral("path")] = fullPath;
  map[QStringLiteral("isDir")] = isDir;
  map[QStringLiteral("size")] = isDir ? (std::max)(qint64(0), size) : size;
  if (isDir) {
    map[QStringLiteral("sizeFormatted")] =
        (size >= 0) ? QStringLiteral("%1 items").arg(size)
                    : QStringLiteral("DIR");
  } else {
    map[QStringLiteral("sizeFormatted")] = formatSize(size);
  }
  map[QStringLiteral("modified")] =
      modified.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"));
  map[QStringLiteral("modifiedRaw")] = modified.toUTC().toString(Qt::ISODate);
  map[QStringLiteral("permissions")] = permissions;
  map[QStringLiteral("owner")] = owner;
  map[QStringLiteral("mimeType")] = mimeType;
  map[QStringLiteral("icon")] = fileIconFor(name, isDir);
  map[QStringLiteral("iconColor")] = iconColorFor(name, isDir);

  const qsizetype dotIndex = name.lastIndexOf(QLatin1Char('.'));
  if (dotIndex > 0 && !isDir) {
    map[QStringLiteral("extension")] = name.mid(dotIndex + 1).toLower();
  } else {
    map[QStringLiteral("extension")] = QString();
  }

  map[QStringLiteral("isMarked")] = isMarked;
  return map;
}

QString FileEntryFormatter::formatSize(qint64 bytes) {
  return QLocale::c().formattedDataSize(bytes, 1,
                                        QLocale::DataSizeTraditionalFormat);
}

QString FileEntryFormatter::formatPermissions(QFileDevice::Permissions perms,
                                              bool isDir, bool isSymLink) {
  QString res(10, QLatin1Char('-'));
  if (isSymLink)
    res[0] = QLatin1Char('l');
  else if (isDir)
    res[0] = QLatin1Char('d');

  if (perms.testFlag(QFileDevice::ReadOwner) ||
      perms.testFlag(QFileDevice::ReadUser))
    res[1] = QLatin1Char('r');
  if (perms.testFlag(QFileDevice::WriteOwner) ||
      perms.testFlag(QFileDevice::WriteUser))
    res[2] = QLatin1Char('w');
  if (perms.testFlag(QFileDevice::ExeOwner) ||
      perms.testFlag(QFileDevice::ExeUser))
    res[3] = QLatin1Char('x');

  if (perms.testFlag(QFileDevice::ReadGroup))
    res[4] = QLatin1Char('r');
  if (perms.testFlag(QFileDevice::WriteGroup))
    res[5] = QLatin1Char('w');
  if (perms.testFlag(QFileDevice::ExeGroup))
    res[6] = QLatin1Char('x');

  if (perms.testFlag(QFileDevice::ReadOther))
    res[7] = QLatin1Char('r');
  if (perms.testFlag(QFileDevice::WriteOther))
    res[8] = QLatin1Char('w');
  if (perms.testFlag(QFileDevice::ExeOther))
    res[9] = QLatin1Char('x');

  return res;
}

QString FileEntryFormatter::formatOwner(const QFileInfo &info) {
  const QString owner = info.owner();
  const QString group = info.group();
  if (!owner.isEmpty() && !group.isEmpty())
    return owner + QLatin1Char(':') + group;
  if (!owner.isEmpty())
    return owner;
  return qEnvironmentVariable(
      "USER", qEnvironmentVariable("USERNAME", QStringLiteral("user")));
}

QString FileEntryFormatter::fileIconFor(const QString &name, bool isDir) {
  if (isDir)
    return QStringLiteral("📁");
  const QString lower = name.toLower();
  if (lower.endsWith(QStringLiteral(".py")))
    return QStringLiteral("🐍");
  if (lower.endsWith(QStringLiteral(".rs")))
    return QStringLiteral("🦀");
  if (lower.endsWith(QStringLiteral(".cpp")) ||
      lower.endsWith(QStringLiteral(".c")) ||
      lower.endsWith(QStringLiteral(".h")) ||
      lower.endsWith(QStringLiteral(".hpp")))
    return QStringLiteral("⚡");
  if (lower.endsWith(QStringLiteral(".md")) ||
      lower.endsWith(QStringLiteral(".txt")))
    return QStringLiteral("📄");
  if (lower.endsWith(QStringLiteral(".json")) ||
      lower.endsWith(QStringLiteral(".yaml")) ||
      lower.endsWith(QStringLiteral(".yml")) ||
      lower.endsWith(QStringLiteral(".toml")))
    return QStringLiteral("⚙️");
  if (lower.endsWith(QStringLiteral(".tar.gz")) ||
      lower.endsWith(QStringLiteral(".zip")) ||
      lower.endsWith(QStringLiteral(".tar")) ||
      lower.endsWith(QStringLiteral(".gz")))
    return QStringLiteral("📦");
  if (lower.endsWith(QStringLiteral(".pdf")))
    return QStringLiteral("📕");
  if (lower.endsWith(QStringLiteral(".svg")) ||
      lower.endsWith(QStringLiteral(".svgz")))
    return QStringLiteral("📐");
  if (lower.endsWith(QStringLiteral(".png")) ||
      lower.endsWith(QStringLiteral(".jpg")) ||
      lower.endsWith(QStringLiteral(".jpeg")) ||
      lower.endsWith(QStringLiteral(".webp")) ||
      lower.endsWith(QStringLiteral(".gif")) ||
      lower.endsWith(QStringLiteral(".bmp")))
    return QStringLiteral("🖼️");
  if (lower.endsWith(QStringLiteral(".sh")) ||
      lower.endsWith(QStringLiteral(".bash")))
    return QStringLiteral("📜");
  if (lower.endsWith(QStringLiteral(".qml")))
    return QStringLiteral("✨");
  if (lower == QStringLiteral("cmakelists.txt"))
    return QStringLiteral("🛠️");
  if (lower == QStringLiteral("license"))
    return QStringLiteral("⚖️");
  return QStringLiteral("📄");
}

QString FileEntryFormatter::iconColorFor(const QString &name, bool isDir) {
  if (isDir)
    return QStringLiteral("ansi:brightblue");
  const QString lower = name.toLower();
  if (lower.endsWith(QStringLiteral(".py")))
    return QStringLiteral("ansi:brightyellow");
  if (lower.endsWith(QStringLiteral(".rs")))
    return QStringLiteral("ansi:yellow");
  if (lower.endsWith(QStringLiteral(".cpp")) ||
      lower.endsWith(QStringLiteral(".h")))
    return QStringLiteral("ansi:brightcyan");
  if (lower.endsWith(QStringLiteral(".md")) ||
      lower.endsWith(QStringLiteral(".txt")))
    return QStringLiteral("ansi:brightgreen");
  if (lower.endsWith(QStringLiteral(".json")) ||
      lower.endsWith(QStringLiteral(".yaml")))
    return QStringLiteral("ansi:magenta");
  if (lower.endsWith(QStringLiteral(".tar.gz")) ||
      lower.endsWith(QStringLiteral(".zip")))
    return QStringLiteral("ansi:brightred");
  if (lower.endsWith(QStringLiteral(".pdf")))
    return QStringLiteral("ansi:brightred");
  if (lower.endsWith(QStringLiteral(".svg")) ||
      lower.endsWith(QStringLiteral(".svgz")))
    return QStringLiteral("ansi:brightcyan");
  if (lower.endsWith(QStringLiteral(".png")) ||
      lower.endsWith(QStringLiteral(".jpg")) ||
      lower.endsWith(QStringLiteral(".jpeg")) ||
      lower.endsWith(QStringLiteral(".webp")) ||
      lower.endsWith(QStringLiteral(".gif")) ||
      lower.endsWith(QStringLiteral(".bmp")))
    return QStringLiteral("ansi:brightmagenta");
  return QStringLiteral("default");
}

bool FileEntryFormatter::isVisualPreviewFile(const QString &fileName,
                                             const QString &mimeName) {
  const QString lowerName = fileName.toLower();
  const QString lowerMime = mimeName.toLower();
  if (lowerMime.startsWith(QStringLiteral("image/")) ||
      lowerMime == QStringLiteral("application/pdf")) {
    return true;
  }
  static const QStringList visualExtensions = {
      QStringLiteral("png"),  QStringLiteral("jpg"), QStringLiteral("jpeg"),
      QStringLiteral("webp"), QStringLiteral("bmp"), QStringLiteral("gif"),
      QStringLiteral("ico"),  QStringLiteral("svg"), QStringLiteral("svgz"),
      QStringLiteral("pdf"),
  };
  const QString suffix = QFileInfo(lowerName).suffix();
  return visualExtensions.contains(suffix);
}

bool FileEntryFormatter::isTextFile(const QMimeDatabase &mimeDb, bool useMock,
                                    const QString &filePath,
                                    const QString &fileName,
                                    const QString &mimeName) {
  if (isVisualPreviewFile(fileName, mimeName))
    return false;

  const QMimeType mt =
      useMock
          ? mimeDb.mimeTypeForName(mimeName)
          : mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchDefault);
  if (mt.isValid() && mt.inherits(QStringLiteral("text/plain")))
    return true;

  if (mimeName.startsWith(QStringLiteral("text/")) ||
      mimeName.contains(QStringLiteral("json")) ||
      mimeName.contains(QStringLiteral("xml")) ||
      mimeName.contains(QStringLiteral("yaml")) ||
      mimeName.contains(QStringLiteral("javascript")) ||
      mimeName.contains(QStringLiteral("x-c")) ||
      mimeName.contains(QStringLiteral("x-python")) ||
      mimeName.contains(QStringLiteral("x-rust")) ||
      mimeName.contains(QStringLiteral("x-cmake"))) {
    return true;
  }

  static const QStringList textExtensions = {
      QStringLiteral("md"),    QStringLiteral("txt"),  QStringLiteral("qml"),
      QStringLiteral("h"),     QStringLiteral("hpp"),  QStringLiteral("cpp"),
      QStringLiteral("c"),     QStringLiteral("rs"),   QStringLiteral("py"),
      QStringLiteral("json"),  QStringLiteral("yaml"), QStringLiteral("yml"),
      QStringLiteral("toml"),  QStringLiteral("sh"),   QStringLiteral("env"),
      QStringLiteral("cmake"), QStringLiteral("ini"),  QStringLiteral("conf"),
  };
  const QString suffix = QFileInfo(fileName).suffix().toLower();
  if (textExtensions.contains(suffix))
    return true;

  return fileName.startsWith(QLatin1Char('.')) ||
         fileName == QStringLiteral("CMakeLists.txt") ||
         fileName == QStringLiteral("LICENSE") ||
         fileName == QStringLiteral("Makefile") ||
         fileName == QStringLiteral("Dockerfile");
}

bool FileEntryFormatter::copyPathRecursively(const QString &srcPath,
                                             const QString &destPath) {
  const QFileInfo srcInfo(srcPath);
  if (!srcInfo.exists())
    return false;

  if (srcInfo.isDir()) {
    QDir destDir(destPath);
    if (!destDir.exists() && !QDir().mkpath(destPath))
      return false;

    using ItFlag = QDirListing::IteratorFlag;
    const auto flags = ItFlag::Default | ItFlag::IncludeHidden;
    for (const auto &entry : QDirListing(srcPath, flags)) {
      const QString childDest = destDir.filePath(entry.fileName());
      if (!copyPathRecursively(entry.filePath(), childDest))
        return false;
    }
    return true;
  }

  if (QFile::exists(destPath))
    return false;
  return QFile::copy(srcPath, destPath);
}

} // namespace qmltui::examples
