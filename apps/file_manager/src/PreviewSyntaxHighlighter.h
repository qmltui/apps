#pragma once

#include <QString>
#include <QStringList>

namespace qmltui::examples {

// Laid-out line-numbered text preview payload for PreviewColumn.
struct FormattedTextPreview {
  QString rawText;
  QStringList lines;
  QString formattedText;
  QString highlightedText;
};

// Formats source and config file previews with Unicode-safe wrapping,
// line-number gutters, and ANSI SGR syntax highlighting backed by
// QmlTui.Markdown's SyntaxHighlighter.
class PreviewSyntaxHighlighter {
public:
  static FormattedTextPreview
  formatPreview(const QString &rawText, int previewWidth,
                const QString &fileName = QString(),
                const QString &mimeType = QString());
};

} // namespace qmltui::examples
