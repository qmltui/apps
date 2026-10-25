#include "FileManagerBackend.h"

#include <QDir>
#include <QDirListing>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QMimeType>
#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QSysInfo>
#include <QTextStream>
#include <algorithm>

namespace qmltui::examples {

namespace {
bool resolveEditorCommand(const QString &rawCommand, QString &outProgram,
                          QStringList &outArgs) {
  const QString trimmed = rawCommand.trimmed();
  if (trimmed.isEmpty())
    return false;

  const QStringList tokens = QProcess::splitCommand(trimmed);
  if (tokens.isEmpty())
    return false;

  const QString candidate = QDir::fromNativeSeparators(tokens.first());
  if (candidate.contains(QLatin1Char('/'))) {
    const QFileInfo info(candidate);
    if (!info.exists() || !info.isExecutable())
      return false;
    outProgram = info.absoluteFilePath();
  } else {
    const QString found = QStandardPaths::findExecutable(candidate);
    if (found.isEmpty())
      return false;
    outProgram = candidate;
  }

  outArgs = tokens.mid(1);
  return true;
}

bool copyPathRecursively(const QString &srcPath, const QString &destPath) {
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

QStringList wrapTextLine(const QString &rawLine, int maxW) {
  if (maxW <= 0)
    return {rawLine};
  QString line = rawLine;
  line.replace(QLatin1Char('\t'), QStringLiteral("    "));
  if (line.isEmpty())
    return {QString()};
  if (line.size() <= maxW)
    return {line};

  QStringList chunks;
  QString current;
  const QStringList words = line.split(QLatin1Char(' '));

  for (qsizetype i = 0; i < words.size(); ++i) {
    const QString &word = words.at(i);
    if (word.size() > maxW) {
      if (!current.isEmpty()) {
        chunks.append(current);
        current.clear();
      }
      for (qsizetype k = 0; k < word.size(); k += maxW) {
        chunks.append(word.mid(k, maxW));
      }
      continue;
    }

    if (current.isEmpty()) {
      current = word;
    } else if (current.size() + 1 + word.size() <= maxW) {
      current += QLatin1Char(' ') + word;
    } else {
      chunks.append(current);
      current = word;
    }
  }
  if (!current.isEmpty()) {
    chunks.append(current);
  }
  return chunks.isEmpty() ? QStringList{line} : chunks;
}
} // namespace

FileManagerBackend::FileManagerBackend(QObject *parent) : QObject(parent) {
  const bool scenarioRequested =
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_SCRIPT") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_EVAL") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_CAPTURE_AFTER_MS") ||
      qEnvironmentVariableIsSet("QMLTUI_FM_MOCK");

  if (scenarioRequested && !qEnvironmentVariableIsSet("QMLTUI_FM_LIVE")) {
    m_useMock = true;
  }

  m_collator.setNumericMode(true);
  m_collator.setCaseSensitivity(Qt::CaseInsensitive);

  QString user = qEnvironmentVariable("USER");
  if (user.isEmpty())
    user = qEnvironmentVariable("USERNAME", QStringLiteral("user"));
  QString host = QSysInfo::machineHostName();
  if (host.isEmpty())
    host = QStringLiteral("terminal");
  m_userHost = user + QLatin1Char('@') + host;

  setEditorProgram(detectEditor());

  if (m_useMock) {
    initMockFileSystem();
    m_currentPath = QStringLiteral("/workspace");
  } else {
    m_currentPath = QDir::cleanPath(QDir::currentPath());
    m_fsWatcher = new QFileSystemWatcher(this);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this,
            [this](const QString &changedDir) {
              if (QDir::cleanPath(changedDir) == m_currentPath) {
                refreshCurrent();
              } else if (QDir::cleanPath(changedDir) == m_parentPath) {
                refreshParent();
              }
            });
  }

  refreshAll();
}

void FileManagerBackend::initMockFileSystem() {
  m_mockFs.clear();

  const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

  auto addDir = [this, &nowUtc](
                    const QString &path, const QString &name,
                    const QStringList &children,
                    const QString &perms = QStringLiteral("drwxr-xr-x")) {
    MockItem item;
    item.name = name;
    item.isDir = true;
    item.size = children.size();
    item.modified = nowUtc.addDays(-2);
    item.permissions = perms;
    item.owner = QStringLiteral("yosri:users");
    item.mimeType = QStringLiteral("inode/directory");
    item.childNames = children;
    m_mockFs.insert(path, item);
  };

  auto addFile = [this, &nowUtc](
                     const QString &path, const QString &name,
                     const QString &content, const QString &mime,
                     qint64 size = 0,
                     const QString &perms = QStringLiteral("-rw-r--r--")) {
    MockItem item;
    item.name = name;
    item.isDir = false;
    item.content = content;
    item.size = size > 0 ? size : content.toUtf8().size();
    item.modified = nowUtc.addSecs(-3600 * (content.size() % 24 + 1));
    item.permissions = perms;
    item.owner = QStringLiteral("yosri:users");
    item.mimeType = mime;
    m_mockFs.insert(path, item);
  };

  // Root
  addDir(QStringLiteral("/"), QStringLiteral("root"),
         {QStringLiteral("workspace"), QStringLiteral("home"),
          QStringLiteral("etc")});
  addDir(QStringLiteral("/workspace"), QStringLiteral("workspace"),
         {QStringLiteral("assets"), QStringLiteral("docs"),
          QStringLiteral("src"), QStringLiteral("tests"),
          QStringLiteral(".git"), QStringLiteral(".env"),
          QStringLiteral("CMakeLists.txt"), QStringLiteral("LICENSE"),
          QStringLiteral("README.md"), QStringLiteral("package.json")});

  // Subdirectories
  addDir(QStringLiteral("/workspace/docs"), QStringLiteral("docs"),
         {QStringLiteral("architecture.md"), QStringLiteral("api_reference.md"),
          QStringLiteral("user_guide.txt")});
  addDir(QStringLiteral("/workspace/src"), QStringLiteral("src"),
         {QStringLiteral("main.cpp"), QStringLiteral("App.cpp"),
          QStringLiteral("App.h"), QStringLiteral("utils.rs"),
          QStringLiteral("deploy.py"), QStringLiteral("config.json")});
  addDir(QStringLiteral("/workspace/assets"), QStringLiteral("assets"),
         {QStringLiteral("logo.png"), QStringLiteral("banner.jpg"),
          QStringLiteral("archive.tar.gz"), QStringLiteral("theme.yaml")});
  addDir(QStringLiteral("/workspace/tests"), QStringLiteral("tests"),
         {QStringLiteral("test_main.cpp"), QStringLiteral("test_cases.json")});
  addDir(QStringLiteral("/workspace/.git"), QStringLiteral(".git"),
         {QStringLiteral("HEAD"), QStringLiteral("config")});

  // Files in /workspace
  addFile(QStringLiteral("/workspace/README.md"), QStringLiteral("README.md"),
          QStringLiteral(
              "# Terminal File Manager\n\n"
              "A fast, asynchronous, 3-column Miller column file manager\n"
              "built with pure QML and QmlTui.\n\n"
              "## Features\n"
              "- 3-Column Miller layout (Parent, Current, Preview)\n"
              "- Embedded shell in the current directory\n"
              "- Vim-style keybindings (h/j/k/l, Space, /, d, r, y, p)\n"
              "- Live filtering and fast fuzzy search\n"
              "- Syntax preview with line numbers\n"
              "- Directory inspection and file metadata cards\n"
              "- Multi-file selection and clipboard management\n\n"
              "## Shortcuts\n"
              "- `h/l`: Navigate parent / child directory\n"
              "- `j/k`: Move cursor down / up\n"
              "- `Space`: Toggle select file\n"
              "- `/`: Instant live filter\n"
              "- `?`: Cheatsheet help modal\n"
              "- `q`: Quit"),
          QStringLiteral("text/markdown"));

  addFile(
      QStringLiteral("/workspace/CMakeLists.txt"),
      QStringLiteral("CMakeLists.txt"),
      QStringLiteral("cmake_minimum_required(VERSION 3.22)\n"
                     "project(FileManager VERSION 1.0.0 LANGUAGES CXX)\n\n"
                     "set(CMAKE_CXX_STANDARD 20)\n"
                     "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
                     "find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml)\n"
                     "find_package(QmlTui REQUIRED)\n\n"
                     "qt_add_executable(qmltui_file_manager main.cpp)\n"
                     "target_link_libraries(qmltui_file_manager PRIVATE "
                     "Qt6::Core qmltui::core)"),
      QStringLiteral("text/x-cmake"));

  addFile(QStringLiteral("/workspace/package.json"),
          QStringLiteral("package.json"),
          QStringLiteral(
              "{\n"
              "  \"name\": \"file-manager\",\n"
              "  \"version\": \"1.0.0\",\n"
              "  \"description\": \"Retained-mode terminal file manager\",\n"
              "  \"main\": \"Main.qml\",\n"
              "  \"scripts\": {\n"
              "    \"start\": \"./build/qmltui_file_manager\",\n"
              "    \"test\": \"ctest --test-dir build\"\n"
              "  },\n"
              "  \"author\": \"QmlTui Team\",\n"
              "  \"license\": \"MIT\"\n"
              "}"),
          QStringLiteral("application/json"));

  addFile(QStringLiteral("/workspace/LICENSE"), QStringLiteral("LICENSE"),
          QStringLiteral("MIT License\n\n"
                         "Copyright (c) 2026 QmlTui Contributors\n\n"
                         "Permission is hereby granted, free of charge, to any "
                         "person obtaining a copy\n"
                         "of this software and associated documentation files "
                         "(the \"Software\"), to deal\n"
                         "in the Software without restriction..."),
          QStringLiteral("text/plain"));

  addFile(QStringLiteral("/workspace/.env"), QStringLiteral(".env"),
          QStringLiteral("APP_ENV=production\n"
                         "LOG_LEVEL=debug\n"
                         "MAX_PREVIEW_LINES=200\n"
                         "ENABLE_NERD_FONTS=true"),
          QStringLiteral("text/plain"));

  // Files in /workspace/docs
  addFile(QStringLiteral("/workspace/docs/architecture.md"),
          QStringLiteral("architecture.md"),
          QStringLiteral(
              "# Architecture Overview\n\n"
              "1. **Core Engine**: C++20 backend with reactive properties.\n"
              "2. **Declarative UI**: Pure QML frontend utilizing QmlTui.\n"
              "3. **Miller Navigation**: 3 synchronized reactive viewports."),
          QStringLiteral("text/markdown"));

  addFile(QStringLiteral("/workspace/docs/api_reference.md"),
          QStringLiteral("api_reference.md"),
          QStringLiteral("# API Reference\n\n"
                         "- `navigateTo(path)`: Traverse directory\n"
                         "- `toggleMark(index)`: Select item\n"
                         "- `copyMarked()`: Copy selection"),
          QStringLiteral("text/markdown"));

  addFile(QStringLiteral("/workspace/docs/user_guide.txt"),
          QStringLiteral("user_guide.txt"),
          QStringLiteral("FILE MANAGER USER MANUAL\n"
                         "========================\n"
                         "Welcome to the QmlTui terminal file manager."),
          QStringLiteral("text/plain"));

  // Files in /workspace/src
  addFile(QStringLiteral("/workspace/src/main.cpp"), QStringLiteral("main.cpp"),
          QStringLiteral(
              "#include <QCoreApplication>\n"
              "#include <QQmlApplicationEngine>\n\n"
              "int main(int argc, char *argv[]) {\n"
              "    QCoreApplication app(argc, argv);\n"
              "    QQmlApplicationEngine engine;\n"
              "    engine.loadFromModule(\"QmlTui.Examples.FileManager\", "
              "\"Main\");\n"
              "    return app.exec();\n"
              "}"),
          QStringLiteral("text/x-c++src"));

  addFile(QStringLiteral("/workspace/src/App.cpp"), QStringLiteral("App.cpp"),
          QStringLiteral("#include \"App.h\"\n\n"
                         "App::App(QObject *parent) : QObject(parent) {\n"
                         "    // Initialization logic\n"
                         "}"),
          QStringLiteral("text/x-c++src"));

  addFile(QStringLiteral("/workspace/src/App.h"), QStringLiteral("App.h"),
          QStringLiteral("#pragma once\n#include <QObject>\n\n"
                         "class App : public QObject {\n"
                         "    Q_OBJECT\n"
                         "public:\n"
                         "    explicit App(QObject *parent = nullptr);\n"
                         "};"),
          QStringLiteral("text/x-c++hdr"));

  addFile(
      QStringLiteral("/workspace/src/utils.rs"), QStringLiteral("utils.rs"),
      QStringLiteral("pub fn format_size(bytes: u64) -> String {\n"
                     "    if bytes < 1024 {\n"
                     "        format!(\"{} B\", bytes)\n"
                     "    } else {\n"
                     "        format!(\"{:.1} KB\", bytes as f64 / 1024.0)\n"
                     "    }\n"
                     "}"),
      QStringLiteral("text/x-rust"));

  addFile(QStringLiteral("/workspace/src/deploy.py"),
          QStringLiteral("deploy.py"),
          QStringLiteral("#!/usr/bin/env python3\n"
                         "import sys, os\n\n"
                         "def deploy():\n"
                         "    print('Deploying QmlTui application...')\n\n"
                         "if __name__ == '__main__':\n"
                         "    deploy()"),
          QStringLiteral("text/x-python"));

  addFile(QStringLiteral("/workspace/src/config.json"),
          QStringLiteral("config.json"),
          QStringLiteral("{\n"
                         "  \"theme\": \"tokyo_night\",\n"
                         "  \"show_hidden\": false,\n"
                         "  \"columns_ratio\": [20, 35, 45],\n"
                         "  \"sort_by\": \"name\"\n"
                         "}"),
          QStringLiteral("application/json"));

  // Files in /workspace/assets
  addFile(QStringLiteral("/workspace/assets/logo.png"),
          QStringLiteral("logo.png"), QStringLiteral(""),
          QStringLiteral("image/png"), 48291);
  addFile(QStringLiteral("/workspace/assets/banner.jpg"),
          QStringLiteral("banner.jpg"), QStringLiteral(""),
          QStringLiteral("image/jpeg"), 125890);
  addFile(QStringLiteral("/workspace/assets/archive.tar.gz"),
          QStringLiteral("archive.tar.gz"), QStringLiteral(""),
          QStringLiteral("application/gzip"), 2048576);
  addFile(QStringLiteral("/workspace/assets/theme.yaml"),
          QStringLiteral("theme.yaml"),
          QStringLiteral("name: Catppuccin Mocha\n"
                         "primary: '#89b4fa'\n"
                         "secondary: '#cdd6f4'\n"
                         "background: '#1e1e2e'"),
          QStringLiteral("text/yaml"));

  // Files in /workspace/tests
  addFile(QStringLiteral("/workspace/tests/test_main.cpp"),
          QStringLiteral("test_main.cpp"),
          QStringLiteral("#include <QtTest>\n\n"
                         "class TestFileManager : public QObject {\n"
                         "    Q_OBJECT\n"
                         "private slots:\n"
                         "    void testNavigation();\n"
                         "};\n\n"
                         "QTEST_MAIN(TestFileManager)\n"
                         "#include \"test_main.moc\""),
          QStringLiteral("text/x-c++src"));

  addFile(
      QStringLiteral("/workspace/tests/test_cases.json"),
      QStringLiteral("test_cases.json"),
      QStringLiteral("[\n  {\"test\": \"navigation\", \"expected\": true},\n  "
                     "{\"test\": \"preview\", \"expected\": true}\n]"),
      QStringLiteral("application/json"));

  // Files in /workspace/.git
  addFile(QStringLiteral("/workspace/.git/HEAD"), QStringLiteral("HEAD"),
          QStringLiteral("ref: refs/heads/main\n"),
          QStringLiteral("text/plain"));
  addFile(QStringLiteral("/workspace/.git/config"), QStringLiteral("config"),
          QStringLiteral(
              "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n"),
          QStringLiteral("text/plain"));
}

QString FileManagerBackend::breadcrumbs() const {
  if (m_currentPath.isEmpty() || m_currentPath == QStringLiteral("/")) {
    return QStringLiteral(" /");
  }

  QString clean = QDir::fromNativeSeparators(m_currentPath);
  if (m_useMock) {
    if (clean.startsWith(QStringLiteral("/workspace"))) {
      clean = QStringLiteral("~") + clean.mid(10);
    }
  } else {
    const QString home = QDir::fromNativeSeparators(QDir::homePath());
    if (!home.isEmpty() && home != QStringLiteral("/") &&
        clean.startsWith(home) &&
        (clean.size() == home.size() ||
         clean.at(home.size()) == QLatin1Char('/'))) {
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
    m_currentIndex =
        std::clamp(index, 0, static_cast<int>(m_filteredEntries.size()) - 1);
  }
  emit currentIndexChanged();
  updatePreview();
}

void FileManagerBackend::setPreviewWidth(int w) {
  if (m_previewWidth == w || w <= 0)
    return;
  m_previewWidth = w;
  emit previewWidthChanged();
  if (m_previewData.value(QStringLiteral("type")).toString() ==
      QStringLiteral("text")) {
    updatePreview();
  }
}

void FileManagerBackend::setFilterText(const QString &filter) {
  if (m_filterText == filter)
    return;
  m_filterText = filter;
  emit filterTextChanged();
  applyFilterAndSort();
}

void FileManagerBackend::setShowHidden(bool show) {
  if (m_showHidden == show)
    return;
  m_showHidden = show;
  emit showHiddenChanged();
  refreshParent();
  applyFilterAndSort();
}

QString FileManagerBackend::sortField() const {
  return sortFieldToString(m_sortField);
}

void FileManagerBackend::setSortField(const QString &field) {
  const SortField parsed = sortFieldFromString(field);
  if (m_sortField == parsed)
    return;
  m_sortField = parsed;
  emit sortFieldChanged();
  applyFilterAndSort();
}

void FileManagerBackend::setSortAscending(bool ascending) {
  if (m_sortAscending == ascending)
    return;
  m_sortAscending = ascending;
  emit sortAscendingChanged();
  applyFilterAndSort();
}

QString FileManagerBackend::clipboardOp() const {
  return clipboardOpToString(m_clipboardOp);
}

void FileManagerBackend::setEditorProgram(const QString &prog) {
  const QString trimmed = prog.trimmed();
  if (trimmed.isEmpty())
    return;

  QString resolvedProg;
  QStringList resolvedArgs;
  if (!resolveEditorCommand(trimmed, resolvedProg, resolvedArgs)) {
    const QStringList tokens = QProcess::splitCommand(trimmed);
    resolvedProg = tokens.isEmpty() ? trimmed : tokens.first();
    resolvedArgs = tokens.mid(1);
  }

  if (m_editorProgram == resolvedProg && m_editorArguments == resolvedArgs)
    return;

  m_editorProgram = resolvedProg;
  m_editorArguments = resolvedArgs;
  emit editorProgramChanged();
}

QStringList
FileManagerBackend::editorArgumentsFor(const QString &filePath) const {
  QStringList args = m_editorArguments;
  if (!filePath.isEmpty())
    args.append(filePath);
  return args;
}

QString FileManagerBackend::detectEditor() const {
  QString prog;
  QStringList args;
  if (qEnvironmentVariableIsSet("VISUAL")) {
    const QString visual = qEnvironmentVariable("VISUAL").trimmed();
    if (resolveEditorCommand(visual, prog, args))
      return visual;
  }
  if (qEnvironmentVariableIsSet("EDITOR")) {
    const QString editor = qEnvironmentVariable("EDITOR").trimmed();
    if (resolveEditorCommand(editor, prog, args))
      return editor;
  }
  const QStringList candidates = {
      QStringLiteral("nvim"),  QStringLiteral("hx"),   QStringLiteral("helix"),
      QStringLiteral("vim"),   QStringLiteral("nano"), QStringLiteral("micro"),
      QStringLiteral("emacs"), QStringLiteral("vi")};
  for (const QString &cand : candidates) {
    if (!QStandardPaths::findExecutable(cand).isEmpty()) {
      return cand;
    }
  }
  return QStringLiteral("vi");
}

QString FileManagerBackend::currentItemPath() const {
  if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size())
    return QString();
  return m_filteredEntries.at(m_currentIndex)
      .toMap()
      .value(QStringLiteral("path"))
      .toString();
}

bool FileManagerBackend::isCurrentItemDir() const {
  if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size())
    return false;
  return m_filteredEntries.at(m_currentIndex)
      .toMap()
      .value(QStringLiteral("isDir"))
      .toBool();
}

void FileManagerBackend::refreshCurrentAndPreview() { refreshCurrent(); }

void FileManagerBackend::refreshAll(const QString &selectPath,
                                    bool pathChanged) {
  refreshParent();
  refreshCurrent(selectPath);
  updateDiskUsage();
  updateWatchedDirectory();
  if (pathChanged) {
    emit currentPathChanged();
  }
}

void FileManagerBackend::updateWatchedDirectory() {
  if (!m_fsWatcher)
    return;
  const QStringList watched = m_fsWatcher->directories();
  if (!watched.isEmpty()) {
    m_fsWatcher->removePaths(watched);
  }
  QStringList toWatch;
  if (!m_currentPath.isEmpty() && QFileInfo::exists(m_currentPath))
    toWatch.append(m_currentPath);
  if (!m_parentPath.isEmpty() && QFileInfo::exists(m_parentPath))
    toWatch.append(m_parentPath);
  if (!toWatch.isEmpty()) {
    m_fsWatcher->addPaths(toWatch);
  }
}

QString FileManagerBackend::parentDirectoryPath(const QString &path) const {
  if (path.isEmpty())
    return QString();

  if (m_useMock) {
    const QString clean = QDir::cleanPath(path);
    if (clean == QStringLiteral("/") || clean.isEmpty())
      return QString();
    const qsizetype lastSlash = clean.lastIndexOf(QLatin1Char('/'));
    return (lastSlash <= 0) ? QStringLiteral("/") : clean.left(lastSlash);
  }

  QDir dir(path);
  if (dir.isRoot())
    return QString();
  if (!dir.cdUp())
    return QString();
  return QDir::cleanPath(dir.absolutePath());
}

QString FileManagerBackend::joinPath(const QString &dirPath,
                                     const QString &name) const {
  if (dirPath.isEmpty())
    return QDir::cleanPath(name);
  return QDir::cleanPath(QDir(dirPath).filePath(name));
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
    const auto parentIt = m_mockFs.constFind(m_parentPath);
    if (parentIt != m_mockFs.cend()) {
      for (const QString &childName : parentIt->childNames) {
        if (!m_showHidden && childName.startsWith(QLatin1Char('.')))
          continue;
        const QString childPath = joinPath(m_parentPath, childName);
        const auto childIt = m_mockFs.constFind(childPath);
        if (childIt != m_mockFs.cend()) {
          m_parentEntries.append(formatFileEntry(
              childIt->name, childPath, childIt->isDir, childIt->size,
              childIt->modified, childIt->permissions, childIt->owner,
              childIt->mimeType));
        }
      }
    }
  } else {
    using ItFlag = QDirListing::IteratorFlag;
    QDirListing::IteratorFlags flags = ItFlag::Default;
    if (m_showHidden) {
      flags |= ItFlag::IncludeHidden;
    }
    for (const auto &entry : QDirListing(m_parentPath, flags)) {
      const bool isDir = entry.isDir();
      const QFileInfo info = entry.fileInfo();
      const QString mime =
          isDir ? QStringLiteral("inode/directory")
                : m_mimeDb
                      .mimeTypeForFile(entry.filePath(),
                                       QMimeDatabase::MatchExtension)
                      .name();
      const QString perms =
          formatPermissions(info.permissions(), isDir, entry.isSymLink());
      m_parentEntries.append(formatFileEntry(
          entry.fileName(), QDir::cleanPath(entry.filePath()), isDir,
          isDir ? -1 : entry.size(), info.lastModified(), perms, m_userHost,
          mime));
    }
  }

  std::sort(m_parentEntries.begin(), m_parentEntries.end(),
            [this](const QVariant &a, const QVariant &b) {
              const QVariantMap ma = a.toMap();
              const QVariantMap mb = b.toMap();
              const bool isDirA = ma.value(QStringLiteral("isDir")).toBool();
              const bool isDirB = mb.value(QStringLiteral("isDir")).toBool();
              if (isDirA != isDirB)
                return isDirA;
              return m_collator.compare(
                         ma.value(QStringLiteral("name")).toString(),
                         mb.value(QStringLiteral("name")).toString()) < 0;
            });

  emit parentEntriesChanged();
}

void FileManagerBackend::refreshCurrent(const QString &selectPath) {
  m_rawCurrentEntries.clear();

  if (m_useMock) {
    const auto curIt = m_mockFs.constFind(m_currentPath);
    if (curIt != m_mockFs.cend()) {
      for (const QString &childName : curIt->childNames) {
        const QString childPath = joinPath(m_currentPath, childName);
        const auto childIt = m_mockFs.constFind(childPath);
        if (childIt != m_mockFs.cend()) {
          m_rawCurrentEntries.append(formatFileEntry(
              childIt->name, childPath, childIt->isDir, childIt->size,
              childIt->modified, childIt->permissions, childIt->owner,
              childIt->mimeType));
        }
      }
    }
  } else {
    using ItFlag = QDirListing::IteratorFlag;
    const auto flags = ItFlag::Default | ItFlag::IncludeHidden;
    for (const auto &entry : QDirListing(m_currentPath, flags)) {
      const bool isDir = entry.isDir();
      const QFileInfo info = entry.fileInfo();
      const QString mime =
          isDir ? QStringLiteral("inode/directory")
                : m_mimeDb
                      .mimeTypeForFile(entry.filePath(),
                                       QMimeDatabase::MatchExtension)
                      .name();
      const QString perms =
          formatPermissions(info.permissions(), isDir, entry.isSymLink());
      m_rawCurrentEntries.append(formatFileEntry(
          entry.fileName(), QDir::cleanPath(entry.filePath()), isDir,
          isDir ? -1 : entry.size(), info.lastModified(), perms, m_userHost,
          mime));
    }
  }

  applyFilterAndSort(selectPath);
}

void FileManagerBackend::applyFilterAndSort(const QString &selectPath) {
  m_filteredEntries.clear();

  for (const auto &var : std::as_const(m_rawCurrentEntries)) {
    QVariantMap map = var.toMap();
    const QString name = map.value(QStringLiteral("name")).toString();

    if (!m_showHidden && name.startsWith(QLatin1Char('.'))) {
      continue;
    }

    if (!m_filterText.isEmpty() &&
        !name.contains(m_filterText, Qt::CaseInsensitive)) {
      continue;
    }

    map[QStringLiteral("isMarked")] =
        m_markedSet.contains(map.value(QStringLiteral("path")).toString());
    m_filteredEntries.append(map);
  }

  std::sort(m_filteredEntries.begin(), m_filteredEntries.end(),
            [this](const QVariant &a, const QVariant &b) {
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
                const QString ta =
                    ma.value(QStringLiteral("modifiedRaw")).toString();
                const QString tb =
                    mb.value(QStringLiteral("modifiedRaw")).toString();
                cmp = QString::compare(ta, tb, Qt::CaseInsensitive);
                break;
              }
              case SortField::Type: {
                const QString ea =
                    ma.value(QStringLiteral("extension")).toString();
                const QString eb =
                    mb.value(QStringLiteral("extension")).toString();
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
      if (m_filteredEntries.at(i)
              .toMap()
              .value(QStringLiteral("path"))
              .toString() == selectPath) {
        foundIndex = static_cast<int>(i);
        break;
      }
    }
    m_currentIndex =
        (foundIndex >= 0)
            ? foundIndex
            : std::clamp(m_currentIndex, 0,
                         static_cast<int>(m_filteredEntries.size()) - 1);
  } else {
    m_currentIndex = std::clamp(m_currentIndex, 0,
                                static_cast<int>(m_filteredEntries.size()) - 1);
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

bool FileManagerBackend::isTextFile(const QString &filePath,
                                    const QString &fileName,
                                    const QString &mimeName) const {
  const QMimeType mt = m_useMock ? m_mimeDb.mimeTypeForName(mimeName)
                                 : m_mimeDb.mimeTypeForFile(
                                       filePath, QMimeDatabase::MatchDefault);
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
      QStringLiteral("md"),   QStringLiteral("txt"),  QStringLiteral("qml"),
      QStringLiteral("h"),    QStringLiteral("hpp"),  QStringLiteral("cpp"),
      QStringLiteral("c"),    QStringLiteral("rs"),   QStringLiteral("py"),
      QStringLiteral("json"), QStringLiteral("yaml"), QStringLiteral("yml"),
      QStringLiteral("toml"), QStringLiteral("sh"),   QStringLiteral("env"),
      QStringLiteral("cmake"), QStringLiteral("ini"), QStringLiteral("conf"),
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
    owner = formatOwner(info);
    const QMimeType resolvedMime =
        m_mimeDb.mimeTypeForFile(info, QMimeDatabase::MatchDefault);
    if (resolvedMime.isValid())
      mime = resolvedMime.name();
  } else if (!m_useMock && isDir) {
    owner = formatOwner(QFileInfo(itemPath));
  }

  m_previewData[QStringLiteral("name")] = itemName;
  m_previewData[QStringLiteral("path")] = itemPath;
  m_previewData[QStringLiteral("isDir")] = isDir;
  m_previewData[QStringLiteral("sizeFormatted")] =
      currentItem.value(QStringLiteral("sizeFormatted"));
  m_previewData[QStringLiteral("modifiedFormatted")] =
      currentItem.value(QStringLiteral("modified"));
  m_previewData[QStringLiteral("permissions")] =
      currentItem.value(QStringLiteral("permissions"));
  m_previewData[QStringLiteral("owner")] = owner;
  m_previewData[QStringLiteral("mimeType")] = mime;
  m_previewData[QStringLiteral("icon")] =
      currentItem.value(QStringLiteral("icon"));
  m_previewData[QStringLiteral("iconColor")] =
      currentItem.value(QStringLiteral("iconColor"));

  if (isDir) {
    m_previewData[QStringLiteral("type")] = QStringLiteral("directory");
    QVariantList childPreviews;

    if (m_useMock) {
      const auto dirIt = m_mockFs.constFind(itemPath);
      if (dirIt != m_mockFs.cend()) {
        m_previewData[QStringLiteral("itemCount")] = dirIt->childNames.size();
        for (const QString &childName : dirIt->childNames) {
          const QString childPath = joinPath(itemPath, childName);
          const auto subIt = m_mockFs.constFind(childPath);
          if (subIt != m_mockFs.cend()) {
            childPreviews.append(formatFileEntry(
                subIt->name, childPath, subIt->isDir, subIt->size,
                subIt->modified, subIt->permissions, subIt->owner,
                subIt->mimeType));
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
      for (const auto &entry : QDirListing(itemPath, flags)) {
        count++;
        if (childPreviews.size() < 30) {
          const bool childIsDir = entry.isDir();
          const QFileInfo childInfo = entry.fileInfo();
          const QString childMime =
              childIsDir ? QStringLiteral("inode/directory")
                         : m_mimeDb
                               .mimeTypeForFile(entry.filePath(),
                                                QMimeDatabase::MatchExtension)
                               .name();
          const QString perms = formatPermissions(
              childInfo.permissions(), childIsDir, entry.isSymLink());
          childPreviews.append(formatFileEntry(
              entry.fileName(), QDir::cleanPath(entry.filePath()), childIsDir,
              childIsDir ? -1 : entry.size(), childInfo.lastModified(), perms,
              m_userHost, childMime));
        }
      }
      m_previewData[QStringLiteral("itemCount")] = count;
    }
    m_previewData[QStringLiteral("dirChildren")] = childPreviews;
  } else {
    if (isTextFile(itemPath, itemName, mime)) {
      m_previewData[QStringLiteral("type")] = QStringLiteral("text");
      QString rawText;
      if (m_useMock) {
        const auto fileIt = m_mockFs.constFind(itemPath);
        if (fileIt != m_mockFs.cend()) {
          rawText = fileIt->content;
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

      QStringList lines = rawText.split(QLatin1Char('\n'));
      if (lines.endsWith(QString()))
        lines.removeLast();
      m_previewData[QStringLiteral("rawText")] = rawText;
      m_previewData[QStringLiteral("lineCount")] = lines.size();
      m_previewData[QStringLiteral("textLines")] = lines;

      QString formattedText;
      int gutterDigits = 2;
      if (lines.size() >= 100)
        gutterDigits = 3;
      if (lines.size() >= 1000)
        gutterDigits = 4;
      const int gutterWidth = gutterDigits + 3; // e.g. " 01 │ " (5 chars)
      const int contentWidth = (std::max)(10, m_previewWidth - 2 - gutterWidth);
      const QString blankGutter =
          QString(gutterDigits, QLatin1Char(' ')) + QStringLiteral(" │ ");

      for (qsizetype i = 0; i < lines.size(); ++i) {
        const QString lineNumStr = QStringLiteral("%1 │ ").arg(
            i + 1, gutterDigits, 10, QLatin1Char(' '));
        const QStringList wrappedChunks =
            wrapTextLine(lines.at(i), contentWidth);
        for (qsizetype c = 0; c < wrappedChunks.size(); ++c) {
          if (c == 0) {
            formattedText +=
                lineNumStr + wrappedChunks.at(c) + QLatin1Char('\n');
          } else {
            formattedText +=
                blankGutter + wrappedChunks.at(c) + QLatin1Char('\n');
          }
        }
      }
      m_previewData[QStringLiteral("formattedText")] = formattedText;
    } else if (mime.startsWith(QStringLiteral("image/"))) {
      m_previewData[QStringLiteral("type")] = QStringLiteral("image");
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
      const QString freeStr = formatSize(storage.bytesAvailable());
      const QString totalStr = formatSize(storage.bytesTotal());
      m_diskUsage = QStringLiteral("Free: %1 / %2").arg(freeStr, totalStr);
    } else {
      m_diskUsage = QStringLiteral("Storage: Ready");
    }
  }
  emit diskUsageChanged();
}

QVariantMap FileManagerBackend::formatFileEntry(
    const QString &name, const QString &fullPath, bool isDir, qint64 size,
    const QDateTime &modified, const QString &permissions, const QString &owner,
    const QString &mimeType) const {
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

  map[QStringLiteral("isMarked")] = m_markedSet.contains(fullPath);
  return map;
}

QString FileManagerBackend::formatSize(qint64 bytes) {
  return QLocale::c().formattedDataSize(bytes, 1,
                                        QLocale::DataSizeTraditionalFormat);
}

QString FileManagerBackend::formatPermissions(QFileDevice::Permissions perms,
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

QString FileManagerBackend::formatOwner(const QFileInfo &info) {
  const QString owner = info.owner();
  const QString group = info.group();
  if (!owner.isEmpty() && !group.isEmpty())
    return owner + QLatin1Char(':') + group;
  if (!owner.isEmpty())
    return owner;
  return qEnvironmentVariable(
      "USER", qEnvironmentVariable("USERNAME", QStringLiteral("user")));
}

QString FileManagerBackend::fileIconFor(const QString &name, bool isDir) {
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
  if (lower.endsWith(QStringLiteral(".png")) ||
      lower.endsWith(QStringLiteral(".jpg")) ||
      lower.endsWith(QStringLiteral(".jpeg")) ||
      lower.endsWith(QStringLiteral(".svg")))
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

QString FileManagerBackend::iconColorFor(const QString &name, bool isDir) {
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
  if (lower.endsWith(QStringLiteral(".png")) ||
      lower.endsWith(QStringLiteral(".jpg")))
    return QStringLiteral("ansi:brightmagenta");
  return QStringLiteral("default");
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

FileManagerBackend::SortField
FileManagerBackend::sortFieldFromString(const QString &field) {
  const QString lower = field.trimmed().toLower();
  if (lower == QStringLiteral("size"))
    return SortField::Size;
  if (lower == QStringLiteral("mtime"))
    return SortField::ModifiedTime;
  if (lower == QStringLiteral("type"))
    return SortField::Type;
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

void FileManagerBackend::navigateTo(const QString &path) {
  const QString clean = QDir::cleanPath(path);
  if (clean.isEmpty() || m_currentPath == clean)
    return;
  m_currentPath = clean;
  m_currentIndex = 0;
  if (!m_filterText.isEmpty()) {
    m_filterText.clear();
    emit filterTextChanged();
  }
  refreshAll();
}

void FileManagerBackend::navigateUp() {
  if (m_parentPath.isEmpty())
    return;
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
  if (m_currentIndex < 0 || m_currentIndex >= m_filteredEntries.size())
    return;
  const QVariantMap item = m_filteredEntries.at(m_currentIndex).toMap();
  if (item.value(QStringLiteral("isDir")).toBool()) {
    navigateTo(item.value(QStringLiteral("path")).toString());
  } else {
    showToast(QStringLiteral("Opened file: %1")
                  .arg(item.value(QStringLiteral("name")).toString()));
  }
}

void FileManagerBackend::toggleMarkCurrent() { toggleMark(m_currentIndex); }

void FileManagerBackend::toggleMark(int index) {
  if (index < 0 || index >= m_filteredEntries.size())
    return;
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
    for (const auto &item : std::as_const(m_filteredEntries)) {
      const QString path =
          item.toMap().value(QStringLiteral("path")).toString();
      m_markedSet.insert(path);
      m_markedPaths.append(path);
    }
  }
  emit markedPathsChanged();
  syncMarkedStateInFilteredEntries();
}

void FileManagerBackend::clearMarks() {
  if (m_markedPaths.isEmpty())
    return;
  m_markedSet.clear();
  m_markedPaths.clear();
  emit markedPathsChanged();
  syncMarkedStateInFilteredEntries();
}

void FileManagerBackend::copyMarked() {
  m_clipboardPaths.clear();
  if (m_markedPaths.isEmpty() && m_currentIndex >= 0 &&
      m_currentIndex < m_filteredEntries.size()) {
    m_clipboardPaths.append(m_filteredEntries.at(m_currentIndex)
                                .toMap()
                                .value(QStringLiteral("path"))
                                .toString());
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
    m_clipboardPaths.append(m_filteredEntries.at(m_currentIndex)
                                .toMap()
                                .value(QStringLiteral("path"))
                                .toString());
  } else {
    m_clipboardPaths = m_markedPaths;
  }
  m_clipboardOp = ClipboardOp::Cut;
  emit clipboardChanged();
  showToast(QStringLiteral("Cut %1 item(s)").arg(m_clipboardPaths.size()));
}

bool FileManagerBackend::copyMockRecursive(const QString &srcPath,
                                           const QString &destPath) {
  const auto srcIt = m_mockFs.constFind(srcPath);
  if (srcIt == m_mockFs.cend())
    return false;

  MockItem item = *srcIt;
  item.name = QFileInfo(destPath).fileName();
  m_mockFs.insert(destPath, item);

  const QString destParent = parentDirectoryPath(destPath);
  if (!destParent.isEmpty() && m_mockFs.contains(destParent)) {
    QStringList &siblings = m_mockFs[destParent].childNames;
    if (!siblings.contains(item.name)) {
      siblings.append(item.name);
      m_mockFs[destParent].size = siblings.size();
    }
  }

  if (item.isDir) {
    for (const QString &childName : std::as_const(item.childNames)) {
      const QString childSrc = joinPath(srcPath, childName);
      const QString childDest = joinPath(destPath, childName);
      copyMockRecursive(childSrc, childDest);
    }
  }
  return true;
}

void FileManagerBackend::removeMockRecursive(const QString &path) {
  const auto it = m_mockFs.constFind(path);
  if (it == m_mockFs.cend())
    return;

  const QString name = it->name;
  const QString parent = parentDirectoryPath(path);
  if (!parent.isEmpty() && m_mockFs.contains(parent)) {
    m_mockFs[parent].childNames.removeAll(name);
    m_mockFs[parent].size = m_mockFs[parent].childNames.size();
  }

  const QString prefix = path + QLatin1Char('/');
  QStringList keysToRemove;
  for (auto mapIt = m_mockFs.cbegin(); mapIt != m_mockFs.cend(); ++mapIt) {
    if (mapIt.key() == path || mapIt.key().startsWith(prefix)) {
      keysToRemove.append(mapIt.key());
    }
  }
  for (const QString &k : keysToRemove) {
    m_mockFs.remove(k);
  }
}

void FileManagerBackend::renameMockRecursive(const QString &oldPath,
                                             const QString &newPath,
                                             const QString &newName) {
  const auto it = m_mockFs.constFind(oldPath);
  if (it == m_mockFs.cend())
    return;

  const QString oldName = it->name;
  MockItem rootItem = *it;
  rootItem.name = newName;
  m_mockFs.remove(oldPath);
  m_mockFs.insert(newPath, rootItem);

  const QString parent = parentDirectoryPath(oldPath);
  if (!parent.isEmpty() && m_mockFs.contains(parent)) {
    const qsizetype idx = m_mockFs[parent].childNames.indexOf(oldName);
    if (idx >= 0) {
      m_mockFs[parent].childNames[idx] = newName;
    }
  }

  if (rootItem.isDir) {
    const QString oldPrefix = oldPath + QLatin1Char('/');
    const QString newPrefix = newPath + QLatin1Char('/');
    QHash<QString, MockItem> relocated;
    QStringList oldKeys;
    for (auto mapIt = m_mockFs.cbegin(); mapIt != m_mockFs.cend(); ++mapIt) {
      if (mapIt.key().startsWith(oldPrefix)) {
        oldKeys.append(mapIt.key());
        relocated.insert(newPrefix + mapIt.key().mid(oldPrefix.size()),
                         mapIt.value());
      }
    }
    for (const QString &k : oldKeys) {
      m_mockFs.remove(k);
    }
    for (auto relIt = relocated.cbegin(); relIt != relocated.cend(); ++relIt) {
      m_mockFs.insert(relIt.key(), relIt.value());
    }
  }
}

void FileManagerBackend::pasteClipboard() {
  if (m_clipboardPaths.isEmpty() || m_clipboardOp == ClipboardOp::None) {
    showToast(QStringLiteral("Clipboard empty"));
    return;
  }

  int count = 0;
  for (const QString &srcPath : std::as_const(m_clipboardPaths)) {
    const QString name = QFileInfo(srcPath).fileName();
    const QString destPath = joinPath(m_currentPath, name);
    if (srcPath == destPath)
      continue;

    if (m_useMock) {
      if (copyMockRecursive(srcPath, destPath)) {
        if (m_clipboardOp == ClipboardOp::Cut) {
          removeMockRecursive(srcPath);
        }
        count++;
      }
    } else {
      if (m_clipboardOp == ClipboardOp::Cut) {
        if (QFile::rename(srcPath, destPath)) {
          count++;
        } else if (copyPathRecursively(srcPath, destPath)) {
          const QFileInfo srcInfo(srcPath);
          if (srcInfo.isDir())
            QDir(srcPath).removeRecursively();
          else
            QFile::remove(srcPath);
          count++;
        }
      } else {
        if (copyPathRecursively(srcPath, destPath))
          count++;
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

bool FileManagerBackend::createItem(const QString &name, bool isDirectory) {
  const QString trimmed = name.trimmed();
  if (trimmed.isEmpty())
    return false;
  const QString targetPath = joinPath(m_currentPath, trimmed);

  if (m_useMock) {
    if (m_mockFs.contains(targetPath))
      return false;
    MockItem item;
    item.name = trimmed;
    item.isDir = isDirectory;
    item.size = 0;
    item.modified = QDateTime::currentDateTimeUtc();
    item.permissions = isDirectory ? QStringLiteral("drwxr-xr-x")
                                   : QStringLiteral("-rw-r--r--");
    item.owner = QStringLiteral("yosri:users");
    item.mimeType = isDirectory ? QStringLiteral("inode/directory")
                                : QStringLiteral("text/plain");
    m_mockFs.insert(targetPath, item);
    if (m_mockFs.contains(m_currentPath)) {
      m_mockFs[m_currentPath].childNames.append(trimmed);
      m_mockFs[m_currentPath].size = m_mockFs[m_currentPath].childNames.size();
    }
  } else {
    if (isDirectory) {
      QDir dir(m_currentPath);
      if (!dir.mkdir(trimmed))
        return false;
    } else {
      if (QFileInfo::exists(targetPath))
        return false;
      QFile file(targetPath);
      if (!file.open(QIODevice::WriteOnly))
        return false;
      file.close();
    }
  }

  showToast(QStringLiteral("Created %1: %2")
                .arg(isDirectory ? QStringLiteral("dir")
                                 : QStringLiteral("file"),
                     trimmed));
  refreshAll(targetPath, false);
  return true;
}

bool FileManagerBackend::deleteCurrentOrMarked() {
  QStringList toDelete;
  if (!m_markedPaths.isEmpty()) {
    toDelete = m_markedPaths;
  } else if (m_currentIndex >= 0 && m_currentIndex < m_filteredEntries.size()) {
    toDelete.append(m_filteredEntries.at(m_currentIndex)
                        .toMap()
                        .value(QStringLiteral("path"))
                        .toString());
  }

  if (toDelete.isEmpty())
    return false;

  int deleted = 0;
  for (const QString &path : std::as_const(toDelete)) {
    if (m_useMock) {
      if (m_mockFs.contains(path)) {
        removeMockRecursive(path);
        deleted++;
      }
    } else {
      if (QFile::moveToTrash(path)) {
        deleted++;
      } else {
        const QFileInfo info(path);
        if (info.isDir()) {
          QDir dir(path);
          if (dir.removeRecursively())
            deleted++;
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

bool FileManagerBackend::renameCurrent(const QString &newName) {
  const QString trimmed = newName.trimmed();
  if (trimmed.isEmpty() || m_currentIndex < 0 ||
      m_currentIndex >= m_filteredEntries.size())
    return false;
  const QVariantMap currentItem = m_filteredEntries.at(m_currentIndex).toMap();
  const QString oldPath = currentItem.value(QStringLiteral("path")).toString();
  const QString oldName = currentItem.value(QStringLiteral("name")).toString();
  const QString newPath = joinPath(m_currentPath, trimmed);

  if (oldName == trimmed)
    return true;

  if (m_useMock) {
    if (!m_mockFs.contains(oldPath) || m_mockFs.contains(newPath))
      return false;
    renameMockRecursive(oldPath, newPath, trimmed);
  } else {
    if (!QFile::rename(oldPath, newPath))
      return false;
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

void FileManagerBackend::showToast(const QString &msg) {
  m_statusMessage = msg;
  emit statusMessageChanged();
}

} // namespace qmltui::examples
