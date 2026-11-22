#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

namespace turbohex {

// In-memory binary buffer model backing HexWindow, InspectorWindow, and
// StructureWindow. Decodes integers/floats/bits at cursorOffset, parses
// ELF/PNG/ZIP/PE/WASM headers + printable strings, supports hex/ASCII search,
// nibble/byte patching with undo/redo, and atomic QSaveFile persistence.
class BinaryDocument : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString filePath READ filePath NOTIFY documentReset)
  Q_PROPERTY(QString displayName READ displayName NOTIFY documentReset)
  Q_PROPERTY(QString formatBadge READ formatBadge NOTIFY documentReset)
  Q_PROPERTY(int byteCount READ byteCount NOTIFY documentReset)
  Q_PROPERTY(int cursorOffset READ cursorOffset WRITE setCursorOffset NOTIFY
                 cursorChanged)
  Q_PROPERTY(int selectionStart READ selectionStart NOTIFY selectionChanged)
  Q_PROPERTY(int selectionLength READ selectionLength NOTIFY selectionChanged)
  Q_PROPERTY(bool littleEndian READ littleEndian WRITE setLittleEndian NOTIFY
                 endianChanged)
  Q_PROPERTY(bool isModified READ isModified NOTIFY modifiedChanged)
  Q_PROPERTY(bool canUndo READ canUndo NOTIFY modifiedChanged)
  Q_PROPERTY(bool canRedo READ canRedo NOTIFY modifiedChanged)
  Q_PROPERTY(QString cursorHex READ cursorHex NOTIFY cursorChanged)
  Q_PROPERTY(
      QVariantList inspectorRows READ inspectorRows NOTIFY inspectorChanged)
  Q_PROPERTY(
      QVariantList structureItems READ structureItems NOTIFY structureChanged)

public:
  explicit BinaryDocument(QObject *parent = nullptr);
  ~BinaryDocument() override = default;

  QString filePath() const { return m_filePath; }
  QString displayName() const { return m_displayName; }
  QString formatBadge() const { return m_formatBadge; }
  int byteCount() const { return m_bytes.size(); }
  const QByteArray &rawBytes() const { return m_bytes; }

  int cursorOffset() const { return m_cursorOffset; }
  void setCursorOffset(int offset);

  int selectionStart() const { return m_selectionStart; }
  int selectionLength() const { return m_selectionLength; }

  bool littleEndian() const { return m_littleEndian; }
  void setLittleEndian(bool le);

  bool isModified() const { return !m_modifiedOffsets.isEmpty(); }
  bool canUndo() const { return !m_undoStack.isEmpty(); }
  bool canRedo() const { return !m_redoStack.isEmpty(); }
  bool isOffsetModified(int offset) const {
    return m_modifiedOffsets.contains(offset);
  }

  QString cursorHex() const;
  QVariantList inspectorRows() const { return m_inspectorRows; }
  QVariantList structureItems() const { return m_structureItems; }

  void loadBuffer(const QString &path, const QString &dispName,
                  const QByteArray &data);

  Q_INVOKABLE bool loadFile(const QString &path);
  Q_INVOKABLE bool save();
  Q_INVOKABLE bool saveAs(const QString &targetPath);
  Q_INVOKABLE void selectRange(int start, int length);
  Q_INVOKABLE void clearSelection();
  Q_INVOKABLE bool setByteAt(int offset, int byteValue);
  Q_INVOKABLE void undo();
  Q_INVOKABLE void redo();
  Q_INVOKABLE int findPattern(const QString &query, bool hexMode,
                              bool fromCursor = true);
  Q_INVOKABLE static int parseOffsetString(const QString &text);
  static QString detectFormatBadge(const QByteArray &bytes);

signals:
  void documentReset();
  void cursorChanged();
  void selectionChanged();
  void endianChanged();
  void modifiedChanged();
  void bytesEdited(int offset);
  void inspectorChanged();
  void structureChanged();

private:
  struct EditCmd {
    int offset;
    quint8 oldByte;
    quint8 newByte;
  };

  void rebuildInspector();
  void rebuildStructure();
  void recomputeModifiedSet();

  QString m_filePath;
  QString m_displayName = QStringLiteral("empty.bin");
  QString m_formatBadge = QStringLiteral("BIN");
  QByteArray m_savedBytes;
  QByteArray m_bytes;
  int m_cursorOffset = 0;
  int m_selectionStart = 0;
  int m_selectionLength = 0;
  bool m_littleEndian = true;

  QList<EditCmd> m_undoStack;
  QList<EditCmd> m_redoStack;
  QSet<int> m_modifiedOffsets;

  QVariantList m_inspectorRows;
  QVariantList m_structureItems;
};

} // namespace turbohex
