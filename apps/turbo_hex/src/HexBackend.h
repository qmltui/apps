#pragma once

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace turbohex {

// Singleton backend providing CLI startup file paths, directory browsing with
// binary magic-byte detection, and built-in ELF64 / PNG demo buffers.
class HexBackend : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(HexWorkspace)
  QML_SINGLETON

  Q_PROPERTY(
      QString rootPath READ rootPath WRITE setRootPath NOTIFY rootPathChanged)
  Q_PROPERTY(bool demoMode READ demoMode CONSTANT)
  Q_PROPERTY(QStringList initialFiles READ initialFiles CONSTANT)

public:
  explicit HexBackend(QObject *parent = nullptr);
  ~HexBackend() override = default;

  static HexBackend *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
  static void configureStartup(const QStringList &positionalPaths,
                               bool forceDemo);

  QString rootPath() const { return m_rootPath; }
  void setRootPath(const QString &path);
  bool demoMode() const { return m_demoMode; }
  QStringList initialFiles() const { return m_initialFiles; }

  Q_INVOKABLE QVariantList
  listDirectory(const QString &dirPath = QString(),
                const QString &mask = QStringLiteral("*")) const;
  Q_INVOKABLE QString resolvePath(const QString &baseDir,
                                  const QString &inputPath) const;

  static QByteArray demoFileBytes(const QString &uri);
  static void updateDemoFileBytes(const QString &uri, const QByteArray &bytes);

signals:
  void rootPathChanged();

private:
  static void ensureDemoStoreSeeded();

  static QStringList s_startupPaths;
  static bool s_forceDemo;
  static QHash<QString, QByteArray> s_demoStore;

  QString m_rootPath;
  bool m_demoMode = false;
  QStringList m_initialFiles;
};

} // namespace turbohex
