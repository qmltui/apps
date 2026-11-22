#include "BinaryDocument.h"

#include "HexBackend.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QtEndian>
#include <algorithm>
#include <cstring>

namespace turbohex {

namespace {

QString hexPad(quint64 val, int digits) {
  return QStringLiteral("0x") +
         QString::number(val, 16).toUpper().rightJustified(digits,
                                                           QLatin1Char('0'));
}

quint16 readU16(const QByteArray &b, int off, bool le) {
  quint16 raw = 0;
  std::memcpy(&raw, b.constData() + off, 2);
  return le ? qFromLittleEndian(raw) : qFromBigEndian(raw);
}

quint32 readU32(const QByteArray &b, int off, bool le) {
  quint32 raw = 0;
  std::memcpy(&raw, b.constData() + off, 4);
  return le ? qFromLittleEndian(raw) : qFromBigEndian(raw);
}

quint64 readU64(const QByteArray &b, int off, bool le) {
  quint64 raw = 0;
  std::memcpy(&raw, b.constData() + off, 8);
  return le ? qFromLittleEndian(raw) : qFromBigEndian(raw);
}

QVariantMap makeStructItem(const QString &kind, const QString &label,
                           const QString &detail, int offset, int length) {
  QVariantMap m;
  m.insert(QStringLiteral("kind"), kind); // "header" or "string"
  m.insert(QStringLiteral("label"), label);
  m.insert(QStringLiteral("detail"), detail);
  m.insert(QStringLiteral("offset"), offset);
  m.insert(QStringLiteral("length"), std::max(1, length));
  m.insert(QStringLiteral("offsetHex"),
           hexPad(static_cast<quint64>(offset), 4));
  return m;
}

} // namespace

BinaryDocument::BinaryDocument(QObject *parent) : QObject(parent) {
  rebuildInspector();
}

QString BinaryDocument::detectFormatBadge(const QByteArray &bytes) {
  if (bytes.size() >= 5 && bytes.startsWith("\x7F"
                                            "ELF")) {
    return (static_cast<quint8>(bytes.at(4)) == 2) ? QStringLiteral("ELF64")
                                                   : QStringLiteral("ELF32");
  }
  if (bytes.size() >= 8 &&
      bytes.startsWith(QByteArray("\x89PNG\r\n\x1A\n", 8))) {
    return QStringLiteral("PNG");
  }
  if (bytes.size() >= 4 && bytes.startsWith("PK\x03\x04")) {
    return QStringLiteral("ZIP");
  }
  if (bytes.size() >= 4 && bytes.startsWith(QByteArray("\x00"
                                                       "asm",
                                                       4))) {
    return QStringLiteral("WASM");
  }
  if (bytes.size() >= 2 && bytes.startsWith("MZ")) {
    return QStringLiteral("PE/MZ");
  }
  if (bytes.size() >= 4 && bytes.startsWith("%PDF")) {
    return QStringLiteral("PDF");
  }
  return QStringLiteral("BIN");
}

void BinaryDocument::loadBuffer(const QString &path, const QString &dispName,
                                const QByteArray &data) {
  m_filePath = path;
  m_displayName = dispName;
  m_savedBytes = data;
  m_bytes = data;
  m_formatBadge = detectFormatBadge(m_bytes);
  m_cursorOffset = 0;
  m_selectionStart = 0;
  m_selectionLength = 0;
  m_undoStack.clear();
  m_redoStack.clear();
  m_modifiedOffsets.clear();

  // Default endianness from format when obvious (PNG is big-endian; ELF checks
  // EI_DATA)
  if (m_formatBadge == QStringLiteral("PNG")) {
    m_littleEndian = false;
  } else if (m_bytes.size() >= 6 && m_bytes.startsWith("\x7F"
                                                       "ELF")) {
    m_littleEndian = (static_cast<quint8>(m_bytes.at(5)) != 2);
  } else {
    m_littleEndian = true;
  }

  rebuildInspector();
  rebuildStructure();
  emit documentReset();
  emit cursorChanged();
  emit selectionChanged();
  emit endianChanged();
  emit modifiedChanged();
}

bool BinaryDocument::loadFile(const QString &path) {
  if (path.startsWith(QStringLiteral("demo://"))) {
    const QByteArray sample = HexBackend::demoFileBytes(path);
    if (sample.isEmpty())
      return false;
    loadBuffer(path, path.mid(7), sample);
    return true;
  }

  const QString cleaned = QDir::cleanPath(path);
  QFile file(cleaned);
  if (!file.open(QIODevice::ReadOnly))
    return false;
  // Cap in-memory buffer at 16 MiB so large binaries remain responsive
  const QByteArray data = file.read(16 * 1024 * 1024);
  loadBuffer(cleaned, QFileInfo(cleaned).fileName(), data);
  return true;
}

bool BinaryDocument::save() {
  if (m_filePath.isEmpty())
    return false;
  if (m_filePath.startsWith(QStringLiteral("demo://"))) {
    HexBackend::updateDemoFileBytes(m_filePath, m_bytes);
    m_savedBytes = m_bytes;
    m_undoStack.clear();
    m_redoStack.clear();
    m_modifiedOffsets.clear();
    emit modifiedChanged();
    return true;
  }

  QDir().mkpath(QFileInfo(m_filePath).absolutePath());
  QSaveFile out(m_filePath);
  if (!out.open(QIODevice::WriteOnly))
    return false;
  if (out.write(m_bytes) != m_bytes.size() || !out.commit())
    return false;

  m_savedBytes = m_bytes;
  m_formatBadge = detectFormatBadge(m_bytes);
  m_undoStack.clear();
  m_redoStack.clear();
  m_modifiedOffsets.clear();
  rebuildStructure();
  emit documentReset();
  emit modifiedChanged();
  return true;
}

bool BinaryDocument::saveAs(const QString &targetPath) {
  if (targetPath.trimmed().isEmpty())
    return false;
  if (targetPath.startsWith(QStringLiteral("demo://"))) {
    m_filePath = targetPath;
    m_displayName = targetPath.mid(7);
    return save();
  }
  const QString cleaned = QFileInfo(targetPath).isAbsolute()
                              ? QDir::cleanPath(targetPath)
                              : QFileInfo(targetPath).absoluteFilePath();
  m_filePath = cleaned;
  m_displayName = QFileInfo(cleaned).fileName();
  return save();
}

void BinaryDocument::setCursorOffset(int offset) {
  const int maxOff = std::max(0, static_cast<int>(m_bytes.size()) - 1);
  const int clamped = std::clamp(offset, 0, maxOff);
  if (m_cursorOffset == clamped)
    return;
  m_cursorOffset = clamped;
  rebuildInspector();
  emit cursorChanged();
}

void BinaryDocument::setLittleEndian(bool le) {
  if (m_littleEndian == le)
    return;
  m_littleEndian = le;
  rebuildInspector();
  emit endianChanged();
}

QString BinaryDocument::cursorHex() const {
  return hexPad(static_cast<quint64>(m_cursorOffset), 6);
}

void BinaryDocument::selectRange(int start, int length) {
  if (m_bytes.isEmpty() || length <= 0) {
    clearSelection();
    return;
  }
  const int clampedStart =
      std::clamp(start, 0, static_cast<int>(m_bytes.size()) - 1);
  const int clampedLen =
      std::min(length, static_cast<int>(m_bytes.size()) - clampedStart);
  const bool selDiff =
      (m_selectionStart != clampedStart || m_selectionLength != clampedLen);
  m_selectionStart = clampedStart;
  m_selectionLength = clampedLen;
  setCursorOffset(clampedStart);
  if (selDiff)
    emit selectionChanged();
}

void BinaryDocument::clearSelection() {
  if (m_selectionLength == 0)
    return;
  m_selectionLength = 0;
  emit selectionChanged();
}

void BinaryDocument::recomputeModifiedSet() {
  m_modifiedOffsets.clear();
  const int n = std::min(m_bytes.size(), m_savedBytes.size());
  for (int i = 0; i < n; ++i) {
    if (m_bytes.at(i) != m_savedBytes.at(i))
      m_modifiedOffsets.insert(i);
  }
}

bool BinaryDocument::setByteAt(int offset, int byteValue) {
  if (offset < 0 || offset >= m_bytes.size())
    return false;
  const quint8 nextVal = static_cast<quint8>(byteValue & 0xFF);
  const quint8 prevVal = static_cast<quint8>(m_bytes.at(offset));
  if (prevVal == nextVal)
    return true;

  m_bytes[offset] = static_cast<char>(nextVal);
  m_undoStack.append({offset, prevVal, nextVal});
  m_redoStack.clear();
  recomputeModifiedSet();
  m_formatBadge = detectFormatBadge(m_bytes);

  rebuildInspector();
  rebuildStructure();
  emit bytesEdited(offset);
  emit modifiedChanged();
  return true;
}

void BinaryDocument::undo() {
  if (m_undoStack.isEmpty())
    return;
  const EditCmd cmd = m_undoStack.takeLast();
  m_bytes[cmd.offset] = static_cast<char>(cmd.oldByte);
  m_redoStack.append(cmd);
  recomputeModifiedSet();
  setCursorOffset(cmd.offset);
  rebuildInspector();
  rebuildStructure();
  emit bytesEdited(cmd.offset);
  emit modifiedChanged();
}

void BinaryDocument::redo() {
  if (m_redoStack.isEmpty())
    return;
  const EditCmd cmd = m_redoStack.takeLast();
  m_bytes[cmd.offset] = static_cast<char>(cmd.newByte);
  m_undoStack.append(cmd);
  recomputeModifiedSet();
  setCursorOffset(cmd.offset);
  rebuildInspector();
  rebuildStructure();
  emit bytesEdited(cmd.offset);
  emit modifiedChanged();
}

int BinaryDocument::parseOffsetString(const QString &text) {
  QString s = text.trimmed();
  if (s.isEmpty())
    return -1;
  bool ok = false;
  int val = -1;
  if (s.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
    val = s.mid(2).toInt(&ok, 16);
  } else if (s.endsWith(QLatin1Char('h'), Qt::CaseInsensitive)) {
    val = s.left(s.size() - 1).toInt(&ok, 16);
  } else {
    val = s.toInt(&ok, 10);
    if (!ok)
      val = s.toInt(&ok, 16);
  }
  return (ok && val >= 0) ? val : -1;
}

int BinaryDocument::findPattern(const QString &query, bool hexMode,
                                bool fromCursor) {
  if (m_bytes.isEmpty() || query.trimmed().isEmpty())
    return -1;

  QByteArray needle;
  if (hexMode) {
    QString hexClean;
    for (const QChar c : query) {
      if ((c >= QLatin1Char('0') && c <= QLatin1Char('9')) ||
          (c >= QLatin1Char('a') && c <= QLatin1Char('f')) ||
          (c >= QLatin1Char('A') && c <= QLatin1Char('F'))) {
        hexClean.append(c);
      }
    }
    if (hexClean.isEmpty() || (hexClean.size() % 2) != 0)
      return -1;
    needle = QByteArray::fromHex(hexClean.toLatin1());
  } else {
    needle = query.toUtf8();
  }

  if (needle.isEmpty())
    return -1;

  const int startIdx = fromCursor ? std::min(static_cast<int>(m_bytes.size()),
                                             m_cursorOffset + 1)
                                  : 0;
  int hit = static_cast<int>(m_bytes.indexOf(needle, startIdx));
  if (hit < 0 && startIdx > 0) {
    hit = static_cast<int>(m_bytes.indexOf(needle, 0));
  }
  if (hit >= 0) {
    selectRange(hit, static_cast<int>(needle.size()));
  }
  return hit;
}

void BinaryDocument::rebuildInspector() {
  QVariantList rows;
  auto addRow = [&](const QString &key, const QString &val) {
    QVariantMap r;
    r.insert(QStringLiteral("label"), key);
    r.insert(QStringLiteral("value"), val);
    rows.append(r);
  };

  if (m_bytes.isEmpty() || m_cursorOffset >= m_bytes.size()) {
    addRow(QStringLiteral("Offset"), QStringLiteral("(empty)"));
    m_inspectorRows = rows;
    emit inspectorChanged();
    return;
  }

  const int off = m_cursorOffset;
  const int rem = static_cast<int>(m_bytes.size()) - off;
  const quint8 u8 = static_cast<quint8>(m_bytes.at(off));
  const qint8 i8 = static_cast<qint8>(u8);

  addRow(QStringLiteral("Offset"),
         QStringLiteral("%1 (%2)").arg(hexPad(off, 6)).arg(off));
  addRow(QStringLiteral("Hex / Bits"),
         QStringLiteral("0x%1  %2")
             .arg(QString::number(u8, 16).toUpper().rightJustified(
                      2, QLatin1Char('0')),
                  QString::number(u8, 2).rightJustified(8, QLatin1Char('0'))));
  addRow(QStringLiteral("UInt8 / Int8"),
         QStringLiteral("%1 / %2").arg(u8).arg(i8));

  if (rem >= 2) {
    const quint16 u16 = readU16(m_bytes, off, m_littleEndian);
    const qint16 i16 = static_cast<qint16>(u16);
    addRow(QStringLiteral("UInt16 / Int16"),
           QStringLiteral("%1 / %2").arg(u16).arg(i16));
  } else {
    addRow(QStringLiteral("UInt16 / Int16"), QStringLiteral("—"));
  }

  if (rem >= 4) {
    const quint32 u32 = readU32(m_bytes, off, m_littleEndian);
    const qint32 i32 = static_cast<qint32>(u32);
    float f32 = 0.0f;
    std::memcpy(&f32, &u32, 4);
    addRow(QStringLiteral("UInt32 / Int32"),
           QStringLiteral("%1 / %2").arg(u32).arg(i32));
    addRow(QStringLiteral("Hex32"), hexPad(u32, 8));
    addRow(QStringLiteral("Float32"), QString::number(f32, 'g', 6));
  } else {
    addRow(QStringLiteral("UInt32 / Int32"), QStringLiteral("—"));
    addRow(QStringLiteral("Hex32"), QStringLiteral("—"));
    addRow(QStringLiteral("Float32"), QStringLiteral("—"));
  }

  if (rem >= 8) {
    const quint64 u64 = readU64(m_bytes, off, m_littleEndian);
    addRow(QStringLiteral("UInt64"), hexPad(u64, 16));
  } else {
    addRow(QStringLiteral("UInt64"), QStringLiteral("—"));
  }

  QString asciiPreview;
  const int previewLen = std::min(rem, 12);
  for (int i = 0; i < previewLen; ++i) {
    const quint8 ch = static_cast<quint8>(m_bytes.at(off + i));
    asciiPreview.append((ch >= 0x20 && ch <= 0x7E) ? QLatin1Char(ch)
                                                   : QLatin1Char('.'));
  }
  addRow(QStringLiteral("ASCII"),
         QStringLiteral("\"") + asciiPreview + QStringLiteral("\""));

  m_inspectorRows = rows;
  emit inspectorChanged();
}

void BinaryDocument::rebuildStructure() {
  QVariantList items;
  const int sz = static_cast<int>(m_bytes.size());

  // 1. Parse known binary headers
  if (sz >= 16 && m_bytes.startsWith("\x7F"
                                     "ELF")) {
    const bool is64 = (static_cast<quint8>(m_bytes.at(4)) == 2);
    const bool le = (static_cast<quint8>(m_bytes.at(5)) != 2);
    items.append(makeStructItem(QStringLiteral("header"),
                                QStringLiteral("ELF Magic"),
                                QStringLiteral("7F 'E' 'L' 'F'"), 0, 4));
    items.append(makeStructItem(
        QStringLiteral("header"), QStringLiteral("EI_CLASS / DATA"),
        QStringLiteral("%1 · %2").arg(is64 ? QStringLiteral("ELF64")
                                           : QStringLiteral("ELF32"),
                                      le ? QStringLiteral("Little-Endian")
                                         : QStringLiteral("Big-Endian")),
        4, 2));
    if (sz >= 20) {
      const quint16 eType = readU16(m_bytes, 16, le);
      const quint16 eMach = readU16(m_bytes, 18, le);
      const QString machName =
          (eMach == 0x3E)
              ? QStringLiteral("x86-64")
              : ((eMach == 0xB7) ? QStringLiteral("AArch64")
                                 : ((eMach == 0xF3) ? QStringLiteral("RISC-V")
                                                    : hexPad(eMach, 4)));
      items.append(makeStructItem(
          QStringLiteral("header"), QStringLiteral("e_type / e_machine"),
          QStringLiteral("type=%1 · %2").arg(eType).arg(machName), 16, 4));
    }
    if (is64 && sz >= 64) {
      const quint64 entry = readU64(m_bytes, 24, le);
      const quint64 phoff = readU64(m_bytes, 32, le);
      const quint64 shoff = readU64(m_bytes, 40, le);
      const quint16 phnum = readU16(m_bytes, 56, le);
      items.append(makeStructItem(QStringLiteral("header"),
                                  QStringLiteral("e_entry"), hexPad(entry, 8),
                                  24, 8));
      items.append(makeStructItem(
          QStringLiteral("header"), QStringLiteral("e_phoff"),
          QStringLiteral("%1 (%2 hdrs)").arg(hexPad(phoff, 4)).arg(phnum), 32,
          8));
      items.append(makeStructItem(QStringLiteral("header"),
                                  QStringLiteral("e_shoff"), hexPad(shoff, 4),
                                  40, 8));
      if (phnum > 0 && phoff + 56 <= static_cast<quint64>(sz)) {
        items.append(makeStructItem(
            QStringLiteral("header"), QStringLiteral("Program Header #0"),
            QStringLiteral("PT_LOAD segment"), static_cast<int>(phoff), 56));
      }
    }
  } else if (sz >= 8 &&
             m_bytes.startsWith(QByteArray("\x89PNG\r\n\x1A\n", 8))) {
    items.append(makeStructItem(
        QStringLiteral("header"), QStringLiteral("PNG Signature"),
        QStringLiteral("89 'P' 'N' 'G' CR LF 1A LF"), 0, 8));
    int pos = 8;
    while (pos + 12 <= sz && items.size() < 24) {
      const quint32 chunkLen = readU32(m_bytes, pos, false);
      const QString chunkType = QString::fromLatin1(m_bytes.mid(pos + 4, 4));
      const int totalChunk =
          static_cast<int>(std::min<quint64>(sz - pos, 12ULL + chunkLen));
      QString detail = QStringLiteral("%1 B data").arg(chunkLen);
      if (chunkType == QStringLiteral("IHDR") && pos + 24 <= sz) {
        const quint32 w = readU32(m_bytes, pos + 8, false);
        const quint32 h = readU32(m_bytes, pos + 12, false);
        detail = QStringLiteral("%1x%2 px").arg(w).arg(h);
      }
      items.append(makeStructItem(QStringLiteral("header"),
                                  QStringLiteral("Chunk ") + chunkType, detail,
                                  pos, totalChunk));
      if (totalChunk <= 0 || chunkType == QStringLiteral("IEND"))
        break;
      pos += totalChunk;
    }
  } else if (sz >= 4 && m_bytes.startsWith("PK\x03\x04")) {
    items.append(makeStructItem(
        QStringLiteral("header"), QStringLiteral("ZIP Local Header"),
        QStringLiteral("PK 03 04"), 0, std::min(sz, 30)));
  } else if (sz >= 8 && m_bytes.startsWith(QByteArray("\x00"
                                                      "asm",
                                                      4))) {
    const quint32 ver = readU32(m_bytes, 4, true);
    items.append(makeStructItem(QStringLiteral("header"),
                                QStringLiteral("WASM Magic"),
                                QStringLiteral("\\0asm"), 0, 4));
    items.append(makeStructItem(QStringLiteral("header"),
                                QStringLiteral("WASM Version"),
                                QStringLiteral("v%1").arg(ver), 4, 4));
  } else if (sz >= 2 && m_bytes.startsWith("MZ")) {
    items.append(makeStructItem(
        QStringLiteral("header"), QStringLiteral("DOS MZ Header"),
        QStringLiteral("'M' 'Z' Executable"), 0, std::min(sz, 64)));
  } else if (sz > 0) {
    items.append(makeStructItem(
        QStringLiteral("header"), QStringLiteral("Lead Bytes"),
        QStringLiteral("Raw binary header"), 0, std::min(sz, 16)));
  }

  // 2. Scan printable ASCII strings (>= 4 chars)
  const int scanCap = std::min(sz, 65536);
  int runStart = -1;
  for (int i = 0; i <= scanCap; ++i) {
    const bool printable =
        (i < scanCap) && (static_cast<quint8>(m_bytes.at(i)) >= 0x20 &&
                          static_cast<quint8>(m_bytes.at(i)) <= 0x7E);
    if (printable) {
      if (runStart < 0)
        runStart = i;
    } else if (runStart >= 0) {
      const int runLen = i - runStart;
      if (runLen >= 4) {
        QString str =
            QString::fromLatin1(m_bytes.mid(runStart, std::min(runLen, 36)));
        if (runLen > 36)
          str.append(QStringLiteral("..."));
        items.append(makeStructItem(
            QStringLiteral("string"),
            QStringLiteral("\"") + str + QStringLiteral("\""),
            QStringLiteral("%1 B").arg(runLen), runStart, runLen));
        if (items.size() >= 80)
          break;
      }
      runStart = -1;
    }
  }

  m_structureItems = items;
  emit structureChanged();
}

} // namespace turbohex
