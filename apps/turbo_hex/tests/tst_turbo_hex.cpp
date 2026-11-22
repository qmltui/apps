#include "BinaryDocument.h"
#include "HexBackend.h"
#include "HexGridView.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QmlTui/Core/Primitives/TextGridBuffer.h>
#include <QtTest/QtTest>

using namespace turbohex;

class TestTurboHex : public QObject {
  Q_OBJECT

private slots:
  void parsesElf64AndPngHeadersStringsAndInspectorValues() {
    BinaryDocument doc;
    QVERIFY(doc.loadFile(QStringLiteral("demo://firmware.elf")));
    QCOMPARE(doc.formatBadge(), QStringLiteral("ELF64"));
    QCOMPARE(doc.byteCount(), 256);
    QVERIFY(doc.littleEndian());

    // Structure items contain ELF Magic, e_entry (offset 24, len 8), and
    // strings
    bool foundEntry = false;
    bool foundFirmwareStr = false;
    for (const QVariant &v : doc.structureItems()) {
      const QVariantMap m = v.toMap();
      if (m.value(QStringLiteral("label")).toString() ==
          QStringLiteral("e_entry")) {
        foundEntry = true;
        QCOMPARE(m.value(QStringLiteral("offset")).toInt(), 24);
        QCOMPARE(m.value(QStringLiteral("length")).toInt(), 8);
        QVERIFY(m.value(QStringLiteral("detail"))
                    .toString()
                    .contains(QStringLiteral("0x00401000")));
      }
      if (m.value(QStringLiteral("label"))
              .toString()
              .contains(QStringLiteral("QmlTui::TVision Firmware"))) {
        foundFirmwareStr = true;
      }
    }
    QVERIFY(foundEntry);
    QVERIFY(foundFirmwareStr);

    // Jump to e_entry (offset 24 -> Little-Endian 0x00401000 = 4198400)
    doc.setCursorOffset(24);
    bool verifiedHex32 = false;
    for (const QVariant &v : doc.inspectorRows()) {
      const QVariantMap row = v.toMap();
      if (row.value(QStringLiteral("label")).toString() ==
          QStringLiteral("Hex32")) {
        QCOMPARE(row.value(QStringLiteral("value")).toString(),
                 QStringLiteral("0x00401000"));
        verifiedHex32 = true;
      }
    }
    QVERIFY(verifiedHex32);

    // PNG sample parses big-endian IHDR 32x16 chunk
    QVERIFY(doc.loadFile(QStringLiteral("demo://badge.png")));
    QCOMPARE(doc.formatBadge(), QStringLiteral("PNG"));
    QVERIFY(!doc.littleEndian());
    bool foundIhdr = false;
    for (const QVariant &v : doc.structureItems()) {
      const QVariantMap m = v.toMap();
      if (m.value(QStringLiteral("label")).toString() ==
          QStringLiteral("Chunk IHDR")) {
        QCOMPARE(m.value(QStringLiteral("detail")).toString(),
                 QStringLiteral("32x16 px"));
        foundIhdr = true;
      }
    }
    QVERIFY(foundIhdr);
  }

  void supportsHexAndAsciiSearchPatchUndoRedoAndAtomicSave() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString binPath =
        QDir(tempDir.path()).filePath(QStringLiteral("test.wasm"));

    QByteArray wasm("\x00"
                    "asm"
                    "\x01\x00\x00\x00"
                    "HELLO_HEX"
                    "\x00\xDE\xAD\xBE\xEF",
                    22);
    {
      QFile f(binPath);
      QVERIFY(f.open(QIODevice::WriteOnly));
      f.write(wasm);
    }

    BinaryDocument doc;
    QVERIFY(doc.loadFile(binPath));
    QCOMPARE(doc.formatBadge(), QStringLiteral("WASM"));

    // Hex pattern search (`DE AD BE EF` at offset 18)
    QCOMPARE(doc.findPattern(QStringLiteral("DE AD BE EF"), true, false), 18);
    QCOMPARE(doc.selectionStart(), 18);
    QCOMPARE(doc.selectionLength(), 4);

    // ASCII text search (`HELLO_HEX` at offset 8)
    QCOMPARE(doc.findPattern(QStringLiteral("HELLO_HEX"), false, false), 8);
    QCOMPARE(doc.cursorOffset(), 8);

    // Byte edit + undo + redo + save
    QVERIFY(!doc.isModified());
    QVERIFY(doc.setByteAt(8, 'J'));
    QVERIFY(doc.isModified());
    QCOMPARE(doc.rawBytes().at(8), 'J');

    doc.undo();
    QVERIFY(!doc.isModified());
    QCOMPARE(doc.rawBytes().at(8), 'H');

    doc.redo();
    QVERIFY(doc.isModified());
    QVERIFY(doc.save());
    QVERIFY(!doc.isModified());

    BinaryDocument reloaded;
    QVERIFY(reloaded.loadFile(binPath));
    QCOMPARE(reloaded.rawBytes().at(8), 'J');
  }

  void hexGridViewPaintsOffsetHexPairsAndAsciiColumn() {
    BinaryDocument doc;
    QVERIFY(doc.loadFile(QStringLiteral("demo://firmware.elf")));

    qmltui::TextGridBuffer buf;
    buf.resize(82, 6);

    HexGridView grid;
    grid.setDocument(&doc);
    grid.setWidth(80);
    grid.setHeight(5);
    QCOMPARE(grid.effectiveBytesPerRow(), 16);

    grid.paint(buf, 0, 0, qmltui::ClipRect{0, 0, 82, 6});

    // Row 1 is offset 00000000: bytes `7F 45 4C 46` -> hex at col 10..11 =
    // "7F", ASCII at col 61..64 = ".ELF"
    QCOMPARE(buf.cellAt(0, 1).character(), QByteArrayView("0"));
    QCOMPARE(buf.cellAt(10, 1).character(), QByteArrayView("7"));
    QCOMPARE(buf.cellAt(11, 1).character(), QByteArrayView("F"));
    QCOMPARE(buf.cellAt(13, 1).character(), QByteArrayView("4"));
    QCOMPARE(buf.cellAt(14, 1).character(), QByteArrayView("5"));
    QCOMPARE(buf.cellAt(61, 1).character(), QByteArrayView("."));
    QCOMPARE(buf.cellAt(62, 1).character(), QByteArrayView("E"));
    QCOMPARE(buf.cellAt(63, 1).character(), QByteArrayView("L"));
    QCOMPARE(buf.cellAt(64, 1).character(), QByteArrayView("F"));
  }
};

QTEST_MAIN(TestTurboHex)
#include "tst_turbo_hex.moc"
