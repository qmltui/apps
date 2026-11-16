#include "ShellProcessHelper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <algorithm>

#ifdef Q_OS_MACOS
#include <libproc.h>
#include <sys/proc_info.h>
#endif

namespace qmltui::examples {

bool ShellProcessHelper::resolveEditorCommand(const QString& rawCommand, QString& outProgram,
                                              QStringList& outArgs) {
    const QString trimmed = rawCommand.trimmed();
    if (trimmed.isEmpty()) return false;

    const QStringList tokens = QProcess::splitCommand(trimmed);
    if (tokens.isEmpty()) return false;

    const QString candidate = QDir::fromNativeSeparators(tokens.first());
    if (candidate.contains(QLatin1Char('/'))) {
        const QFileInfo info(candidate);
        if (!info.exists() || !info.isExecutable()) return false;
        outProgram = info.absoluteFilePath();
    } else {
        const QString found = QStandardPaths::findExecutable(candidate);
        if (found.isEmpty()) return false;
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
        if (resolveEditorCommand(visual, prog, args)) return visual;
    }
    if (qEnvironmentVariableIsSet("EDITOR")) {
        const QString editor = qEnvironmentVariable("EDITOR").trimmed();
        if (resolveEditorCommand(editor, prog, args)) return editor;
    }
    const QStringList candidates = {QStringLiteral("nvim"),  QStringLiteral("hx"),
                                    QStringLiteral("helix"), QStringLiteral("vim"),
                                    QStringLiteral("nano"),  QStringLiteral("micro"),
                                    QStringLiteral("emacs"), QStringLiteral("vi")};
    for (const QString& cand : candidates) {
        if (!QStandardPaths::findExecutable(cand).isEmpty()) {
            return cand;
        }
    }
    return QStringLiteral("vi");
}

QString ShellProcessHelper::shellQuote(const QString& value) {
    QString escaped = value;
    escaped.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + escaped + QLatin1Char('\'');
}

bool ShellProcessHelper::isForegroundBusy(qint64 pid) {
    if (pid <= 0) return false;

#if defined(Q_OS_LINUX)
    // On Linux, field 8 of /proc/<pid>/stat is tpgid (the foreground process
    // group ID of the controlling terminal). When an interactive child (vim,
    // nvim, less, fzf, htop) is running in the foreground, tpgid != pid.
    QFile statFile(QStringLiteral("/proc/%1/stat").arg(pid));
    if (statFile.open(QIODevice::ReadOnly)) {
        const QByteArray data = statFile.readAll();
        const qsizetype rparen = data.lastIndexOf(')');
        if (rparen >= 0 && rparen + 2 < data.size()) {
            const QList<QByteArray> fields = data.mid(rparen + 2).trimmed().split(' ');
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
        if (!children.isEmpty()) return true;
    }
#elif defined(Q_OS_MACOS)
    struct proc_bsdinfo bsdInfo{};
    const int ret = proc_pidinfo(static_cast<int>(pid), PROC_PIDTBSDINFO, 0, &bsdInfo,
                                 static_cast<int>(sizeof(bsdInfo)));
    if (ret == static_cast<int>(sizeof(bsdInfo))) {
        const qint64 tpgid = static_cast<qint64>(bsdInfo.e_tpgid);
        if (tpgid > 0 && tpgid != pid) {
            return true;
        }
    }

    pid_t childPids[64] = {};
    const int childCount =
        proc_listchildpids(static_cast<pid_t>(pid), childPids, static_cast<int>(sizeof(childPids)));
    for (int i = 0; i < std::min(childCount, 64); ++i) {
        if (childPids[i] > 0) {
            return true;
        }
    }
#endif

    return false;
}

QString ShellProcessHelper::resolveWorkingDirectory(qint64 pid, const QString& ptyTitle) {
    if (pid > 0) {
#if defined(Q_OS_LINUX)
        const QString procCwd = QFileInfo(QStringLiteral("/proc/%1/cwd").arg(pid)).symLinkTarget();
        if (!procCwd.isEmpty() && QFileInfo(procCwd).isDir()) {
            return QDir::cleanPath(procCwd);
        }
#elif defined(Q_OS_MACOS)
        struct proc_vnodepathinfo vnodeInfo{};
        const int ret = proc_pidinfo(static_cast<int>(pid), PROC_PIDVNODEPATHINFO, 0, &vnodeInfo,
                                     static_cast<int>(sizeof(vnodeInfo)));
        if (ret == static_cast<int>(sizeof(vnodeInfo)) && vnodeInfo.pvi_cdir.vip_path[0] != '\0') {
            const QString procCwd = QString::fromUtf8(vnodeInfo.pvi_cdir.vip_path);
            if (!procCwd.isEmpty() && QFileInfo(procCwd).isDir()) {
                return QDir::cleanPath(procCwd);
            }
        }
#endif
    }
    if (!ptyTitle.isEmpty()) {
        const qsizetype colonIdx = ptyTitle.lastIndexOf(QLatin1Char(':'));
        QString candidate =
            (colonIdx >= 0) ? ptyTitle.mid(colonIdx + 1).trimmed() : ptyTitle.trimmed();
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
