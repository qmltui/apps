#pragma once

#include <QmlTui/Image/ImageProvider.h>

#include <QImage>
#include <QSize>
#include <QString>
#include <QThread>

namespace qmltui::examples {

class PreviewWorker;

// Renders raster images, SVGs, multi-page PDFs, and procedural mock assets on a
// dedicated worker thread for the file manager's preview column. Registered
// under `image://preview/render?...` so superseded requests are cancelled
// immediately when the selection cursor moves.
class PreviewImageProvider : public qmltui::AsyncImageProvider {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PreviewImageProvider)
public:
  explicit PreviewImageProvider(QObject *parent = nullptr);
  ~PreviewImageProvider() override;

  qmltui::ImageResponse *
  requestImageResponse(const QString &id, const QSize &requestedSize) override;

  // Synthesizes a preview image for `path` (or mock asset when `isMock` is
  // true) fitted to `targetSize`. Safe to call from any worker thread.
  static QImage renderPreview(const QString &path, const QString &mimeType,
                              int page, bool isMock, const QSize &targetSize,
                              QString *errorOut = nullptr);

private:
  QThread m_thread;
  PreviewWorker *m_worker = nullptr;
};

} // namespace qmltui::examples
