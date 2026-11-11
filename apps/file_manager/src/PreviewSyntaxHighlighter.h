#pragma once

#include <QString>
#include <QStringList>

namespace qmltui::examples {

struct FormattedTextPreview {
  QString rawText;
  QStringList lines;
  QString formattedText;
  QString highlightedText;
};

class PreviewSyntaxHighlighter {
public:
  static FormattedTextPreview formatPreview(const QString &rawText,
                                            int previewWidth,
                                            bool isMarkdown);
};

} // namespace qmltui::examples
