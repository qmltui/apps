#include "FileManagerBackend.h"

#include "FileEntryFormatter.h"
#include "PreviewSyntaxHighlighter.h"
#include "ShellProcessHelper.h"

#include <QDir>
#include <QDirListing>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeType>
#if FILE_MANAGER_HAVE_PDF
#include <QPdfDocument>
#endif
#include <QProcess>
#include <QStorageInfo>
#include <QSysInfo>
#include <QTextStream>
#include <algorithm>

namespace qmltui::examples {

FileManagerBackend::FileManagerBackend(QObject* parent) : QObject(parent) {
    const bool scenarioRequested = qEnvironmentVariableIsSet("QMLTUI_RUNNER_SCRIPT") ||
                                   qEnvironmentVariableIsSet("QMLTUI_RUNNER_EVAL") ||
                                   qEnvironmentVariableIsSet("QMLTUI_RUNNER_CAPTURE_AFTER_MS") ||
                                   qEnvironmentVariableIsSet("QMLTUI_FM_MOCK");

    if (scenarioRequested && !qEnvironmentVariableIsSet("QMLTUI_FM_LIVE")) {
        m_useMock = true;
    }

    m_collator.setNumericMode(true);
    m_collator.setCaseSensitivity(Qt::CaseInsensitive);

    QString user = qEnvironmentVariable("USER");
    if (user.isEmpty()) user = qEnvironmentVariable("USERNAME", QStringLiteral("user"));
    QString host = QSysInfo::machineHostName();
    if (host.isEmpty()) host = QStringLiteral("terminal");
    m_userHost = user + QLatin1Char('@') + host;

    setEditorProgram(detectEditor());

    if (m_useMock) {
        m_mockFs.reset();
        m_currentPath = QStringLiteral("/workspace");
    } else {
        m_currentPath = QDir::cleanPath(QDir::currentPath());
        m_fsWatcher = new QFileSystemWatcher(this);
        connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this,
                [this](const QString& changedDir) {
                    if (QDir::cleanPath(changedDir) == m_currentPath) {
                        refreshCurrent();
                    } else if (QDir::cleanPath(changedDir) == m_parentPath) {
                        refreshParent();
                    }
                });
    }

    m_shellWorkingDirectory = m_currentPath;
    refreshAll();
}

QString FileManagerBackend::breadcrumbs() const {
    if (m_currentPath.isEmpty()) return QString();

    QString clean = QDir::fromNativeSeparators(m_currentPath);
    if (m_useMock) {
        if (clean.startsWith(QStringLiteral("/workspace"))) {
            clean = QStringLiteral("~") + clean.mid(10);
        }
    } else {
        const QString home = QDir::fromNativeSeparators(QDir::homePath());
        if (!home.isEmpty() && home != QStringLiteral("/") && clean.startsWith(home) &&
            (clean.size() == home.size() || clean.at(home.size()) == QLatin1Char('/'))) {
            clean = QStringLiteral("~") + clean.mid(home.size());
        }
    }

    const QStringList parts = clean.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return QStringLiteral(" ~");
    }

    return QStringLiteral(" ") + parts.join(QStringLiteral("/"));
}

void FileManagerBackend::setCurrentIndex(int index) {
    if (m_filteredEntries.isEmpty()) {
        m_currentIndex = -1;
    } else {
        m_currentIndex = std::clamp(index, 0, static_cast<int>(m_filteredEntries.size()) - 1);
    }
    if (m_previewPage != 0) {
        m_previewPage = 0;
        emit previewPageChanged();
    }
    emit currentIndexChanged();
    updatePreview();
}

void FileManagerBackend::setPreviewWidth(int w) {
    if (m_previewWidth == w || w <= 0) return;
    m_previewWidth = w;
    emit previewWidthChanged();
    if (m_previewData.value(QStringLiteral("type")).toString() == QStringLiteral("text")) {
        updatePreview();
    }
}

void FileManagerBackend::setPreviewPage(int page) {
    const int pageCount = std::max(1, m_previewData.value(QStringLiteral("pageCount"), 1).toInt());
    const int clamped = std::clamp(page, 0, pageCount - 1);
    if (m_previewPage == clamped) return;
    m_previewPage = clamped;
    emit previewPageChanged();
}

void FileManagerBackend::nextPreviewPage() {
    const int pageCount = std::max(1, m_previewData.value(QStringLiteral("pageCount"), 1).toInt());
    if (pageCount <= 1) return;
    setPreviewPage((m_previewPage + 1) % pageCount);
    showToast(QStringLiteral("Page %1 / %2").arg(m_previewPage + 1).arg(pageCount));
}

void FileManagerBackend::previousPreviewPage() {
    const int pageCount = std::max(1, m_previewData.value(QStringLiteral("pageCount"), 1).toInt());
    if (pageCount <= 1) return;
    setPreviewPage((m_previewPage - 1 + pageCount) % pageCount);
    showToast(QStringLiteral("Page %1 / %2").arg(m_previewPage + 1).arg(pageCount));
}

void FileManagerBackend::setFilterText(const QString& filter) {
    if (m_filterText == filter) return;
    m_filterText = filter;
    emit filterTextChanged();
    applyFilterAndSort();
}

void FileManagerBackend::setShowHidden(bool show) {
    if (m_showHidden == show) return;
    m_showHidden = show;
    emit showHiddenChanged();
    refreshParent();
    applyFilterAndSort();
}

QString FileManagerBackend::sortField() const {
    return sortFieldToString(m_sortField);
}

void FileManagerBackend::setSortField(const QString& field) {
    const SortField parsed = sortFieldFromString(field);
    if (m_sortField == parsed) return;
    m_sortField = parsed;
    emit sortFieldChanged();
    applyFilterAndSort();
}

void FileManagerBackend::setSortAscending(bool ascending) {
    if (m_sortAscending == ascending) return;
    m_sortAscending = ascending;
    emit sortAscendingChanged();
    applyFilterAndSort();
}

QString FileManagerBackend::clipboardOp() const {
    return clipboardOpToString(m_clipboardOp);
}

void FileManagerBackend::setEditorProgram(const QString& prog) {
    const QString trimmed = prog.trimmed();
    if (trimmed.isEmpty()) return;

    QString resolvedProg;
    QStringList resolvedArgs;
    if (!ShellProcessHelper::resolveEditorCommand(trimmed, resolvedProg, resolvedArgs)) {
        const QStringList tokens = QProcess::splitCommand(trimmed);
        resolvedProg = tokens.isEmpty() ? trimmed : tokens.first();
        resolvedArgs = tokens.mid(1);
    }

    if (m_editorProgram == resolvedProg && m_editorArguments == resolvedArgs) return;

    m_editorProgram = resolvedProg;
    m_editorArguments = resolvedArgs;
    emit editorProgramChanged();
}

QStringList FileManagerBackend::editorArgumentsFor(const QString& filePath) const {
    QStringList args = m_editorArguments;
    if (!filePath.isEmpty()) args.append(filePath);
    return args;
}

QString FileManagerBackend::detectEditor() const {
    return ShellProcessHelper::detectEditor();
}

QString FileManagerBackend::currentItemPath() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size()) return QString();
    return m_filteredEntries.at(m_currentIndex).toMap().value(QStringLiteral("path")).toString();
}

bool FileManagerBackend::isCurrentItemDir() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size()) return false;
    return m_filteredEntries.at(m_currentIndex).toMap().value(QStringLiteral("isDir")).toBool();
}

void FileManagerBackend::refreshCurrentAndPreview() {
    refreshCurrent();
}

void FileManagerBackend::refreshAll(const QString& selectPath, bool pathChanged) {
    refreshParent();
    refreshCurrent(selectPath);
    updateDiskUsage();
    updateWatchedDirectory();
    if (pathChanged) {
        emit currentPathChanged();
    }
}

void FileManagerBackend::updateWatchedDirectory() {
    if (!m_fsWatcher) return;
    const QStringList watched = m_fsWatcher->directories();
    if (!watched.isEmpty()) {
        m_fsWatcher->removePaths(watched);
    }
    QStringList toWatch;
    if (!m_currentPath.isEmpty() && QFileInfo::exists(m_currentPath)) toWatch.append(m_currentPath);
    if (!m_parentPath.isEmpty() && QFileInfo::exists(m_parentPath)) toWatch.append(m_parentPath);
    if (!toWatch.isEmpty()) {
        m_fsWatcher->addPaths(toWatch);
    }
}

QString FileManagerBackend::parentDirectoryPath(const QString& path) const {
    if (path.isEmpty()) return QString();

    if (m_useMock) {
        return MockFileSystem::parentDirectoryPath(path);
    }

    QDir dir(path);
    if (dir.isRoot()) return QString();
    if (!dir.cdUp()) return QString();
    return QDir::cleanPath(dir.absolutePath());
}

QString FileManagerBackend::joinPath(const QString& dirPath, const QString& name) const {
    return MockFileSystem::joinPath(dirPath, name);
}

void FileManagerBackend::refreshParent() {
    m_parentEntries.clear();

    const QString newParentPath = parentDirectoryPath(m_currentPath);
    if (m_parentPath != newParentPath) {
        m_parentPath = newParentPath;
        emit parentPathChanged();
    }

    if (m_parentPath.isEmpty()) {
        emit parentEntriesChanged();
        return;
    }

    if (m_useMock) {
        if (const MockItem* parentItem = m_mockFs.find(m_parentPath)) {
            for (const QString& childName : parentItem->childNames) {
                if (!m_showHidden && childName.startsWith(QLatin1Char('.'))) continue;
                const QString childPath = joinPath(m_parentPath, childName);
                if (const MockItem* childItem = m_mockFs.find(childPath)) {
                    m_parentEntries.append(formatFileEntry(
                        childItem->name, childPath, childItem->isDir, childItem->size,
                        childItem->modified, childItem->permissions, childItem->owner,
                        childItem->mimeType));
                }
            }
        }
    } else {
        using ItFlag = QDirListing::IteratorFlag;
        QDirListing::IteratorFlags flags = ItFlag::Default;
        if (m_showHidden) {
            flags |= ItFlag::IncludeHidden;
        }
        for (const auto& entry : QDirListing(m_parentPath, flags)) {
            const bool isDir = entry.isDir();
            const QFileInfo info = entry.fileInfo();
            const QString mime =
                isDir ? QStringLiteral("inode/directory")
                      : m_mimeDb.mimeTypeForFile(entry.filePath(), QMimeDatabase::MatchExtension)
                            .name();
            const QString perms =
                FileEntryFormatter::formatPermissions(info.permissions(), isDir, entry.isSymLink());
            m_parentEntries.append(formatFileEntry(
                entry.fileName(), QDir::cleanPath(entry.filePath()), isDir,
                isDir ? -1 : entry.size(), info.lastModified(), perms, m_userHost, mime));
        }
    }

    std::sort(m_parentEntries.begin(), m_parentEntries.end(),
              [this](const QVariant& a, const QVariant& b) {
                  const QVariantMap ma = a.toMap();
                  const QVariantMap mb = b.toMap();
                  const bool isDirA = ma.value(QStringLiteral("isDir")).toBool();
                  const bool isDirB = mb.value(QStringLiteral("isDir")).toBool();
                  if (isDirA != isDirB) return isDirA;
                  return m_collator.compare(ma.value(QStringLiteral("name")).toString(),
                                            mb.value(QStringLiteral("name")).toString()) < 0;
              });

    emit parentEntriesChanged();
}

void FileManagerBackend::refreshCurrent(const QString& selectPath) {
    m_rawCurrentEntries.clear();

    if (m_useMock) {
        if (const MockItem* curItem = m_mockFs.find(m_currentPath)) {
            for (const QString& childName : curItem->childNames) {
                const QString childPath = joinPath(m_currentPath, childName);
                if (const MockItem* childItem = m_mockFs.find(childPath)) {
                    m_rawCurrentEntries.append(formatFileEntry(
                        childItem->name, childPath, childItem->isDir, childItem->size,
                        childItem->modified, childItem->permissions, childItem->owner,
                        childItem->mimeType));
                }
            }
        }
    } else {
        using ItFlag = QDirListing::IteratorFlag;
        const auto flags = ItFlag::Default | ItFlag::IncludeHidden;
        for (const auto& entry : QDirListing(m_currentPath, flags)) {
            const bool isDir = entry.isDir();
            const QFileInfo info = entry.fileInfo();
            const QString mime =
                isDir ? QStringLiteral("inode/directory")
                      : m_mimeDb.mimeTypeForFile(entry.filePath(), QMimeDatabase::MatchExtension)
                            .name();
            const QString perms =
                FileEntryFormatter::formatPermissions(info.permissions(), isDir, entry.isSymLink());
            m_rawCurrentEntries.append(formatFileEntry(
                entry.fileName(), QDir::cleanPath(entry.filePath()), isDir,
                isDir ? -1 : entry.size(), info.lastModified(), perms, m_userHost, mime));
        }
    }

    applyFilterAndSort(selectPath);
}

void FileManagerBackend::applyFilterAndSort(const QString& selectPath) {
    m_filteredEntries.clear();

    for (const auto& var : std::as_const(m_rawCurrentEntries)) {
        QVariantMap map = var.toMap();
        const QString name = map.value(QStringLiteral("name")).toString();

        if (!m_showHidden && name.startsWith(QLatin1Char('.'))) {
            continue;
        }

        if (!m_filterText.isEmpty() && !name.contains(m_filterText, Qt::CaseInsensitive)) {
            continue;
        }

        map[QStringLiteral("isMarked")] =
            m_markedSet.contains(map.value(QStringLiteral("path")).toString());
        m_filteredEntries.append(map);
    }

    std::sort(m_filteredEntries.begin(), m_filteredEntries.end(),
              [this](const QVariant& a, const QVariant& b) {
                  const QVariantMap ma = a.toMap();
                  const QVariantMap mb = b.toMap();
                  const bool isDirA = ma.value(QStringLiteral("isDir")).toBool();
                  const bool isDirB = mb.value(QStringLiteral("isDir")).toBool();

                  if (isDirA != isDirB) {
                      return isDirA; // Folders always first
                  }

                  int cmp = 0;
                  switch (m_sortField) {
                  case SortField::Name:
                      break;
                  case SortField::Size: {
                      const qint64 sa = ma.value(QStringLiteral("size")).toLongLong();
                      const qint64 sb = mb.value(QStringLiteral("size")).toLongLong();
                      cmp = (sa < sb) ? -1 : (sa > sb ? 1 : 0);
                      break;
                  }
                  case SortField::ModifiedTime: {
                      const QString ta = ma.value(QStringLiteral("modifiedRaw")).toString();
                      const QString tb = mb.value(QStringLiteral("modifiedRaw")).toString();
                      cmp = QString::compare(ta, tb, Qt::CaseInsensitive);
                      break;
                  }
                  case SortField::Type: {
                      const QString ea = ma.value(QStringLiteral("extension")).toString();
                      const QString eb = mb.value(QStringLiteral("extension")).toString();
                      cmp = m_collator.compare(ea, eb);
                      break;
                  }
                  }

                  if (cmp == 0) {
                      const QString na = ma.value(QStringLiteral("name")).toString();
                      const QString nb = mb.value(QStringLiteral("name")).toString();
                      cmp = m_collator.compare(na, nb);
                  }

                  return m_sortAscending ? (cmp < 0) : (cmp > 0);
              });

    emit currentEntriesChanged();

    if (m_filteredEntries.isEmpty()) {
        m_currentIndex = -1;
    } else if (!selectPath.isEmpty()) {
        int foundIndex = -1;
        for (qsizetype i = 0; i < m_filteredEntries.size(); ++i) {
            if (m_filteredEntries.at(i).toMap().value(QStringLiteral("path")).toString() ==
                selectPath) {
                foundIndex = static_cast<int>(i);
                break;
            }
        }
        m_currentIndex =
            (foundIndex >= 0)
                ? foundIndex
                : std::clamp(m_currentIndex, 0, static_cast<int>(m_filteredEntries.size()) - 1);
    } else {
        m_currentIndex =
            std::clamp(m_currentIndex, 0, static_cast<int>(m_filteredEntries.size()) - 1);
    }
    if (m_previewPage != 0) {
        m_previewPage = 0;
        emit previewPageChanged();
    }
    emit currentIndexChanged();
    updatePreview();
}

void FileManagerBackend::syncMarkedStateInFilteredEntries() {
    for (qsizetype i = 0; i < m_filteredEntries.size(); ++i) {
        QVariantMap map = m_filteredEntries.at(i).toMap();
        const QString path = map.value(QStringLiteral("path")).toString();
        const bool marked = m_markedSet.contains(path);
        if (map.value(QStringLiteral("isMarked")).toBool() != marked) {
            map[QStringLiteral("isMarked")] = marked;
            m_filteredEntries[i] = map;
        }
    }
    emit currentEntriesChanged();
}

void FileManagerBackend::updatePreview() {
    m_previewData.clear();

    if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size()) {
        m_previewData[QStringLiteral("type")] = QStringLiteral("empty");
        emit previewDataChanged();
        return;
    }

    const QVariantMap currentItem = m_filteredEntries.at(m_currentIndex).toMap();
    const QString itemPath = currentItem.value(QStringLiteral("path")).toString();
    const QString itemName = currentItem.value(QStringLiteral("name")).toString();
    const bool isDir = currentItem.value(QStringLiteral("isDir")).toBool();
    QString mime = currentItem.value(QStringLiteral("mimeType")).toString();
    QString owner = currentItem.value(QStringLiteral("owner")).toString();

    if (!m_useMock && !isDir) {
        const QFileInfo info(itemPath);
        owner = FileEntryFormatter::formatOwner(info);
        const QMimeType resolvedMime = m_mimeDb.mimeTypeForFile(info, QMimeDatabase::MatchDefault);
        if (resolvedMime.isValid()) mime = resolvedMime.name();
    } else if (!m_useMock && isDir) {
        owner = FileEntryFormatter::formatOwner(QFileInfo(itemPath));
    }

    m_previewData[QStringLiteral("name")] = itemName;
    m_previewData[QStringLiteral("path")] = itemPath;
    m_previewData[QStringLiteral("isDir")] = isDir;
    m_previewData[QStringLiteral("sizeFormatted")] =
        currentItem.value(QStringLiteral("sizeFormatted"));
    m_previewData[QStringLiteral("modifiedFormatted")] =
        currentItem.value(QStringLiteral("modified"));
    m_previewData[QStringLiteral("permissions")] = currentItem.value(QStringLiteral("permissions"));
    m_previewData[QStringLiteral("owner")] = owner;
    m_previewData[QStringLiteral("mimeType")] = mime;
    m_previewData[QStringLiteral("icon")] = currentItem.value(QStringLiteral("icon"));
    m_previewData[QStringLiteral("iconColor")] = currentItem.value(QStringLiteral("iconColor"));

    if (isDir) {
        m_previewData[QStringLiteral("type")] = QStringLiteral("directory");
        QVariantList childPreviews;

        if (m_useMock) {
            if (const MockItem* dirItem = m_mockFs.find(itemPath)) {
                m_previewData[QStringLiteral("itemCount")] = dirItem->childNames.size();
                for (const QString& childName : dirItem->childNames) {
                    const QString childPath = joinPath(itemPath, childName);
                    if (const MockItem* subItem = m_mockFs.find(childPath)) {
                        childPreviews.append(
                            formatFileEntry(subItem->name, childPath, subItem->isDir, subItem->size,
                                            subItem->modified, subItem->permissions, subItem->owner,
                                            subItem->mimeType));
                    }
                }
            }
        } else {
            using ItFlag = QDirListing::IteratorFlag;
            QDirListing::IteratorFlags flags = ItFlag::Default;
            if (m_showHidden) {
                flags |= ItFlag::IncludeHidden;
            }
            int count = 0;
            for (const auto& entry : QDirListing(itemPath, flags)) {
                count++;
                if (childPreviews.size() < 30) {
                    const bool childIsDir = entry.isDir();
                    const QFileInfo childInfo = entry.fileInfo();
                    const QString childMime =
                        childIsDir
                            ? QStringLiteral("inode/directory")
                            : m_mimeDb
                                  .mimeTypeForFile(entry.filePath(), QMimeDatabase::MatchExtension)
                                  .name();
                    const QString perms = FileEntryFormatter::formatPermissions(
                        childInfo.permissions(), childIsDir, entry.isSymLink());
                    childPreviews.append(
                        formatFileEntry(entry.fileName(), QDir::cleanPath(entry.filePath()),
                                        childIsDir, childIsDir ? -1 : entry.size(),
                                        childInfo.lastModified(), perms, m_userHost, childMime));
                }
            }
            m_previewData[QStringLiteral("itemCount")] = count;
        }
        m_previewData[QStringLiteral("dirChildren")] = childPreviews;
    } else {
        if (FileEntryFormatter::isTextFile(m_mimeDb, m_useMock, itemPath, itemName, mime)) {
            m_previewData[QStringLiteral("type")] = QStringLiteral("text");
            QString rawText;
            if (m_useMock) {
                if (const MockItem* fileItem = m_mockFs.find(itemPath)) {
                    rawText = fileItem->content;
                }
            } else {
                QFile file(itemPath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    int lineCount = 0;
                    while (!stream.atEnd() && lineCount < 200) {
                        rawText += stream.readLine() + QLatin1Char('\n');
                        lineCount++;
                    }
                }
            }

            const QString lowerName = itemName.toLower();
            const bool isMarkdown = lowerName.endsWith(QStringLiteral(".md")) ||
                                    lowerName.endsWith(QStringLiteral(".markdown")) ||
                                    mime.contains(QStringLiteral("markdown"), Qt::CaseInsensitive);

            const FormattedTextPreview formatted =
                PreviewSyntaxHighlighter::formatPreview(rawText, m_previewWidth, isMarkdown);
            m_previewData[QStringLiteral("rawText")] = formatted.rawText;
            m_previewData[QStringLiteral("lineCount")] = formatted.lines.size();
            m_previewData[QStringLiteral("textLines")] = formatted.lines;
            m_previewData[QStringLiteral("formattedText")] = formatted.formattedText;
            m_previewData[QStringLiteral("highlightedText")] = formatted.highlightedText;
        } else if (FileEntryFormatter::isVisualPreviewFile(itemName, mime)) {
            m_previewData[QStringLiteral("type")] = QStringLiteral("image");
            const QString lowerName = itemName.toLower();
            const QString lowerMime = mime.toLower();
            const bool isSvg = lowerName.endsWith(QStringLiteral(".svg")) ||
                               lowerName.endsWith(QStringLiteral(".svgz")) ||
                               lowerMime.contains(QStringLiteral("svg"));
            const bool isPdf = lowerName.endsWith(QStringLiteral(".pdf")) ||
                               lowerMime.contains(QStringLiteral("pdf"));

            if (isSvg) {
                m_previewData[QStringLiteral("mediaKind")] = QStringLiteral("svg");
                m_previewData[QStringLiteral("pageCount")] = 1;
                m_previewData[QStringLiteral("dimensions")] = QStringLiteral("Vector SVG");
            } else if (isPdf) {
                m_previewData[QStringLiteral("mediaKind")] = QStringLiteral("pdf");
                int pages = 3;
                if (!m_useMock) {
#if FILE_MANAGER_HAVE_PDF
                    QPdfDocument doc;
                    if (doc.load(itemPath) == QPdfDocument::Error::None && doc.pageCount() > 0) {
                        pages = doc.pageCount();
                    } else {
                        pages = 1;
                    }
#else
                    pages = 1;
#endif
                }
                m_previewData[QStringLiteral("pageCount")] = pages;
                m_previewData[QStringLiteral("dimensions")] =
                    QStringLiteral("%1 page%2")
                        .arg(pages)
                        .arg(pages == 1 ? QString() : QStringLiteral("s"));
            } else {
                m_previewData[QStringLiteral("mediaKind")] = QStringLiteral("raster");
                m_previewData[QStringLiteral("pageCount")] = 1;
                if (m_useMock) {
                    m_previewData[QStringLiteral("dimensions")] =
                        lowerName.contains(QStringLiteral("banner"))
                            ? QStringLiteral("1920x1080 px")
                            : QStringLiteral("512x512 px");
                } else {
                    QImageReader reader(itemPath);
                    const QSize imgSize = reader.size();
                    if (imgSize.isValid() && !imgSize.isEmpty()) {
                        m_previewData[QStringLiteral("dimensions")] =
                            QStringLiteral("%1x%2 px").arg(imgSize.width()).arg(imgSize.height());
                    } else {
                        m_previewData[QStringLiteral("dimensions")] =
                            QStringLiteral("Raster Image");
                    }
                }
            }
        } else {
            m_previewData[QStringLiteral("type")] = QStringLiteral("binary");
        }
    }

    emit previewDataChanged();
}

void FileManagerBackend::updateDiskUsage() {
    if (m_useMock) {
        m_diskUsage = QStringLiteral("Free: 42.8 GB / 256.0 GB");
    } else {
        const QStorageInfo storage(m_currentPath);
        if (storage.isValid() && storage.bytesTotal() > 0) {
            const QString freeStr = FileEntryFormatter::formatSize(storage.bytesAvailable());
            const QString totalStr = FileEntryFormatter::formatSize(storage.bytesTotal());
            m_diskUsage = QStringLiteral("Free: %1 / %2").arg(freeStr, totalStr);
        } else {
            m_diskUsage = QStringLiteral("Storage: Ready");
        }
    }
    emit diskUsageChanged();
}

QVariantMap FileManagerBackend::formatFileEntry(const QString& name, const QString& fullPath,
                                                bool isDir, qint64 size, const QDateTime& modified,
                                                const QString& permissions, const QString& owner,
                                                const QString& mimeType) const {
    return FileEntryFormatter::formatFileEntry(name, fullPath, isDir, size, modified, permissions,
                                               owner, mimeType, m_markedSet.contains(fullPath));
}

QString FileManagerBackend::sortFieldToString(SortField field) {
    switch (field) {
    case SortField::Name:
        return QStringLiteral("name");
    case SortField::Size:
        return QStringLiteral("size");
    case SortField::ModifiedTime:
        return QStringLiteral("mtime");
    case SortField::Type:
        return QStringLiteral("type");
    }
    return QStringLiteral("name");
}

FileManagerBackend::SortField FileManagerBackend::sortFieldFromString(const QString& field) {
    const QString lower = field.trimmed().toLower();
    if (lower == QStringLiteral("size")) return SortField::Size;
    if (lower == QStringLiteral("mtime")) return SortField::ModifiedTime;
    if (lower == QStringLiteral("type")) return SortField::Type;
    return SortField::Name;
}

QString FileManagerBackend::clipboardOpToString(ClipboardOp op) {
    switch (op) {
    case ClipboardOp::None:
        return QString();
    case ClipboardOp::Copy:
        return QStringLiteral("copy");
    case ClipboardOp::Cut:
        return QStringLiteral("cut");
    }
    return QString();
}

void FileManagerBackend::navigateTo(const QString& path) {
    const QString clean = QDir::cleanPath(path);
    if (clean.isEmpty() || m_currentPath == clean) return;
    m_currentPath = clean;
    m_currentIndex = 0;
    if (!m_filterText.isEmpty()) {
        m_filterText.clear();
        emit filterTextChanged();
    }
    refreshAll();
}

void FileManagerBackend::navigateUp() {
    if (m_parentPath.isEmpty()) return;
    const QString prevCurrent = m_currentPath;
    m_currentPath = m_parentPath;
    m_currentIndex = 0;
    if (!m_filterText.isEmpty()) {
        m_filterText.clear();
        emit filterTextChanged();
    }
    refreshAll(prevCurrent);
}

void FileManagerBackend::navigateIntoCurrent() {
    if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size()) return;
    const QVariantMap item = m_filteredEntries.at(m_currentIndex).toMap();
    if (item.value(QStringLiteral("isDir")).toBool()) {
        navigateTo(item.value(QStringLiteral("path")).toString());
    } else {
        showToast(
            QStringLiteral("Opened file: %1").arg(item.value(QStringLiteral("name")).toString()));
    }
}

void FileManagerBackend::toggleMarkCurrent() {
    toggleMark(m_currentIndex);
}

void FileManagerBackend::toggleMark(int index) {
    if (index < 0 || index >= m_filteredEntries.size()) return;
    QVariantMap item = m_filteredEntries.at(index).toMap();
    const QString path = item.value(QStringLiteral("path")).toString();
    bool isNowMarked = false;
    if (m_markedSet.contains(path)) {
        m_markedSet.remove(path);
        m_markedPaths.removeAll(path);
    } else {
        m_markedSet.insert(path);
        m_markedPaths.append(path);
        isNowMarked = true;
    }
    item[QStringLiteral("isMarked")] = isNowMarked;
    m_filteredEntries[index] = item;
    emit markedPathsChanged();
    emit currentEntriesChanged();
}

void FileManagerBackend::selectAll() {
    if (m_markedSet.size() == m_filteredEntries.size()) {
        m_markedSet.clear();
        m_markedPaths.clear();
    } else {
        m_markedSet.clear();
        m_markedPaths.clear();
        for (const auto& item : std::as_const(m_filteredEntries)) {
            const QString path = item.toMap().value(QStringLiteral("path")).toString();
            m_markedSet.insert(path);
            m_markedPaths.append(path);
        }
    }
    emit markedPathsChanged();
    syncMarkedStateInFilteredEntries();
}

void FileManagerBackend::clearMarks() {
    if (m_markedPaths.isEmpty()) return;
    m_markedSet.clear();
    m_markedPaths.clear();
    emit markedPathsChanged();
    syncMarkedStateInFilteredEntries();
}

void FileManagerBackend::copyMarked() {
    m_clipboardPaths.clear();
    if (m_markedPaths.isEmpty() && m_currentIndex >= 0 &&
        m_currentIndex < m_filteredEntries.size()) {
        m_clipboardPaths.append(
            m_filteredEntries.at(m_currentIndex).toMap().value(QStringLiteral("path")).toString());
    } else {
        m_clipboardPaths = m_markedPaths;
    }
    m_clipboardOp = ClipboardOp::Copy;
    emit clipboardChanged();
    showToast(QStringLiteral("Yanked %1 item(s)").arg(m_clipboardPaths.size()));
}

void FileManagerBackend::cutMarked() {
    m_clipboardPaths.clear();
    if (m_markedPaths.isEmpty() && m_currentIndex >= 0 &&
        m_currentIndex < m_filteredEntries.size()) {
        m_clipboardPaths.append(
            m_filteredEntries.at(m_currentIndex).toMap().value(QStringLiteral("path")).toString());
    } else {
        m_clipboardPaths = m_markedPaths;
    }
    m_clipboardOp = ClipboardOp::Cut;
    emit clipboardChanged();
    showToast(QStringLiteral("Cut %1 item(s)").arg(m_clipboardPaths.size()));
}

void FileManagerBackend::pasteClipboard() {
    if (m_clipboardPaths.isEmpty() || m_clipboardOp == ClipboardOp::None) {
        showToast(QStringLiteral("Clipboard empty"));
        return;
    }

    int count = 0;
    for (const QString& srcPath : std::as_const(m_clipboardPaths)) {
        const QString name = QFileInfo(srcPath).fileName();
        const QString destPath = joinPath(m_currentPath, name);
        if (srcPath == destPath) continue;

        if (m_useMock) {
            if (m_mockFs.copyRecursive(srcPath, destPath)) {
                if (m_clipboardOp == ClipboardOp::Cut) {
                    m_mockFs.removeRecursive(srcPath);
                }
                count++;
            }
        } else {
            if (m_clipboardOp == ClipboardOp::Cut) {
                if (QFile::rename(srcPath, destPath)) {
                    count++;
                } else if (FileEntryFormatter::copyPathRecursively(srcPath, destPath)) {
                    const QFileInfo srcInfo(srcPath);
                    if (srcInfo.isDir())
                        QDir(srcPath).removeRecursively();
                    else
                        QFile::remove(srcPath);
                    count++;
                }
            } else {
                if (FileEntryFormatter::copyPathRecursively(srcPath, destPath)) count++;
            }
        }
    }

    if (m_clipboardOp == ClipboardOp::Cut) {
        m_clipboardPaths.clear();
        m_clipboardOp = ClipboardOp::None;
        emit clipboardChanged();
    }

    showToast(QStringLiteral("Pasted %1 item(s)").arg(count));
    refreshAll(QString(), false);
}

bool FileManagerBackend::createItem(const QString& name, bool isDirectory) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return false;
    const QString targetPath = joinPath(m_currentPath, trimmed);

    if (m_useMock) {
        if (!m_mockFs.createItem(m_currentPath, trimmed, isDirectory)) return false;
    } else {
        if (isDirectory) {
            QDir dir(m_currentPath);
            if (!dir.mkdir(trimmed)) return false;
        } else {
            if (QFileInfo::exists(targetPath)) return false;
            QFile file(targetPath);
            if (!file.open(QIODevice::WriteOnly)) return false;
            file.close();
        }
    }

    showToast(QStringLiteral("Created %1: %2")
                  .arg(isDirectory ? QStringLiteral("dir") : QStringLiteral("file"), trimmed));
    refreshAll(targetPath, false);
    return true;
}

bool FileManagerBackend::deleteCurrentOrMarked() {
    QStringList toDelete;
    if (!m_markedPaths.isEmpty()) {
        toDelete = m_markedPaths;
    } else if (m_currentIndex >= 0 && m_currentIndex < m_filteredEntries.size()) {
        toDelete.append(
            m_filteredEntries.at(m_currentIndex).toMap().value(QStringLiteral("path")).toString());
    }

    if (toDelete.isEmpty()) return false;

    int deleted = 0;
    for (const QString& path : std::as_const(toDelete)) {
        if (m_useMock) {
            if (m_mockFs.removeRecursive(path)) {
                deleted++;
            }
        } else {
            if (QFile::moveToTrash(path)) {
                deleted++;
            } else {
                const QFileInfo info(path);
                if (info.isDir()) {
                    QDir dir(path);
                    if (dir.removeRecursively()) deleted++;
                } else if (QFile::remove(path)) {
                    deleted++;
                }
            }
        }
    }

    m_markedSet.clear();
    m_markedPaths.clear();
    emit markedPathsChanged();
    showToast(QStringLiteral("Deleted %1 item(s)").arg(deleted));
    refreshAll(QString(), false);
    return true;
}

bool FileManagerBackend::renameCurrent(const QString& newName) {
    const QString trimmed = newName.trimmed();
    if (trimmed.isEmpty() || m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size())
        return false;
    const QVariantMap currentItem = m_filteredEntries.at(m_currentIndex).toMap();
    const QString oldPath = currentItem.value(QStringLiteral("path")).toString();
    const QString oldName = currentItem.value(QStringLiteral("name")).toString();
    const QString newPath = joinPath(m_currentPath, trimmed);

    if (oldName == trimmed) return true;

    if (m_useMock) {
        if (!m_mockFs.renameRecursive(oldPath, newPath, trimmed)) return false;
    } else {
        if (!QFile::rename(oldPath, newPath)) return false;
    }

    showToast(QStringLiteral("Renamed to: %1").arg(trimmed));
    refreshAll(newPath, false);
    return true;
}

void FileManagerBackend::cycleSort() {
    switch (m_sortField) {
    case SortField::Name:
        m_sortField = SortField::Size;
        showToast(QStringLiteral("Sort by: Size"));
        break;
    case SortField::Size:
        m_sortField = SortField::ModifiedTime;
        showToast(QStringLiteral("Sort by: Modified Time"));
        break;
    case SortField::ModifiedTime:
        m_sortField = SortField::Type;
        showToast(QStringLiteral("Sort by: Type / Extension"));
        break;
    case SortField::Type:
        m_sortField = SortField::Name;
        showToast(QStringLiteral("Sort by: Name"));
        break;
    }
    emit sortFieldChanged();
    applyFilterAndSort();
}

void FileManagerBackend::toggleShowHidden() {
    setShowHidden(!m_showHidden);
    showToast(m_showHidden ? QStringLiteral("Showing hidden files")
                           : QStringLiteral("Hiding dotfiles"));
}

void FileManagerBackend::showToast(const QString& msg) {
    m_statusMessage = msg;
    emit statusMessageChanged();
}

void FileManagerBackend::setShellWorkingDirectory(const QString& path) {
    const QString clean = QDir::cleanPath(path.trimmed());
    if (clean.isEmpty() || m_shellWorkingDirectory == clean) return;
    m_shellWorkingDirectory = clean;
    emit shellWorkingDirectoryChanged();
}

void FileManagerBackend::setMockShellBusy(bool busy) {
    if (m_mockShellBusy == busy) return;
    m_mockShellBusy = busy;
    emit mockShellBusyChanged();
}

bool FileManagerBackend::isShellForegroundBusy(qint64 pid) const {
    if (m_useMock) return m_mockShellBusy;
    return ShellProcessHelper::isForegroundBusy(pid);
}

QString FileManagerBackend::buildShellCdCommand() {
    if (m_currentPath.isEmpty()) return QString();
    setShellWorkingDirectory(m_currentPath);
    showToast(QStringLiteral("Shell cd → %1").arg(m_currentPath));
    return QStringLiteral("cd -- %1\n").arg(ShellProcessHelper::shellQuote(m_currentPath));
}

bool FileManagerBackend::syncDirectoryFromShell(qint64 pid, const QString& ptyTitle,
                                                bool onlyIfShellChanged) {
    bool shellChanged = false;
    if (!m_useMock) {
        const QString resolved = ShellProcessHelper::resolveWorkingDirectory(pid, ptyTitle);
        if (!resolved.isEmpty() && resolved != m_shellWorkingDirectory) {
            setShellWorkingDirectory(resolved);
            shellChanged = true;
        }
    }

    if (onlyIfShellChanged && !shellChanged) {
        return false;
    }

    const QString targetDir =
        m_shellWorkingDirectory.isEmpty() ? m_currentPath : m_shellWorkingDirectory;
    if (targetDir.isEmpty()) return false;

    if (m_currentPath != targetDir) {
        navigateTo(targetDir);
        showToast(QStringLiteral("Synced to shell: %1").arg(m_currentPath));
    }
    return true;
}

} // namespace qmltui::examples
