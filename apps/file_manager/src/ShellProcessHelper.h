#pragma once

#include <QString>
#include <QStringList>

namespace qmltui::examples {

class ShellProcessHelper {
public:
  static bool resolveEditorCommand(const QString &rawCommand,
                                   QString &outProgram, QStringList &outArgs);
  static QString detectEditor();
  static QString shellQuote(const QString &value);
  static bool isForegroundBusy(qint64 pid);
  static QString resolveWorkingDirectory(qint64 pid, const QString &ptyTitle);
};

} // namespace qmltui::examples
