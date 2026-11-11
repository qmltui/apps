#include "ShellProcessHelper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

namespace qmltui::examples {

bool ShellProcessHelper::resolveEditorCommand(const QString &rawCommand,
                                              QString &outProgram,
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

QString ShellProcessHelper::detectEditor() {
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

QString ShellProcessHelper::shellQuote(const QString &value) {
  QString escaped = value;
  escaped.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
  return QLatin1Char('\'') + escaped + QLatin1Char('\'');
}

bool ShellProcessHelper::isForegroundBusy(qint64 pid) {
  if (pid <= 0)
    return false;

  // On Linux, field 8 of /proc/<pid>/stat is tpgid (the foreground process
  // group ID of the controlling terminal). When an interactive child (vim,
  // nvim, less, fzf, htop) is running in the foreground, tpgid != pid.
  QFile statFile(QStringLiteral("/proc/%1/stat").arg(pid));
  if (statFile.open(QIODevice::ReadOnly)) {
    const QByteArray data = statFile.readAll();
    const qsizetype rparen = data.lastIndexOf(')');
    if (rparen >= 0 && rparen + 2 < data.size()) {
      const QList<QByteArray> fields =
          data.mid(rparen + 2).trimmed().split(' ');
      // fields[0] = state (3), [1] = ppid (4), [2] = pgrp (5), [3] = session
      // (6), [4] = tty_nr (7), [5] = tpgid (8)
      if (fields.size() >= 6) {
        bool ok = false;
        const qint64 tpgid = fields.at(5).toLongLong(&ok);
        if (ok && tpgid > 0 && tpgid != pid) {
          return true;
        }
      }
    }
  }

  // Fallback: check /proc/<pid>/task/<pid>/children for active child processes
  QFile childrenFile(QStringLiteral("/proc/%1/task/%1/children").arg(pid));
  if (childrenFile.open(QIODevice::ReadOnly)) {
    const QByteArray children = childrenFile.readAll().trimmed();
    if (!children.isEmpty())
      return true;
  }

  return false;
}

QString ShellProcessHelper::resolveWorkingDirectory(qint64 pid,
                                                    const QString &ptyTitle) {
  if (pid > 0) {
    const QString procCwd =
        QFileInfo(QStringLiteral("/proc/%1/cwd").arg(pid)).symLinkTarget();
    if (!procCwd.isEmpty() && QFileInfo(procCwd).isDir()) {
      return QDir::cleanPath(procCwd);
    }
  }
  if (!ptyTitle.isEmpty()) {
    const qsizetype colonIdx = ptyTitle.lastIndexOf(QLatin1Char(':'));
    QString candidate = (colonIdx >= 0) ? ptyTitle.mid(colonIdx + 1).trimmed()
                                        : ptyTitle.trimmed();
    if (candidate.startsWith(QLatin1Char('~'))) {
      candidate = QDir::homePath() + candidate.mid(1);
    }
    if (!candidate.isEmpty() && QFileInfo(candidate).isDir()) {
      return QDir::cleanPath(candidate);
    }
  }
  return QString();
}

} // namespace qmltui::examples
