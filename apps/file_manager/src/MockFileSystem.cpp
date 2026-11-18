#include "MockFileSystem.h"

#include <QDir>
#include <QFileInfo>
#include <utility>

namespace qmltui::examples {

MockFileSystem::MockFileSystem() { reset(); }

const MockItem *MockFileSystem::find(const QString &path) const {
  const auto it = m_items.constFind(path);
  return (it != m_items.cend()) ? &(*it) : nullptr;
}

QString MockFileSystem::parentDirectoryPath(const QString &path) {
  if (path.isEmpty())
    return QString();
  const QString clean = QDir::cleanPath(path);
  if (clean == QStringLiteral("/") || clean.isEmpty())
    return QString();
  const qsizetype lastSlash = clean.lastIndexOf(QLatin1Char('/'));
  return (lastSlash <= 0) ? QStringLiteral("/") : clean.left(lastSlash);
}

QString MockFileSystem::joinPath(const QString &dirPath, const QString &name) {
  if (dirPath.isEmpty())
    return QDir::cleanPath(name);
  return QDir::cleanPath(QDir(dirPath).filePath(name));
}

void MockFileSystem::reset() {
  m_items.clear();

  const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

  auto addDir = [this,
                 &nowUtc](const QString &path, const QString &name,
                          const QStringList &children,
                          const QString &perms = QStringLiteral("drwxr-xr-x")) {
    MockItem item;
    item.name = name;
    item.isDir = true;
    item.size = children.size();
    item.modified = nowUtc.addDays(-2);
    item.permissions = perms;
    item.owner = QStringLiteral("dev:users");
    item.mimeType = QStringLiteral("inode/directory");
    item.childNames = children;
    m_items.insert(path, item);
  };

  auto addFile = [this, &nowUtc](const QString &path, const QString &name,
                                 const QString &content, const QString &mime,
                                 qint64 size = 0,
                                 const QString &perms =
                                     QStringLiteral("-rw-r--r--")) {
    MockItem item;
    item.name = name;
    item.isDir = false;
    item.content = content;
    item.size = size > 0 ? size : content.toUtf8().size();
    item.modified = nowUtc.addSecs(-3600 * (content.size() % 24 + 1));
    item.permissions = perms;
    item.owner = QStringLiteral("dev:users");
    item.mimeType = mime;
    m_items.insert(path, item);
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
          QStringLiteral("icon.svg"), QStringLiteral("spec.pdf"),
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

  addFile(QStringLiteral("/workspace/CMakeLists.txt"),
          QStringLiteral("CMakeLists.txt"),
          QStringLiteral("cmake_minimum_required(VERSION 3.21)\n"
                         "project(FileManager VERSION 1.0.0 LANGUAGES CXX)\n\n"
                         "set(CMAKE_CXX_STANDARD 17)\n"
                         "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
                         "find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml)\n"
                         "find_package(QmlTui 0.6 REQUIRED)\n\n"
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
              "1. **Core Engine**: C++17 backend with reactive properties.\n"
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
  addFile(QStringLiteral("/workspace/assets/icon.svg"),
          QStringLiteral("icon.svg"), QStringLiteral(""),
          QStringLiteral("image/svg+xml"), 6420);
  addFile(QStringLiteral("/workspace/assets/spec.pdf"),
          QStringLiteral("spec.pdf"), QStringLiteral(""),
          QStringLiteral("application/pdf"), 348160);
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

bool MockFileSystem::createItem(const QString &dirPath, const QString &name,
                                bool isDirectory) {
  const QString targetPath = joinPath(dirPath, name);
  if (m_items.contains(targetPath))
    return false;

  MockItem item;
  item.name = name;
  item.isDir = isDirectory;
  item.size = 0;
  item.modified = QDateTime::currentDateTimeUtc();
  item.permissions = isDirectory ? QStringLiteral("drwxr-xr-x")
                                 : QStringLiteral("-rw-r--r--");
  item.owner = QStringLiteral("yosri:users");
  item.mimeType = isDirectory ? QStringLiteral("inode/directory")
                              : QStringLiteral("text/plain");
  m_items.insert(targetPath, item);
  if (m_items.contains(dirPath)) {
    m_items[dirPath].childNames.append(name);
    m_items[dirPath].size = m_items[dirPath].childNames.size();
  }
  return true;
}

bool MockFileSystem::copyRecursive(const QString &srcPath,
                                   const QString &destPath) {
  const auto srcIt = m_items.constFind(srcPath);
  if (srcIt == m_items.cend())
    return false;

  MockItem item = *srcIt;
  item.name = QFileInfo(destPath).fileName();
  m_items.insert(destPath, item);

  const QString destParent = parentDirectoryPath(destPath);
  if (!destParent.isEmpty() && m_items.contains(destParent)) {
    QStringList &siblings = m_items[destParent].childNames;
    if (!siblings.contains(item.name)) {
      siblings.append(item.name);
      m_items[destParent].size = siblings.size();
    }
  }

  if (item.isDir) {
    for (const QString &childName : std::as_const(item.childNames)) {
      const QString childSrc = joinPath(srcPath, childName);
      const QString childDest = joinPath(destPath, childName);
      copyRecursive(childSrc, childDest);
    }
  }
  return true;
}

bool MockFileSystem::removeRecursive(const QString &path) {
  const auto it = m_items.constFind(path);
  if (it == m_items.cend())
    return false;

  const QString name = it->name;
  const QString parent = parentDirectoryPath(path);
  if (!parent.isEmpty() && m_items.contains(parent)) {
    m_items[parent].childNames.removeAll(name);
    m_items[parent].size = m_items[parent].childNames.size();
  }

  const QString prefix = path + QLatin1Char('/');
  QStringList keysToRemove;
  for (auto mapIt = m_items.cbegin(); mapIt != m_items.cend(); ++mapIt) {
    if (mapIt.key() == path || mapIt.key().startsWith(prefix)) {
      keysToRemove.append(mapIt.key());
    }
  }
  for (const QString &k : keysToRemove) {
    m_items.remove(k);
  }
  return true;
}

bool MockFileSystem::renameRecursive(const QString &oldPath,
                                     const QString &newPath,
                                     const QString &newName) {
  const auto it = m_items.constFind(oldPath);
  if (it == m_items.cend() || m_items.contains(newPath))
    return false;

  const QString oldName = it->name;
  MockItem rootItem = *it;
  rootItem.name = newName;
  m_items.remove(oldPath);
  m_items.insert(newPath, rootItem);

  const QString parent = parentDirectoryPath(oldPath);
  if (!parent.isEmpty() && m_items.contains(parent)) {
    const qsizetype idx = m_items[parent].childNames.indexOf(oldName);
    if (idx >= 0) {
      m_items[parent].childNames[idx] = newName;
    }
  }

  if (rootItem.isDir) {
    const QString oldPrefix = oldPath + QLatin1Char('/');
    const QString newPrefix = newPath + QLatin1Char('/');
    QHash<QString, MockItem> relocated;
    QStringList oldKeys;
    for (auto mapIt = m_items.cbegin(); mapIt != m_items.cend(); ++mapIt) {
      if (mapIt.key().startsWith(oldPrefix)) {
        oldKeys.append(mapIt.key());
        relocated.insert(newPrefix + mapIt.key().mid(oldPrefix.size()),
                         mapIt.value());
      }
    }
    for (const QString &k : oldKeys) {
      m_items.remove(k);
    }
    for (auto relIt = relocated.cbegin(); relIt != relocated.cend(); ++relIt) {
      m_items.insert(relIt.key(), relIt.value());
    }
  }
  return true;
}

} // namespace qmltui::examples
