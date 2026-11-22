#pragma once

#include "BinaryDocument.h"

#include <QPointer>
#include <QmlTui/Core/Items/Item.h>
#include <QmlTui/Core/Primitives/Color.h>
#include <QtQml/qqmlregistration.h>

namespace turbohex {

// BoundedSelfPaint Hex + ASCII grid item rendering `Offset | Hex Bytes | ASCII`
// in a single pass with responsive 8/16 bytes-per-row, range selection
// highlights, modified-byte highlights, mouse selection, and optional overwrite
// editing (`editMode`).
class HexGridView : public qmltui::Item {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(turbohex::BinaryDocument *document READ document WRITE setDocument
                 NOTIFY documentChanged)
  Q_PROPERTY(
      bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
  Q_PROPERTY(
      bool asciiActive READ asciiActive WRITE setAsciiActive NOTIFY paneChanged)
  Q_PROPERTY(
      int bytesPerRow READ effectiveBytesPerRow NOTIFY layoutMetricsChanged)
  Q_PROPERTY(int topRow READ topRow WRITE setTopRow NOTIFY topRowChanged)

  Q_PROPERTY(qmltui::Color backgroundColor READ backgroundColor WRITE
                 setBackgroundColor NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color headerColor READ headerColor WRITE setHeaderColor
                 NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color offsetColor READ offsetColor WRITE setOffsetColor
                 NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color byteColor READ byteColor WRITE setByteColor NOTIFY
                 colorsChanged)
  Q_PROPERTY(qmltui::Color modifiedColor READ modifiedColor WRITE
                 setModifiedColor NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color selectionBg READ selectionBg WRITE setSelectionBg
                 NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color selectionFg READ selectionFg WRITE setSelectionFg
                 NOTIFY colorsChanged)
  Q_PROPERTY(qmltui::Color cursorBg READ cursorBg WRITE setCursorBg NOTIFY
                 colorsChanged)
  Q_PROPERTY(qmltui::Color cursorFg READ cursorFg WRITE setCursorFg NOTIFY
                 colorsChanged)

public:
  explicit HexGridView(QObject *parent = nullptr);
  ~HexGridView() override = default;

  BinaryDocument *document() const { return m_doc.data(); }
  void setDocument(BinaryDocument *doc);

  bool editMode() const { return m_editMode; }
  void setEditMode(bool enabled);

  bool asciiActive() const { return m_asciiActive; }
  void setAsciiActive(bool active);

  int effectiveBytesPerRow() const;
  int topRow() const { return m_topRow; }
  void setTopRow(int row);

  qmltui::Color backgroundColor() const { return m_backgroundColor; }
  void setBackgroundColor(qmltui::Color c);
  qmltui::Color headerColor() const { return m_headerColor; }
  void setHeaderColor(qmltui::Color c);
  qmltui::Color offsetColor() const { return m_offsetColor; }
  void setOffsetColor(qmltui::Color c);
  qmltui::Color byteColor() const { return m_byteColor; }
  void setByteColor(qmltui::Color c);
  qmltui::Color modifiedColor() const { return m_modifiedColor; }
  void setModifiedColor(qmltui::Color c);
  qmltui::Color selectionBg() const { return m_selectionBg; }
  void setSelectionBg(qmltui::Color c);
  qmltui::Color selectionFg() const { return m_selectionFg; }
  void setSelectionFg(qmltui::Color c);
  qmltui::Color cursorBg() const { return m_cursorBg; }
  void setCursorBg(qmltui::Color c);
  qmltui::Color cursorFg() const { return m_cursorFg; }
  void setCursorFg(qmltui::Color c);

  Q_INVOKABLE void ensureCursorVisible();

  bool consumesPrintableKeys() const override { return m_editMode; }
  void keyPressEvent(qmltui::KeyEvent *event) override;
  void mousePressEvent(qmltui::MouseEvent *event) override;
  void mouseMoveEvent(qmltui::MouseEvent *event) override;
  void wheelEvent(qmltui::MouseEvent *event) override;

  void
  paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
        const qmltui::ClipRect &clipRect = qmltui::ClipRect()) const override;

signals:
  void documentChanged();
  void editModeChanged();
  void paneChanged();
  void layoutMetricsChanged();
  void topRowChanged();
  void colorsChanged();

private:
  int hitTestByteOffset(int localX, int localY, bool *outAsciiPane) const;
  void moveCursorBy(int deltaBytes);

  QPointer<BinaryDocument> m_doc;
  bool m_editMode = false;
  bool m_asciiActive = false;
  bool m_lowNibblePending = false;
  int m_topRow = 0;
  int m_dragAnchorOffset = -1;

  qmltui::Color m_backgroundColor = qmltui::Color::fromArgb32(0xFF0000AA);
  qmltui::Color m_headerColor = qmltui::Color::fromArgb32(0xFF55FFFF);
  qmltui::Color m_offsetColor = qmltui::Color::fromArgb32(0xFF88BBEE);
  qmltui::Color m_byteColor = qmltui::Color::fromArgb32(0xFFFFFFFF);
  qmltui::Color m_modifiedColor = qmltui::Color::fromArgb32(0xFFFFFF55);
  qmltui::Color m_selectionBg = qmltui::Color::fromArgb32(0xFF008888);
  qmltui::Color m_selectionFg = qmltui::Color::fromArgb32(0xFFFFFFFF);
  qmltui::Color m_cursorBg = qmltui::Color::fromArgb32(0xFF00AA00);
  qmltui::Color m_cursorFg = qmltui::Color::fromArgb32(0xFF000000);
};

} // namespace turbohex
