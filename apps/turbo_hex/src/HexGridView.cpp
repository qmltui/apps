#include "HexGridView.h"

#include <QmlTui/Core/Engine/KeyEvent.h>
#include <QmlTui/Core/Engine/MouseEvent.h>
#include <QmlTui/Core/Primitives/TextGridBuffer.h>
#include <algorithm>

namespace turbohex {

namespace {
constexpr char kHexDigits[17] = "0123456789ABCDEF";

int hexDigitValue(QChar ch) {
  const ushort u = ch.unicode();
  if (u >= '0' && u <= '9')
    return u - '0';
  if (u >= 'a' && u <= 'f')
    return 10 + (u - 'a');
  if (u >= 'A' && u <= 'F')
    return 10 + (u - 'A');
  return -1;
}
} // namespace

HexGridView::HexGridView(QObject *parent) : qmltui::Item(parent) {
  setItemFlag(ItemFlag::BoundedSelfPaint);
  setFocusPolicy(Qt::StrongFocus);
  connect(this, &Item::widthChanged, this, [this]() {
    ensureCursorVisible();
    emit layoutMetricsChanged();
    markDirty();
  });
  connect(this, &Item::heightChanged, this, [this]() {
    ensureCursorVisible();
    markDirty();
  });
  connect(this, &Item::activeFocusChanged, this, [this]() { markDirty(); });
}

void HexGridView::setDocument(BinaryDocument *doc) {
  if (m_doc == doc)
    return;
  if (m_doc)
    m_doc->disconnect(this);
  m_doc = doc;
  m_topRow = 0;
  m_lowNibblePending = false;
  if (m_doc) {
    connect(m_doc, &BinaryDocument::documentReset, this, [this]() {
      m_topRow = 0;
      m_lowNibblePending = false;
      ensureCursorVisible();
      markDirty();
    });
    connect(m_doc, &BinaryDocument::cursorChanged, this, [this]() {
      m_lowNibblePending = false;
      ensureCursorVisible();
      markDirty();
    });
    connect(m_doc, &BinaryDocument::selectionChanged, this,
            [this]() { markDirty(); });
    connect(m_doc, &BinaryDocument::bytesEdited, this,
            [this](int) { markDirty(); });
    connect(m_doc, &BinaryDocument::modifiedChanged, this,
            [this]() { markDirty(); });
  }
  emit documentChanged();
  markDirty();
}

void HexGridView::setEditMode(bool enabled) {
  if (m_editMode == enabled)
    return;
  m_editMode = enabled;
  m_lowNibblePending = false;
  emit editModeChanged();
  markDirty();
}

void HexGridView::setAsciiActive(bool active) {
  if (m_asciiActive == active)
    return;
  m_asciiActive = active;
  m_lowNibblePending = false;
  emit paneChanged();
  markDirty();
}

int HexGridView::effectiveBytesPerRow() const {
  // 16 bytes/row needs: 8 (offset) + 2 + (16*3 + 1) + 1 + (16 + 2) = 78 cols
  // 8 bytes/row needs:  8 (offset) + 2 + (8*3) + 1 + (8 + 2) = 45 cols
  return (width() >= 78) ? 16 : 8;
}

void HexGridView::setTopRow(int row) {
  const int bpr = effectiveBytesPerRow();
  const int totalRows =
      m_doc ? std::max(1, (m_doc->byteCount() + bpr - 1) / bpr) : 1;
  const int visibleDataRows = std::max(1, height() - 1);
  const int maxTop = std::max(0, totalRows - visibleDataRows);
  const int clamped = std::clamp(row, 0, maxTop);
  if (m_topRow == clamped)
    return;
  m_topRow = clamped;
  emit topRowChanged();
  markDirty();
}

void HexGridView::ensureCursorVisible() {
  if (!m_doc || height() <= 1)
    return;
  const int bpr = effectiveBytesPerRow();
  const int curRow = m_doc->cursorOffset() / bpr;
  const int visibleRows = std::max(1, height() - 1);
  if (curRow < m_topRow) {
    setTopRow(curRow);
  } else if (curRow >= m_topRow + visibleRows) {
    setTopRow(curRow - visibleRows + 1);
  }
}

void HexGridView::setBackgroundColor(qmltui::Color c) {
  if (m_backgroundColor == c)
    return;
  m_backgroundColor = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setHeaderColor(qmltui::Color c) {
  if (m_headerColor == c)
    return;
  m_headerColor = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setOffsetColor(qmltui::Color c) {
  if (m_offsetColor == c)
    return;
  m_offsetColor = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setByteColor(qmltui::Color c) {
  if (m_byteColor == c)
    return;
  m_byteColor = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setModifiedColor(qmltui::Color c) {
  if (m_modifiedColor == c)
    return;
  m_modifiedColor = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setSelectionBg(qmltui::Color c) {
  if (m_selectionBg == c)
    return;
  m_selectionBg = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setSelectionFg(qmltui::Color c) {
  if (m_selectionFg == c)
    return;
  m_selectionFg = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setCursorBg(qmltui::Color c) {
  if (m_cursorBg == c)
    return;
  m_cursorBg = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::setCursorFg(qmltui::Color c) {
  if (m_cursorFg == c)
    return;
  m_cursorFg = c;
  emit colorsChanged();
  markDirty();
}

void HexGridView::moveCursorBy(int deltaBytes) {
  if (!m_doc || m_doc->byteCount() <= 0)
    return;
  m_doc->setCursorOffset(m_doc->cursorOffset() + deltaBytes);
}

int HexGridView::hitTestByteOffset(int localX, int localY,
                                   bool *outAsciiPane) const {
  if (!m_doc || m_doc->byteCount() <= 0 || localY < 1 || localY >= height())
    return -1;
  const int bpr = effectiveBytesPerRow();
  const int row = m_topRow + (localY - 1);
  const int hexStart = 10;
  const int hexWidth = (bpr == 16) ? 49 : 24;
  const int asciiBarCol = hexStart + hexWidth + 1;
  const int asciiStart = asciiBarCol + 1;

  int colInRow = -1;
  bool inAscii = false;
  if (localX >= hexStart && localX < hexStart + hexWidth) {
    int rel = localX - hexStart;
    if (bpr == 16 && rel >= 24)
      rel -= 1; // Middle group gap between byte 7 and byte 8
    colInRow = std::clamp(rel / 3, 0, bpr - 1);
  } else if (localX >= asciiStart && localX < asciiStart + bpr) {
    colInRow = localX - asciiStart;
    inAscii = true;
  } else {
    return -1;
  }

  const int offset = row * bpr + colInRow;
  if (offset < 0 || offset >= m_doc->byteCount())
    return -1;
  if (outAsciiPane)
    *outAsciiPane = inAscii;
  return offset;
}

void HexGridView::mousePressEvent(qmltui::MouseEvent *event) {
  if (event->button() != Qt::LeftButton)
    return;
  forceActiveFocus();
  bool inAscii = false;
  const int off = hitTestByteOffset(event->x(), event->y(), &inAscii);
  if (off >= 0 && m_doc) {
    setAsciiActive(inAscii);
    m_dragAnchorOffset = off;
    m_doc->clearSelection();
    m_doc->setCursorOffset(off);
  }
  event->setAccepted(true);
}

void HexGridView::mouseMoveEvent(qmltui::MouseEvent *event) {
  if (m_dragAnchorOffset < 0 || !m_doc)
    return;
  if (event->isMotion() && event->button() == Qt::NoButton)
    return;
  bool inAscii = false;
  const int off = hitTestByteOffset(event->x(), event->y(), &inAscii);
  if (off >= 0) {
    const int lo = std::min(m_dragAnchorOffset, off);
    const int hi = std::max(m_dragAnchorOffset, off);
    m_doc->selectRange(lo, hi - lo + 1);
    event->setAccepted(true);
  }
}

void HexGridView::wheelEvent(qmltui::MouseEvent *event) {
  const int step = (event->wheelDeltaY() > 0) ? -3 : 3;
  setTopRow(m_topRow + step);
  event->setAccepted(true);
}

void HexGridView::keyPressEvent(qmltui::KeyEvent *event) {
  if (!m_doc || m_doc->byteCount() <= 0)
    return;
  const int bpr = effectiveBytesPerRow();
  const int pageRows = std::max(1, height() - 2);
  const int mods = event->modifiers();

  if ((mods & (Qt::ControlModifier | Qt::AltModifier)) == 0) {
    switch (event->key()) {
    case Qt::Key_Left:
      moveCursorBy(-1);
      event->setAccepted(true);
      return;
    case Qt::Key_Right:
      moveCursorBy(1);
      event->setAccepted(true);
      return;
    case Qt::Key_Up:
      moveCursorBy(-bpr);
      event->setAccepted(true);
      return;
    case Qt::Key_Down:
      moveCursorBy(bpr);
      event->setAccepted(true);
      return;
    case Qt::Key_PageUp:
      moveCursorBy(-bpr * pageRows);
      event->setAccepted(true);
      return;
    case Qt::Key_PageDown:
      moveCursorBy(bpr * pageRows);
      event->setAccepted(true);
      return;
    case Qt::Key_Home:
      m_doc->setCursorOffset(0);
      event->setAccepted(true);
      return;
    case Qt::Key_End:
      m_doc->setCursorOffset(m_doc->byteCount() - 1);
      event->setAccepted(true);
      return;
    case Qt::Key_Tab:
      setAsciiActive(!m_asciiActive);
      event->setAccepted(true);
      return;
    default:
      break;
    }
  }

  // Overwrite editing when editMode is active
  if (m_editMode && (mods & (Qt::ControlModifier | Qt::AltModifier)) == 0 &&
      event->text().size() == 1) {
    const QChar ch = event->text().at(0);
    const int off = m_doc->cursorOffset();
    if (m_asciiActive) {
      if (ch.unicode() >= 0x20 && ch.unicode() <= 0x7E) {
        m_doc->setByteAt(off, static_cast<int>(ch.unicode()));
        moveCursorBy(1);
        event->setAccepted(true);
        return;
      }
    } else {
      const int nib = hexDigitValue(ch);
      if (nib >= 0) {
        const quint8 cur = static_cast<quint8>(m_doc->rawBytes().at(off));
        if (!m_lowNibblePending) {
          const quint8 updated = static_cast<quint8>((nib << 4) | (cur & 0x0F));
          m_doc->setByteAt(off, updated);
          m_lowNibblePending = true;
          markDirty();
        } else {
          const quint8 updated = static_cast<quint8>((cur & 0xF0) | nib);
          m_doc->setByteAt(off, updated);
          m_lowNibblePending = false;
          moveCursorBy(1);
        }
        event->setAccepted(true);
        return;
      }
    }
  }
}

void HexGridView::paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
                        const qmltui::ClipRect &clipRect) const {
  if (!visible())
    return;
  const int w = width();
  const int h = height();
  if (w <= 0 || h <= 0)
    return;

  const uint32_t bg = m_backgroundColor.paintedArgb32();
  const uint32_t hdrFg = m_headerColor.paintedArgb32();
  const uint32_t offFg = m_offsetColor.paintedArgb32();
  const uint32_t byteFg = m_byteColor.paintedArgb32();
  const uint32_t modFg = m_modifiedColor.paintedArgb32();
  const uint32_t selBg = m_selectionBg.paintedArgb32();
  const uint32_t selFg = m_selectionFg.paintedArgb32();
  const uint32_t curBg = m_cursorBg.paintedArgb32();
  const uint32_t curFg = m_cursorFg.paintedArgb32();

  const int bpr = effectiveBytesPerRow();
  const QByteArray emptyBytes;
  const QByteArray &data = m_doc ? m_doc->rawBytes() : emptyBytes;
  const int totalBytes = static_cast<int>(data.size());
  const int curOff = m_doc ? m_doc->cursorOffset() : -1;
  const int selStart = m_doc ? m_doc->selectionStart() : 0;
  const int selEnd = selStart + (m_doc ? m_doc->selectionLength() : 0);

  auto setAsciiCell = [&](int col, int cy, char ch, uint32_t fg,
                          uint32_t cellBg, qmltui::TextAttribute attrs) {
    const int cx = absX + col;
    if (cx < 0 || cx >= buffer.width() || !clipRect.contains(cx, cy))
      return;
    qmltui::Cell &cell = buffer.cellAt(cx, cy);
    char str[2] = {ch, '\0'};
    cell.setCharacter(QByteArrayView(str, 1));
    cell.fgColor = fg;
    cell.bgColor = cellBg;
    cell.attributes = attrs;
    cell.displayWidth = 1;
    cell.continuation = false;
    cell.linkId = 0;
  };

  auto setUtf8Cell = [&](int col, int cy, QByteArrayView glyph, uint32_t fg,
                         uint32_t cellBg, qmltui::TextAttribute attrs) {
    const int cx = absX + col;
    if (cx < 0 || cx >= buffer.width() || !clipRect.contains(cx, cy))
      return;
    qmltui::Cell &cell = buffer.cellAt(cx, cy);
    cell.setCharacter(glyph);
    cell.fgColor = fg;
    cell.bgColor = cellBg;
    cell.attributes = attrs;
    cell.displayWidth = 1;
    cell.continuation = false;
    cell.linkId = 0;
  };

  for (int row = 0; row < h; ++row) {
    const int cy = absY + row;
    if (cy < 0 || cy >= buffer.height() || cy < clipRect.y ||
        cy >= clipRect.y + clipRect.height) {
      continue;
    }

    const int xStart = std::max({0, absX, clipRect.x});
    const int xEnd =
        std::min({buffer.width(), absX + w, clipRect.x + clipRect.width});
    if (xEnd > xStart) {
      buffer.splitWideGlyphSpan(xStart, cy, xEnd - xStart);
      for (int cx = xStart; cx < xEnd; ++cx) {
        qmltui::Cell &c = buffer.cellAt(cx, cy);
        c.setCharacter(" ");
        c.fgColor = byteFg;
        c.bgColor = bg;
        c.attributes = qmltui::TextAttribute::None;
        c.displayWidth = 1;
        c.continuation = false;
        c.linkId = 0;
      }
    }

    const int hexStart = 10;
    const int hexWidth = (bpr == 16) ? 49 : 24;
    const int asciiBarCol = hexStart + hexWidth + 1;

    // Row 0: Column header (`Offset    00 01 02 ...  │ASCII`)
    if (row == 0) {
      const char *offHdr = "Offset  ";
      for (int i = 0; i < 8; ++i)
        setAsciiCell(i, cy, offHdr[i], hdrFg, bg, qmltui::TextAttribute::Bold);
      for (int b = 0; b < bpr; ++b) {
        const int col = hexStart + b * 3 + ((bpr == 16 && b >= 8) ? 1 : 0);
        setAsciiCell(col, cy, kHexDigits[(b >> 4) & 0xF], hdrFg, bg,
                     qmltui::TextAttribute::Bold);
        setAsciiCell(col + 1, cy, kHexDigits[b & 0xF], hdrFg, bg,
                     qmltui::TextAttribute::Bold);
      }
      const char *ascHdr = "ASCII";
      for (int i = 0; i < 5; ++i) {
        setAsciiCell(asciiBarCol + 1 + i, cy, ascHdr[i], hdrFg, bg,
                     qmltui::TextAttribute::Bold);
      }
      continue;
    }

    // Data rows (row >= 1)
    const int dataRow = m_topRow + (row - 1);
    const int rowBaseOff = dataRow * bpr;
    if (rowBaseOff >= totalBytes) {
      if (row == 1 && totalBytes == 0) {
        const char *emptyMsg = "(Empty buffer)";
        for (int i = 0; emptyMsg[i] != '\0'; ++i)
          setAsciiCell(1 + i, cy, emptyMsg[i], offFg, bg,
                       qmltui::TextAttribute::Dim);
      }
      continue;
    }

    // 8-digit hex address column (`00000000`)
    for (int d = 7; d >= 0; --d) {
      const int nib = (rowBaseOff >> ((7 - d) * 4)) & 0xF;
      setAsciiCell(d, cy, kHexDigits[nib], offFg, bg,
                   qmltui::TextAttribute::None);
    }
    setUtf8Cell(asciiBarCol, cy, "│", offFg, bg, qmltui::TextAttribute::Dim);
    setUtf8Cell(asciiBarCol + 1 + bpr, cy, "│", offFg, bg,
                qmltui::TextAttribute::Dim);

    for (int b = 0; b < bpr; ++b) {
      const int byteOff = rowBaseOff + b;
      if (byteOff >= totalBytes)
        break;

      const quint8 val = static_cast<quint8>(data.at(byteOff));
      const bool isCur = (byteOff == curOff);
      const bool isSel = (byteOff >= selStart && byteOff < selEnd);
      const bool isMod = m_doc && m_doc->isOffsetModified(byteOff);
      const bool isZero = (val == 0);

      const int hexCol = hexStart + b * 3 + ((bpr == 16 && b >= 8) ? 1 : 0);
      const int ascCol = asciiBarCol + 1 + b;

      // Hex cell pair
      uint32_t hFg = isMod ? modFg : byteFg;
      uint32_t hBg = bg;
      qmltui::TextAttribute hAttr = isZero
                                        ? qmltui::TextAttribute::Dim
                                        : (isMod ? qmltui::TextAttribute::Bold
                                                 : qmltui::TextAttribute::None);
      if (isCur && !m_asciiActive) {
        hFg = curFg;
        hBg = curBg;
        hAttr = qmltui::TextAttribute::Bold |
                (activeFocus() ? qmltui::TextAttribute::None
                               : qmltui::TextAttribute::Underline);
      } else if (isCur && m_asciiActive) {
        hFg = selFg;
        hBg = selBg;
        hAttr = qmltui::TextAttribute::Bold;
      } else if (isSel) {
        hFg = selFg;
        hBg = selBg;
      }
      setAsciiCell(hexCol, cy, kHexDigits[(val >> 4) & 0xF], hFg, hBg, hAttr);
      setAsciiCell(hexCol + 1, cy, kHexDigits[val & 0xF], hFg, hBg, hAttr);

      // ASCII cell
      const bool printable = (val >= 0x20 && val <= 0x7E);
      const char ascChar = printable ? static_cast<char>(val) : '.';
      uint32_t aFg = isMod ? modFg : (printable ? byteFg : offFg);
      uint32_t aBg = bg;
      qmltui::TextAttribute aAttr =
          printable ? qmltui::TextAttribute::None : qmltui::TextAttribute::Dim;
      if (isCur && m_asciiActive) {
        aFg = curFg;
        aBg = curBg;
        aAttr = qmltui::TextAttribute::Bold;
      } else if (isCur && !m_asciiActive) {
        aFg = selFg;
        aBg = selBg;
        aAttr = qmltui::TextAttribute::Bold;
      } else if (isSel) {
        aFg = selFg;
        aBg = selBg;
      }
      setAsciiCell(ascCol, cy, ascChar, aFg, aBg, aAttr);
    }
  }
}

} // namespace turbohex
