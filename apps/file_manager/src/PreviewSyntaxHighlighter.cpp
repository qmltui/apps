#include "PreviewSyntaxHighlighter.h"

#include <QFileInfo>
#include <QmlTui/Core/Primitives/UnicodeText.h>
#include <SyntaxHighlighter.h>
#include <algorithm>

namespace qmltui::examples {

namespace {

using qmltui::GraphemeIterator;
using qmltui::GraphemeRef;
using qmltui::UnicodeText;
using qmltui::markdown::MdRole;
using qmltui::markdown::StyledSpan;
using qmltui::markdown::SyntaxHighlighter;

SyntaxHighlighter::Language detectLanguageForFile(const QString &fileName,
                                                  const QString &mimeType) {
  const QFileInfo info(fileName);
  const QString base = info.fileName().toLower();
  if (base == QStringLiteral("cmakelists.txt") ||
      base.endsWith(QStringLiteral(".cmake"))) {
    return SyntaxHighlighter::Language::CMake;
  }

  const QString suffix = info.suffix().toLower();
  if (!suffix.isEmpty()) {
    const auto byExt = SyntaxHighlighter::detectLanguage(suffix);
    if (byExt != SyntaxHighlighter::Language::None) {
      return byExt;
    }
  }

  const QString lowerMime = mimeType.toLower();
  if (lowerMime.contains(QStringLiteral("c++")) ||
      lowerMime.contains(QStringLiteral("x-c"))) {
    return SyntaxHighlighter::Language::Cpp;
  }
  if (lowerMime.contains(QStringLiteral("python"))) {
    return SyntaxHighlighter::Language::Python;
  }
  if (lowerMime.contains(QStringLiteral("rust"))) {
    return SyntaxHighlighter::Language::Rust;
  }
  if (lowerMime.contains(QStringLiteral("json"))) {
    return SyntaxHighlighter::Language::Json;
  }
  if (lowerMime.contains(QStringLiteral("yaml"))) {
    return SyntaxHighlighter::Language::Yaml;
  }
  if (lowerMime.contains(QStringLiteral("cmake"))) {
    return SyntaxHighlighter::Language::CMake;
  }
  if (lowerMime.contains(QStringLiteral("javascript")) ||
      lowerMime.contains(QStringLiteral("typescript"))) {
    return SyntaxHighlighter::Language::JavaScript;
  }
  if (lowerMime.contains(QStringLiteral("shellscript"))) {
    return SyntaxHighlighter::Language::Bash;
  }
  if (lowerMime.contains(QStringLiteral("html")) ||
      lowerMime.contains(QStringLiteral("xml"))) {
    return SyntaxHighlighter::Language::Html;
  }
  if (lowerMime.contains(QStringLiteral("markdown"))) {
    return SyntaxHighlighter::Language::Markdown;
  }
  return SyntaxHighlighter::Language::None;
}

QLatin1StringView sgrForRole(MdRole role) {
  switch (role) {
  case MdRole::SyntaxKeyword:
  case MdRole::SyntaxPreprocessor:
    return QLatin1StringView("\033[1;35m"); // bold ansi:magenta
  case MdRole::SyntaxType:
    return QLatin1StringView("\033[96m"); // ansi:brightcyan
  case MdRole::SyntaxFunction:
    return QLatin1StringView("\033[94m"); // ansi:brightblue
  case MdRole::SyntaxString:
    return QLatin1StringView("\033[92m"); // ansi:brightgreen
  case MdRole::SyntaxComment:
    return QLatin1StringView("\033[3;90m"); // italic ansi:brightblack
  case MdRole::SyntaxNumber:
    return QLatin1StringView("\033[93m"); // ansi:brightyellow
  case MdRole::SyntaxSymbol:
    return QLatin1StringView("\033[95m"); // ansi:brightmagenta
  case MdRole::Heading1:
    return QLatin1StringView("\033[1;94m"); // bold ansi:brightblue
  default:
    return QLatin1StringView("");
  }
}

// Lightweight fallback for plain config files (.env, .git/config, .ini) where
// no programming language grammar matched.
QList<StyledSpan> highlightPlainConfigLine(const QString &line) {
  const qsizetype len = line.size();
  qsizetype firstNonSpace = 0;
  while (firstNonSpace < len && line[firstNonSpace].isSpace())
    ++firstNonSpace;
  if (firstNonSpace >= len) {
    return {{line, MdRole::Code}};
  }

  // Comment line (# ...)
  if (line[firstNonSpace] == QLatin1Char('#')) {
    QList<StyledSpan> spans;
    if (firstNonSpace > 0)
      spans.append({line.left(firstNonSpace), MdRole::Code});
    spans.append({line.mid(firstNonSpace), MdRole::SyntaxComment});
    return spans;
  }

  // INI / Git-config [section] header
  if (line[firstNonSpace] == QLatin1Char('[')) {
    qsizetype lastNonSpace = len - 1;
    while (lastNonSpace > firstNonSpace && line[lastNonSpace].isSpace())
      --lastNonSpace;
    if (lastNonSpace > firstNonSpace + 1 &&
        line[lastNonSpace] == QLatin1Char(']')) {
      QList<StyledSpan> spans;
      if (firstNonSpace > 0)
        spans.append({line.left(firstNonSpace), MdRole::Code});
      spans.append({line.mid(firstNonSpace, lastNonSpace - firstNonSpace + 1),
                    MdRole::Heading1});
      if (lastNonSpace + 1 < len)
        spans.append({line.mid(lastNonSpace + 1), MdRole::Code});
      return spans;
    }
  }

  // KEY=value or key: value pair
  const qsizetype eqPos = line.indexOf(QLatin1Char('='), firstNonSpace);
  const qsizetype colonPos = line.indexOf(QLatin1Char(':'), firstNonSpace);
  qsizetype sepPos = -1;
  if (eqPos > firstNonSpace) {
    sepPos = eqPos;
  } else if (colonPos > firstNonSpace &&
             (colonPos + 1 >= len || line[colonPos + 1].isSpace())) {
    sepPos = colonPos;
  }
  if (sepPos > firstNonSpace) {
    const QStringView key = QStringView{line}
                                .sliced(firstNonSpace, sepPos - firstNonSpace)
                                .trimmed();
    bool validKey = !key.isEmpty();
    for (QChar ch : key) {
      if (!ch.isLetterOrNumber() && ch != QLatin1Char('_') &&
          ch != QLatin1Char('-') && ch != QLatin1Char('.')) {
        validKey = false;
        break;
      }
    }
    if (validKey) {
      QList<StyledSpan> spans;
      if (firstNonSpace > 0)
        spans.append({line.left(firstNonSpace), MdRole::Code});
      spans.append({line.mid(firstNonSpace, sepPos - firstNonSpace),
                    MdRole::SyntaxType});
      spans.append({line.mid(sepPos, 1), MdRole::SyntaxSymbol});
      const QString rest = line.mid(sepPos + 1);
      const QString trimmedVal = rest.trimmed();
      MdRole valRole = MdRole::Code;
      if (trimmedVal == QStringLiteral("true") ||
          trimmedVal == QStringLiteral("false") ||
          (!trimmedVal.isEmpty() && trimmedVal[0].isDigit())) {
        valRole = MdRole::SyntaxNumber;
      } else if (trimmedVal.size() >= 2 &&
                 ((trimmedVal.startsWith(QLatin1Char('"')) &&
                   trimmedVal.endsWith(QLatin1Char('"'))) ||
                  (trimmedVal.startsWith(QLatin1Char('\'')) &&
                   trimmedVal.endsWith(QLatin1Char('\''))))) {
        valRole = MdRole::SyntaxString;
      }
      if (!rest.isEmpty())
        spans.append({rest, valRole});
      return spans;
    }
  }

  return {{line, MdRole::Code}};
}

QString takeByDisplayWidth(const QString &text, int maxWidth) {
  if (maxWidth <= 0)
    return {};
  QString out;
  int used = 0;
  GraphemeIterator it(text);
  GraphemeRef cluster;
  while (it.next(cluster)) {
    if (used + cluster.width > maxWidth)
      break;
    out.append(cluster.utf16);
    used += cluster.width;
  }
  return out;
}

struct WrappedRow {
  QString plain;
  QString ansi;
};

QList<WrappedRow> wrapStyledSpans(const QList<StyledSpan> &spans,
                                  int maxWidth) {
  const int limit = std::max(1, maxWidth);
  QList<WrappedRow> rows;
  WrappedRow current;
  int used = 0;

  auto pushRow = [&]() {
    rows.append(current);
    current = WrappedRow{};
    used = 0;
  };

  auto appendFragment = [&](const QString &text, MdRole role) {
    current.plain += text;
    const QLatin1StringView sgr = sgrForRole(role);
    if (!sgr.isEmpty()) {
      current.ansi += sgr;
      current.ansi += text;
      current.ansi += QLatin1StringView("\033[0m");
    } else {
      current.ansi += text;
    }
  };

  for (const StyledSpan &span : spans) {
    QString pending = span.text;
    while (!pending.isEmpty()) {
      const int remaining = limit - used;
      QString piece = takeByDisplayWidth(pending, remaining);
      if (piece.isEmpty()) {
        if (used > 0) {
          pushRow();
          continue;
        }
        GraphemeIterator it(pending);
        GraphemeRef first;
        if (!it.next(first))
          break;
        appendFragment(QString(QChar(0xFFFD)), span.foreground);
        used = 1;
        pending.remove(0, first.utf16Length());
        if (!pending.isEmpty())
          pushRow();
        continue;
      }
      appendFragment(piece, span.foreground);
      used += UnicodeText::displayWidth(piece);
      pending.remove(0, piece.size());
      if (!pending.isEmpty())
        pushRow();
    }
  }

  if (!current.plain.isEmpty() || rows.isEmpty())
    rows.append(current);
  return rows;
}

} // namespace

FormattedTextPreview PreviewSyntaxHighlighter::formatPreview(
    const QString &rawText, int previewWidth, const QString &fileName,
    const QString &mimeType) {
  FormattedTextPreview result;
  result.rawText = rawText;
  result.lines = rawText.split(QLatin1Char('\n'));
  if (result.lines.endsWith(QString()))
    result.lines.removeLast();

  int gutterDigits = 2;
  if (result.lines.size() >= 100)
    gutterDigits = 3;
  if (result.lines.size() >= 1000)
    gutterDigits = 4;
  const int gutterWidth = gutterDigits + 3;
  const int contentWidth = std::max(10, previewWidth - 2 - gutterWidth);
  const QString blankGutter =
      QString(gutterDigits, QLatin1Char(' ')) + QStringLiteral(" │ ");
  const QString ansiBlankGutter =
      QStringLiteral("\033[90m") + blankGutter + QStringLiteral("\033[0m");

  const SyntaxHighlighter::Language lang =
      detectLanguageForFile(fileName, mimeType);

  for (qsizetype i = 0; i < result.lines.size(); ++i) {
    QString line = result.lines.at(i);
    line.replace(QLatin1Char('\t'), QStringLiteral("    "));

    const QList<StyledSpan> spans =
        (lang != SyntaxHighlighter::Language::None)
            ? SyntaxHighlighter::highlightLine(line, lang)
            : highlightPlainConfigLine(line);
    const QList<WrappedRow> wrappedRows = wrapStyledSpans(spans, contentWidth);

    const QString lineNumStr =
        QStringLiteral("%1 │ ").arg(i + 1, gutterDigits, 10, QLatin1Char(' '));
    const QString ansiLineNumStr =
        QStringLiteral("\033[90m") + lineNumStr + QStringLiteral("\033[0m");

    for (qsizetype c = 0; c < wrappedRows.size(); ++c) {
      const WrappedRow &row = wrappedRows.at(c);
      if (c == 0) {
        result.formattedText += lineNumStr + row.plain + QLatin1Char('\n');
        result.highlightedText += ansiLineNumStr + row.ansi + QLatin1Char('\n');
      } else {
        result.formattedText += blankGutter + row.plain + QLatin1Char('\n');
        result.highlightedText +=
            ansiBlankGutter + row.ansi + QLatin1Char('\n');
      }
    }
  }

  return result;
}

} // namespace qmltui::examples
