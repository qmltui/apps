#include "PreviewSyntaxHighlighter.h"

#include <QList>
#include <algorithm>
#include <vector>

namespace qmltui::examples {

namespace {

enum class PreviewTokenStyle : uint8_t {
  Normal = 0,
  Comment,       // \033[3;90m (italic ansi:brightblack / Theme.textMuted)
  Heading,       // \033[1;94m (bold ansi:brightblue / Theme.accentBlue)
  String,        // \033[92m   (ansi:brightgreen / Theme.accentGreen)
  NumberLiteral, // \033[93m   (ansi:brightyellow / Theme.accentYellow)
  KeyCode,       // \033[96m   (ansi:brightcyan / Theme.accentCyan)
};

QLatin1StringView sgrForTokenStyle(PreviewTokenStyle style) {
  switch (style) {
  case PreviewTokenStyle::Comment:
    return QLatin1StringView("\033[3;90m");
  case PreviewTokenStyle::Heading:
    return QLatin1StringView("\033[1;94m");
  case PreviewTokenStyle::String:
    return QLatin1StringView("\033[92m");
  case PreviewTokenStyle::NumberLiteral:
    return QLatin1StringView("\033[93m");
  case PreviewTokenStyle::KeyCode:
    return QLatin1StringView("\033[96m");
  case PreviewTokenStyle::Normal:
    break;
  }
  return QLatin1StringView("");
}

bool isPreprocessorDirective(QStringView word) {
  return word == u"include" || word == u"pragma" || word == u"define" ||
         word == u"ifdef" || word == u"ifndef" || word == u"endif" ||
         word == u"if" || word == u"elif" || word == u"else" ||
         word == u"undef" || word == u"error";
}

bool isCommonLiteral(QStringView word) {
  return word == u"true" || word == u"false" || word == u"null" ||
         word == u"nullptr" || word == u"None" || word == u"True" ||
         word == u"False" || word == u"ON" || word == u"OFF";
}

std::vector<PreviewTokenStyle> tokenizePreviewLine(const QString &line,
                                                   bool isMarkdown) {
  const qsizetype len = line.size();
  std::vector<PreviewTokenStyle> styles(static_cast<size_t>(len),
                                        PreviewTokenStyle::Normal);
  if (len == 0)
    return styles;

  auto fillRange = [&](qsizetype start, qsizetype end,
                       PreviewTokenStyle style) {
    for (qsizetype k = (std::max)(qsizetype(0), start);
         k < (std::min)(len, end); ++k) {
      styles[static_cast<size_t>(k)] = style;
    }
  };

  qsizetype firstNonSpace = 0;
  while (firstNonSpace < len && line[firstNonSpace].isSpace())
    ++firstNonSpace;
  if (firstNonSpace >= len)
    return styles;

  // 1. Markdown heading (# Heading .. ###### Heading)
  if (isMarkdown && line[firstNonSpace] == QLatin1Char('#')) {
    qsizetype hashEnd = firstNonSpace;
    while (hashEnd < len && line[hashEnd] == QLatin1Char('#'))
      ++hashEnd;
    const qsizetype hashCount = hashEnd - firstNonSpace;
    if (hashCount >= 1 && hashCount <= 6 &&
        (hashEnd == len || line[hashEnd].isSpace())) {
      fillRange(firstNonSpace, len, PreviewTokenStyle::Heading);
      return styles;
    }
  }

  // 2. Markdown fenced code block delimiter (```...)
  if (QStringView{line}.sliced(firstNonSpace).startsWith(u"```")) {
    fillRange(firstNonSpace, len, PreviewTokenStyle::KeyCode);
    return styles;
  }

  // 3. INI / TOML / Git-config [section] header
  if (line[firstNonSpace] == QLatin1Char('[')) {
    qsizetype lastNonSpace = len - 1;
    while (lastNonSpace > firstNonSpace && line[lastNonSpace].isSpace())
      --lastNonSpace;
    if (lastNonSpace > firstNonSpace + 1 &&
        line[lastNonSpace] == QLatin1Char(']') &&
        line.indexOf(QLatin1Char(']'), firstNonSpace + 1) == lastNonSpace) {
      fillRange(firstNonSpace, lastNonSpace + 1, PreviewTokenStyle::Heading);
      return styles;
    }
  }

  qsizetype i = firstNonSpace;

  // 4. C/C++ Preprocessor directive (#include, #pragma, #define, ...)
  if (line[firstNonSpace] == QLatin1Char('#')) {
    qsizetype dirStart = firstNonSpace + 1;
    while (dirStart < len && line[dirStart].isSpace())
      ++dirStart;
    qsizetype dirEnd = dirStart;
    while (dirEnd < len && line[dirEnd].isLetter())
      ++dirEnd;
    const QStringView directive =
        QStringView{line}.sliced(dirStart, dirEnd - dirStart);
    if (isPreprocessorDirective(directive) &&
        (dirEnd == len || !line[dirEnd].isLetterOrNumber())) {
      fillRange(firstNonSpace, dirEnd, PreviewTokenStyle::Heading);
      i = dirEnd;
      if (directive == u"include") {
        const qsizetype openAngle = line.indexOf(QLatin1Char('<'), i);
        const qsizetype closeAngle =
            openAngle >= 0 ? line.indexOf(QLatin1Char('>'), openAngle + 1) : -1;
        if (openAngle >= 0 && closeAngle > openAngle) {
          fillRange(openAngle, closeAngle + 1, PreviewTokenStyle::String);
          i = closeAngle + 1;
        }
      }
    }
  }

  // 5. Unquoted config key at start of line (KEY=value or key: value)
  if (i == firstNonSpace) {
    qsizetype keyStart = firstNonSpace;
    if (line[keyStart] == QLatin1Char('-') && keyStart + 1 < len &&
        line[keyStart + 1].isSpace()) {
      ++keyStart;
      while (keyStart < len && line[keyStart].isSpace())
        ++keyStart;
    }
    if (keyStart < len &&
        (line[keyStart].isLetter() || line[keyStart] == QLatin1Char('_') ||
         line[keyStart] == QLatin1Char('.'))) {
      qsizetype keyEnd = keyStart + 1;
      while (keyEnd < len && (line[keyEnd].isLetterOrNumber() ||
                              line[keyEnd] == QLatin1Char('_') ||
                              line[keyEnd] == QLatin1Char('-') ||
                              line[keyEnd] == QLatin1Char('.'))) {
        ++keyEnd;
      }
      qsizetype sepPos = keyEnd;
      while (sepPos < len && line[sepPos].isSpace())
        ++sepPos;
      if (sepPos < len) {
        const bool isEqualsKey =
            line[sepPos] == QLatin1Char('=') &&
            (sepPos + 1 >= len || line[sepPos + 1] != QLatin1Char('=')) &&
            !QStringView{line}.sliced(sepPos + 1).trimmed().endsWith(u';');
        const bool isColonKey =
            line[sepPos] == QLatin1Char(':') &&
            (sepPos + 1 >= len || line[sepPos + 1].isSpace());
        if (isEqualsKey || isColonKey) {
          fillRange(keyStart, keyEnd, PreviewTokenStyle::KeyCode);
          i = sepPos + 1;
        }
      }
    }
  }

  // 6. Left-to-right token scan
  while (i < len) {
    const QChar ch = line[i];

    // Line comment // (avoid matching :// inside URLs)
    if (ch == QLatin1Char('/') && i + 1 < len &&
        line[i + 1] == QLatin1Char('/') &&
        (i == 0 || line[i - 1] != QLatin1Char(':'))) {
      fillRange(i, len, PreviewTokenStyle::Comment);
      break;
    }

    // Line comment # (at start of line or preceded by whitespace)
    if (ch == QLatin1Char('#') &&
        (i == firstNonSpace || line[i - 1].isSpace())) {
      fillRange(i, len, PreviewTokenStyle::Comment);
      break;
    }

    // Inline backtick code `...`
    if (ch == QLatin1Char('`')) {
      const qsizetype closeTick = line.indexOf(QLatin1Char('`'), i + 1);
      if (closeTick > i) {
        fillRange(i, closeTick + 1, PreviewTokenStyle::KeyCode);
        i = closeTick + 1;
        continue;
      }
    }

    // Quoted strings "..." or '...'
    if (ch == QLatin1Char('"') || ch == QLatin1Char('\'')) {
      // Skip contractions/possessives like don't or user's
      if (ch == QLatin1Char('\'') && i > 0 &&
          (line[i - 1].isLetterOrNumber() ||
           line[i - 1] == QLatin1Char('_'))) {
        ++i;
        continue;
      }
      qsizetype j = i + 1;
      bool closed = false;
      while (j < len) {
        if (line[j] == QLatin1Char('\\') && j + 1 < len) {
          j += 2;
          continue;
        }
        if (line[j] == ch) {
          closed = true;
          break;
        }
        ++j;
      }
      if (closed) {
        PreviewTokenStyle quoteStyle = PreviewTokenStyle::String;
        if (ch == QLatin1Char('"')) {
          qsizetype afterQuote = j + 1;
          while (afterQuote < len && line[afterQuote].isSpace())
            ++afterQuote;
          if (afterQuote < len && line[afterQuote] == QLatin1Char(':') &&
              (afterQuote + 1 >= len ||
               line[afterQuote + 1] != QLatin1Char(':'))) {
            quoteStyle = PreviewTokenStyle::KeyCode;
          }
        }
        fillRange(i, j + 1, quoteStyle);
        i = j + 1;
        continue;
      }
    }

    // Numbers (integers, floats, version numbers, hex literals)
    if (ch.isDigit() &&
        (i == 0 || (!line[i - 1].isLetterOrNumber() &&
                    line[i - 1] != QLatin1Char('_')))) {
      qsizetype numEnd = i + 1;
      if (ch == QLatin1Char('0') && numEnd < len &&
          (line[numEnd] == QLatin1Char('x') ||
           line[numEnd] == QLatin1Char('X'))) {
        ++numEnd;
        while (numEnd < len &&
               (line[numEnd].isDigit() ||
                (line[numEnd] >= QLatin1Char('a') &&
                 line[numEnd] <= QLatin1Char('f')) ||
                (line[numEnd] >= QLatin1Char('A') &&
                 line[numEnd] <= QLatin1Char('F')))) {
          ++numEnd;
        }
      } else {
        while (numEnd < len) {
          if (line[numEnd].isDigit()) {
            ++numEnd;
          } else if (line[numEnd] == QLatin1Char('.') && numEnd + 1 < len &&
                     line[numEnd + 1].isDigit()) {
            ++numEnd;
          } else {
            break;
          }
        }
      }
      while (numEnd < len &&
             (line[numEnd] == QLatin1Char('u') ||
              line[numEnd] == QLatin1Char('U') ||
              line[numEnd] == QLatin1Char('l') ||
              line[numEnd] == QLatin1Char('L') ||
              line[numEnd] == QLatin1Char('f') ||
              line[numEnd] == QLatin1Char('%'))) {
        ++numEnd;
      }
      if (numEnd + 1 < len && line[numEnd] == QLatin1Char('p') &&
          line[numEnd + 1] == QLatin1Char('x')) {
        numEnd += 2;
      }
      if (numEnd == len ||
          (!line[numEnd].isLetterOrNumber() &&
           line[numEnd] != QLatin1Char('_'))) {
        fillRange(i, numEnd, PreviewTokenStyle::NumberLiteral);
        i = numEnd;
        continue;
      }
    }

    // Identifiers (check common boolean/null literals)
    if (ch.isLetter() || ch == QLatin1Char('_')) {
      qsizetype wordEnd = i + 1;
      while (wordEnd < len &&
             (line[wordEnd].isLetterOrNumber() ||
              line[wordEnd] == QLatin1Char('_'))) {
        ++wordEnd;
      }
      const QStringView word = QStringView{line}.sliced(i, wordEnd - i);
      if (isCommonLiteral(word)) {
        fillRange(i, wordEnd, PreviewTokenStyle::NumberLiteral);
      }
      i = wordEnd;
      continue;
    }

    ++i;
  }

  return styles;
}

struct WrappedSlice {
  QString text;
  std::vector<PreviewTokenStyle> styles;
};

QList<WrappedSlice> wrapHighlightedLine(const QString &rawLine, int maxW,
                                        bool isMarkdown) {
  QString line = rawLine;
  line.replace(QLatin1Char('\t'), QStringLiteral("    "));
  const std::vector<PreviewTokenStyle> lineStyles =
      tokenizePreviewLine(line, isMarkdown);

  if (line.isEmpty())
    return {{QString(), {}}};
  if (maxW <= 0 || line.size() <= maxW)
    return {{line, lineStyles}};

  QList<WrappedSlice> chunks;
  QString currentText;
  std::vector<PreviewTokenStyle> currentStyles;

  qsizetype pos = 0;
  const qsizetype len = line.size();
  while (pos < len) {
    qsizetype scanStart = pos;
    if (pos == 0) {
      while (scanStart < len && line[scanStart] == QLatin1Char(' ') &&
             scanStart < maxW / 2) {
        ++scanStart;
      }
    }
    qsizetype spacePos = line.indexOf(QLatin1Char(' '), scanStart);
    if (spacePos < 0)
      spacePos = len;
    const qsizetype wordLen = spacePos - pos;
    const QString word = line.mid(pos, wordLen);

    if (wordLen > maxW) {
      if (!currentText.isEmpty()) {
        chunks.append({currentText, currentStyles});
        currentText.clear();
        currentStyles.clear();
      }
      for (qsizetype k = 0; k < wordLen; k += maxW) {
        const qsizetype sliceLen = (std::min)(qsizetype(maxW), wordLen - k);
        std::vector<PreviewTokenStyle> sliceStyles(
            lineStyles.begin() + pos + k,
            lineStyles.begin() + pos + k + sliceLen);
        chunks.append({word.mid(k, sliceLen), std::move(sliceStyles)});
      }
    } else if (currentText.isEmpty()) {
      currentText = word;
      currentStyles.assign(lineStyles.begin() + pos,
                           lineStyles.begin() + pos + wordLen);
    } else if (currentText.size() + 1 + wordLen <= maxW) {
      currentText += QLatin1Char(' ') + word;
      const PreviewTokenStyle spaceStyle =
          pos > 0 ? lineStyles[static_cast<size_t>(pos - 1)]
                  : PreviewTokenStyle::Normal;
      currentStyles.push_back(spaceStyle);
      currentStyles.insert(currentStyles.end(), lineStyles.begin() + pos,
                           lineStyles.begin() + pos + wordLen);
    } else {
      chunks.append({currentText, currentStyles});
      currentText = word;
      currentStyles.assign(lineStyles.begin() + pos,
                           lineStyles.begin() + pos + wordLen);
    }

    if (spacePos >= len)
      break;
    pos = spacePos + 1;
  }

  if (!currentText.isEmpty()) {
    chunks.append({currentText, currentStyles});
  }
  if (chunks.isEmpty()) {
    chunks.append({line, lineStyles});
  }
  return chunks;
}

QString formatAnsiSlice(const WrappedSlice &slice) {
  if (slice.text.isEmpty())
    return QString();

  QString out;
  const qsizetype len = slice.text.size();
  out.reserve(len + 32);

  qsizetype i = 0;
  while (i < len) {
    const PreviewTokenStyle style =
        static_cast<size_t>(i) < slice.styles.size()
            ? slice.styles[static_cast<size_t>(i)]
            : PreviewTokenStyle::Normal;
    qsizetype j = i + 1;
    while (j < len) {
      const PreviewTokenStyle nextStyle =
          static_cast<size_t>(j) < slice.styles.size()
              ? slice.styles[static_cast<size_t>(j)]
              : PreviewTokenStyle::Normal;
      if (nextStyle != style)
        break;
      ++j;
    }
    const QLatin1StringView sgr = sgrForTokenStyle(style);
    if (!sgr.isEmpty()) {
      out += sgr;
      out += QStringView{slice.text}.sliced(i, j - i);
      out += QLatin1StringView("\033[0m");
    } else {
      out += QStringView{slice.text}.sliced(i, j - i);
    }
    i = j;
  }
  return out;
}

} // namespace

FormattedTextPreview PreviewSyntaxHighlighter::formatPreview(
    const QString &rawText, int previewWidth, bool isMarkdown) {
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
  const int gutterWidth = gutterDigits + 3; // e.g. " 01 │ " (5 chars)
  const int contentWidth = (std::max)(10, previewWidth - 2 - gutterWidth);
  const QString blankGutter =
      QString(gutterDigits, QLatin1Char(' ')) + QStringLiteral(" │ ");
  const QString ansiBlankGutter =
      QStringLiteral("\033[90m") + blankGutter + QStringLiteral("\033[0m");

  for (qsizetype i = 0; i < result.lines.size(); ++i) {
    const QString lineNumStr = QStringLiteral("%1 │ ").arg(
        i + 1, gutterDigits, 10, QLatin1Char(' '));
    const QString ansiLineNumStr =
        QStringLiteral("\033[90m") + lineNumStr + QStringLiteral("\033[0m");
    const QList<WrappedSlice> wrappedChunks =
        wrapHighlightedLine(result.lines.at(i), contentWidth, isMarkdown);
    for (qsizetype c = 0; c < wrappedChunks.size(); ++c) {
      const WrappedSlice &slice = wrappedChunks.at(c);
      const QString ansiBody = formatAnsiSlice(slice);
      if (c == 0) {
        result.formattedText += lineNumStr + slice.text + QLatin1Char('\n');
        result.highlightedText +=
            ansiLineNumStr + ansiBody + QLatin1Char('\n');
      } else {
        result.formattedText += blankGutter + slice.text + QLatin1Char('\n');
        result.highlightedText +=
            ansiBlankGutter + ansiBody + QLatin1Char('\n');
      }
    }
  }

  return result;
}

} // namespace qmltui::examples
