#include "PreviewImageProvider.h"

#include <QByteArray>
#include <QColor>
#include <QFileInfo>
#include <QFont>
#include <QImageReader>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPdfDocument>
#include <QPointer>
#include <QRadialGradient>
#include <QSvgRenderer>
#include <QUrl>
#include <QUrlQuery>
#include <QtMath>
#include <algorithm>
#include <atomic>
#include <memory>

namespace qmltui::examples {

namespace {

// Paints a dark two-tone checkerboard so translucent regions in PNGs and SVGs
// stand out against the terminal background.
void paintCheckerboard(QPainter &painter, const QRect &rect) {
  const int tile =
      std::clamp(std::min(rect.width(), rect.height()) / 12, 2, 12);
  const QColor dark(22, 25, 35);
  const QColor light(36, 41, 56);
  painter.fillRect(rect, dark);
  for (int y = rect.top(); y <= rect.bottom(); y += tile) {
    const int row = (y - rect.top()) / tile;
    for (int x = rect.left(); x <= rect.right(); x += tile) {
      const int col = (x - rect.left()) / tile;
      if ((row + col) % 2 == 1) {
        const int w = std::min(tile, rect.right() - x + 1);
        const int h = std::min(tile, rect.bottom() - y + 1);
        painter.fillRect(x, y, w, h, light);
      }
    }
  }
}

QImage paintMockLogo(const QSize &size) {
  QImage img(size, QImage::Format_ARGB32);
  img.fill(Qt::transparent);
  QPainter p(&img);
  p.setRenderHint(QPainter::Antialiasing, true);

  paintCheckerboard(p, img.rect());

  const double w = size.width();
  const double h = size.height();
  const double cx = w * 0.5;
  const double cy = h * 0.5;
  const double r = std::min(w, h) * 0.42;

  // Outer glowing badge ring
  QRadialGradient glow(QPointF(cx, cy), r * 1.15);
  glow.setColorAt(0.0, QColor(56, 189, 248, 220));
  glow.setColorAt(0.65, QColor(99, 102, 241, 200));
  glow.setColorAt(1.0, QColor(15, 23, 42, 0));
  p.setPen(Qt::NoPen);
  p.setBrush(glow);
  p.drawEllipse(QPointF(cx, cy), r * 1.1, r * 1.1);

  // Core dark hexagon/circle plate
  p.setBrush(QColor(15, 23, 42, 240));
  p.setPen(QPen(QColor(56, 189, 248), std::max(1.0, r * 0.08)));
  p.drawEllipse(QPointF(cx, cy), r * 0.85, r * 0.85);

  // Terminal prompt chevron `>` and underscore `_`
  QPainterPath chevron;
  chevron.moveTo(cx - r * 0.42, cy - r * 0.34);
  chevron.lineTo(cx - r * 0.04, cy);
  chevron.lineTo(cx - r * 0.42, cy + r * 0.34);
  p.setPen(QPen(QColor(74, 222, 128), std::max(1.5, r * 0.14), Qt::SolidLine,
                Qt::RoundCap, Qt::RoundJoin));
  p.setBrush(Qt::NoBrush);
  p.drawPath(chevron);

  p.setPen(QPen(QColor(244, 114, 182), std::max(1.5, r * 0.14), Qt::SolidLine,
                Qt::RoundCap));
  p.drawLine(QPointF(cx + r * 0.08, cy + r * 0.30),
             QPointF(cx + r * 0.44, cy + r * 0.30));

  return img;
}

QImage paintMockBanner(const QSize &size) {
  QImage img(size, QImage::Format_RGB32);
  QPainter p(&img);
  p.setRenderHint(QPainter::Antialiasing, true);

  const double w = size.width();
  const double h = size.height();
  const double horizon = h * 0.62;

  // Twilight sky gradient
  QLinearGradient sky(0, 0, 0, horizon);
  sky.setColorAt(0.0, QColor(17, 24, 68));
  sky.setColorAt(0.55, QColor(88, 28, 135));
  sky.setColorAt(1.0, QColor(244, 63, 94));
  p.fillRect(QRectF(0, 0, w, horizon), sky);

  // Retro sun disc
  const double sunR = std::min(w, h) * 0.24;
  const QPointF sunCenter(w * 0.5, horizon - sunR * 0.35);
  QLinearGradient sunGrad(0, sunCenter.y() - sunR, 0, sunCenter.y() + sunR);
  sunGrad.setColorAt(0.0, QColor(253, 224, 71));
  sunGrad.setColorAt(1.0, QColor(249, 115, 22));
  p.setPen(Qt::NoPen);
  p.setBrush(sunGrad);
  p.drawEllipse(sunCenter, sunR, sunR);

  // Mountain range silhouette
  QPainterPath mountains;
  mountains.moveTo(0, horizon);
  mountains.lineTo(w * 0.18, horizon - h * 0.22);
  mountains.lineTo(w * 0.35, horizon - h * 0.08);
  mountains.lineTo(w * 0.54, horizon - h * 0.28);
  mountains.lineTo(w * 0.74, horizon - h * 0.12);
  mountains.lineTo(w * 0.88, horizon - h * 0.24);
  mountains.lineTo(w, horizon);
  mountains.closeSubpath();
  p.setBrush(QColor(15, 23, 42));
  p.drawPath(mountains);

  // Foreground grid plane
  QLinearGradient ground(0, horizon, 0, h);
  ground.setColorAt(0.0, QColor(30, 27, 75));
  ground.setColorAt(1.0, QColor(9, 13, 28));
  p.fillRect(QRectF(0, horizon, w, h - horizon), ground);

  p.setPen(QPen(QColor(56, 189, 248, 160), 1));
  for (int i = 1; i <= 4; ++i) {
    const double ratio = static_cast<double>(i) / 5.0;
    const double y = horizon + (h - horizon) * (ratio * ratio);
    p.drawLine(QPointF(0, y), QPointF(w, y));
  }
  for (int i = -4; i <= 4; ++i) {
    const double topX = w * 0.5 + i * (w * 0.05);
    const double botX = w * 0.5 + i * (w * 0.22);
    p.drawLine(QPointF(topX, horizon), QPointF(botX, h));
  }

  return img;
}

QImage paintMockSvg(const QSize &size) {
  static const QByteArray svgPayload = QByteArrayLiteral(
      "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 120 80'>"
      "<rect x='4' y='4' width='112' height='72' rx='10' fill='#0f172a' "
      "stroke='#38bdf8' "
      "stroke-width='3'/>"
      "<circle cx='32' cy='40' r='12' fill='#22c55e'/>"
      "<circle cx='88' cy='24' r='10' fill='#f59e0b'/>"
      "<circle cx='88' cy='56' r='10' fill='#ec4899'/>"
      "<line x1='43' y1='35' x2='78' y2='26' stroke='#e2e8f0' "
      "stroke-width='3'/>"
      "<line x1='43' y1='45' x2='78' y2='54' stroke='#e2e8f0' "
      "stroke-width='3'/>"
      "</svg>");

  QImage img(size, QImage::Format_ARGB32);
  img.fill(Qt::transparent);
  QPainter p(&img);
  p.setRenderHint(QPainter::Antialiasing, true);
  paintCheckerboard(p, img.rect());

  QSvgRenderer renderer(svgPayload);
  if (renderer.isValid()) {
    QSize svgSize = renderer.defaultSize();
    if (svgSize.isEmpty())
      svgSize = QSize(120, 80);
    svgSize.scale(size, Qt::KeepAspectRatio);
    const QRect targetRect((size.width() - svgSize.width()) / 2,
                           (size.height() - svgSize.height()) / 2,
                           svgSize.width(), svgSize.height());
    renderer.render(&p, targetRect);
  }
  return img;
}

QImage paintMockPdfPage(int page, const QSize &size) {
  const int clampedPage = std::clamp(page, 0, 2);
  QImage img(size, QImage::Format_RGB32);
  img.fill(QColor(18, 22, 32));
  QPainter p(&img);
  p.setRenderHint(QPainter::Antialiasing, true);

  const int marginX = std::max(1, size.width() / 16);
  const int marginY = std::max(1, size.height() / 16);
  const QRectF sheet(marginX, marginY, std::max(4, size.width() - 2 * marginX),
                     std::max(4, size.height() - 2 * marginY));

  // White paper sheet
  p.setPen(QPen(QColor(148, 163, 184), 1));
  p.setBrush(QColor(248, 250, 252));
  p.drawRect(sheet);

  const double sx = sheet.left();
  const double sy = sheet.top();
  const double sw = sheet.width();
  const double sh = sheet.height();

  if (clampedPage == 0) {
    // Page 1: Architecture & 3-Column Miller Diagram
    p.fillRect(QRectF(sx, sy, sw, sh * 0.18), QColor(37, 99, 235));
    const double colTop = sy + sh * 0.26;
    const double colH = sh * 0.62;
    const double pad = sw * 0.05;
    const double colW = (sw - pad * 4) / 3.0;
    p.fillRect(QRectF(sx + pad, colTop, colW, colH), QColor(147, 197, 253));
    p.fillRect(QRectF(sx + pad * 2 + colW, colTop, colW, colH),
               QColor(59, 130, 246));
    p.fillRect(QRectF(sx + pad * 3 + colW * 2, colTop, colW, colH),
               QColor(16, 185, 129));
  } else if (clampedPage == 1) {
    // Page 2: Benchmark Bar Chart
    p.fillRect(QRectF(sx, sy, sw, sh * 0.18), QColor(217, 119, 6));
    const double chartBottom = sy + sh * 0.88;
    const double maxBarH = sh * 0.58;
    const double barW = sw * 0.14;
    const double ratios[4] = {0.42, 0.78, 0.95, 0.64};
    const QColor colors[4] = {QColor(239, 68, 68), QColor(245, 158, 11),
                              QColor(34, 197, 94), QColor(59, 130, 246)};
    for (int i = 0; i < 4; ++i) {
      const double bx = sx + sw * (0.12 + i * 0.20);
      const double bh = maxBarH * ratios[i];
      p.fillRect(QRectF(bx, chartBottom - bh, barW, bh), colors[i]);
    }
  } else {
    // Page 3: Terminal Pixel Protocol Matrix
    p.fillRect(QRectF(sx, sy, sw, sh * 0.18), QColor(147, 51, 234));
    const double gridTop = sy + sh * 0.26;
    const double gridH = sh * 0.62;
    const double cellW = sw * 0.24;
    const double cellH = gridH * 0.42;
    const QColor palette[6] = {QColor(236, 72, 153), QColor(168, 85, 247),
                               QColor(56, 189, 248), QColor(34, 197, 94),
                               QColor(250, 204, 21), QColor(249, 115, 22)};
    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 3; ++c) {
        const double gx = sx + sw * 0.08 + c * (cellW + sw * 0.06);
        const double gy = gridTop + r * (cellH + gridH * 0.10);
        p.fillRect(QRectF(gx, gy, cellW, cellH), palette[r * 3 + c]);
      }
    }
  }

  return img;
}

} // namespace

// GUI-thread handle owned by the requesting `qmltui::Image`. Holds a
// thread-safe cancellation flag shared with `PreviewWorker` so superseded
// requests skip work.
class PreviewImageResponse : public qmltui::ImageResponse {
  Q_OBJECT
public:
  explicit PreviewImageResponse(std::shared_ptr<std::atomic_bool> cancelled)
      : m_cancelled(std::move(cancelled)) {}

  QImage image() const override { return m_image; }
  QString errorString() const override { return m_error; }
  void cancel() override {
    if (m_cancelled)
      m_cancelled->store(true, std::memory_order_relaxed);
  }

  void complete(const QImage &image, const QString &error) {
    if (m_cancelled && m_cancelled->load(std::memory_order_relaxed))
      return;
    m_image = image;
    m_error = error;
    emit finished();
  }

private:
  QImage m_image;
  QString m_error;
  std::shared_ptr<std::atomic_bool> m_cancelled;
};

// Worker object living on `PreviewImageProvider::m_thread`. Executes queued
// preview jobs sequentially and abandons any job whose `cancelled` token is
// set.
class PreviewWorker : public QObject {
  Q_OBJECT
public:
  void process(const QString &path, const QString &mimeType, int page,
               bool isMock, const QSize &targetSize, QObject *receiver,
               const QPointer<PreviewImageResponse> &response,
               const std::shared_ptr<std::atomic_bool> &cancelled) {
    if (cancelled && cancelled->load(std::memory_order_relaxed))
      return;

    QString error;
    const QImage rendered = PreviewImageProvider::renderPreview(
        path, mimeType, page, isMock, targetSize, &error);

    if (cancelled && cancelled->load(std::memory_order_relaxed))
      return;

    QMetaObject::invokeMethod(
        receiver,
        [response, rendered, error] {
          if (response)
            response->complete(rendered, error);
        },
        Qt::QueuedConnection);
  }
};

PreviewImageProvider::PreviewImageProvider(QObject *parent)
    : qmltui::AsyncImageProvider(parent) {
  m_worker = new PreviewWorker;
  m_worker->moveToThread(&m_thread);
  connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
  m_thread.setObjectName(QStringLiteral("fm-preview-worker"));
  m_thread.start();
}

PreviewImageProvider::~PreviewImageProvider() {
  m_thread.quit();
  m_thread.wait();
}

qmltui::ImageResponse *
PreviewImageProvider::requestImageResponse(const QString &id,
                                           const QSize &requestedSize) {
  const int queryPos = id.indexOf(QLatin1Char('?'));
  const QString endpoint = queryPos < 0 ? id : id.left(queryPos);
  if (endpoint != QLatin1String("render"))
    return nullptr;

  const QUrlQuery query(queryPos < 0 ? QString() : id.mid(queryPos + 1));
  const QString path = QUrl::fromPercentEncoding(
      query.queryItemValue(QStringLiteral("path")).toUtf8());
  const QString mime = QUrl::fromPercentEncoding(
      query.queryItemValue(QStringLiteral("mime")).toUtf8());
  const int page =
      std::max(0, query.queryItemValue(QStringLiteral("page")).toInt());
  const bool isMock =
      query.queryItemValue(QStringLiteral("mock")) == QLatin1String("1");
  const int queryW = query.queryItemValue(QStringLiteral("w")).toInt();
  const int queryH = query.queryItemValue(QStringLiteral("h")).toInt();

  QSize target = requestedSize;
  if (!target.isValid() || target.isEmpty()) {
    target =
        (queryW > 0 && queryH > 0) ? QSize(queryW, queryH) : QSize(160, 96);
  }
  target.setWidth(std::clamp(target.width(), 8, 2048));
  target.setHeight(std::clamp(target.height(), 8, 2048));

  auto cancelled = std::make_shared<std::atomic_bool>(false);
  auto *response = new PreviewImageResponse(cancelled);

  QMetaObject::invokeMethod(
      m_worker,
      [worker = m_worker, receiver = this, path, mime, page, isMock, target,
       response = QPointer<PreviewImageResponse>(response), cancelled] {
        worker->process(path, mime, page, isMock, target, receiver, response,
                        cancelled);
      },
      Qt::QueuedConnection);

  return response;
}

QImage PreviewImageProvider::renderPreview(const QString &path,
                                           const QString &mimeType, int page,
                                           bool isMock, const QSize &targetSize,
                                           QString *errorOut) {
  const QSize safeSize(std::clamp(targetSize.width(), 8, 2048),
                       std::clamp(targetSize.height(), 8, 2048));
  const QString lowerPath = path.toLower();
  const QString lowerMime = mimeType.toLower();

  if (isMock) {
    if (lowerPath.endsWith(QLatin1String(".svg")) ||
        lowerMime.contains(QLatin1String("svg"))) {
      return paintMockSvg(safeSize);
    }
    if (lowerPath.endsWith(QLatin1String(".pdf")) ||
        lowerMime.contains(QLatin1String("pdf"))) {
      return paintMockPdfPage(page, safeSize);
    }
    if (lowerPath.endsWith(QLatin1String("banner.jpg")) ||
        lowerMime.contains(QLatin1String("jpeg"))) {
      return paintMockBanner(safeSize);
    }
    return paintMockLogo(safeSize);
  }

  // 1. Live SVG vector preview
  if (lowerPath.endsWith(QLatin1String(".svg")) ||
      lowerPath.endsWith(QLatin1String(".svgz")) ||
      lowerMime.contains(QLatin1String("svg"))) {
    QSvgRenderer svg(path);
    if (!svg.isValid()) {
      if (errorOut)
        *errorOut = QStringLiteral("Invalid SVG document: %1").arg(path);
      return QImage();
    }
    QImage out(safeSize, QImage::Format_ARGB32);
    out.fill(Qt::transparent);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing, true);
    paintCheckerboard(p, out.rect());

    QSize natural = svg.defaultSize();
    if (natural.isEmpty())
      natural = safeSize;
    natural.scale(safeSize, Qt::KeepAspectRatio);
    const QRect dest((safeSize.width() - natural.width()) / 2,
                     (safeSize.height() - natural.height()) / 2,
                     natural.width(), natural.height());
    svg.render(&p, dest);
    return out;
  }

  // 2. Live multi-page PDF document preview
  if (lowerPath.endsWith(QLatin1String(".pdf")) ||
      lowerMime.contains(QLatin1String("pdf"))) {
    QPdfDocument doc;
    const QPdfDocument::Error err = doc.load(path);
    if (err != QPdfDocument::Error::None || doc.pageCount() <= 0) {
      if (errorOut)
        *errorOut = QStringLiteral("Unable to render PDF: %1").arg(path);
      return QImage();
    }
    const int clampedPage = std::clamp(page, 0, doc.pageCount() - 1);
    const QSizeF pts = doc.pagePointSize(clampedPage);
    QSize pageTarget = safeSize;
    if (pts.width() > 0.0 && pts.height() > 0.0) {
      pageTarget = QSize(qRound(pts.width() * 2.0), qRound(pts.height() * 2.0))
                       .scaled(safeSize, Qt::KeepAspectRatio);
    }
    pageTarget.setWidth(std::max(1, pageTarget.width()));
    pageTarget.setHeight(std::max(1, pageTarget.height()));

    QImage out(pageTarget, QImage::Format_ARGB32);
    out.fill(Qt::white);
    QPainter p(&out);
    const QImage renderedPage = doc.render(clampedPage, pageTarget);
    if (!renderedPage.isNull()) {
      p.drawImage(0, 0, renderedPage);
    }
    return out;
  }

  // 3. Live raster image preview (PNG, JPEG, WebP, BMP, GIF, ICO, ...)
  QImageReader reader(path);
  reader.setAutoTransform(true);
  const QSize naturalSize = reader.size();
  if (naturalSize.isValid() && !naturalSize.isEmpty()) {
    if (naturalSize.width() > safeSize.width() * 2 ||
        naturalSize.height() > safeSize.height() * 2) {
      reader.setScaledSize(
          naturalSize.scaled(safeSize * 2, Qt::KeepAspectRatio));
    }
  }
  const QImage decoded = reader.read();
  if (decoded.isNull()) {
    if (errorOut)
      *errorOut = reader.errorString();
    return QImage();
  }

  if (decoded.hasAlphaChannel()) {
    QImage composited(decoded.size(), QImage::Format_ARGB32);
    QPainter p(&composited);
    paintCheckerboard(p, composited.rect());
    p.drawImage(0, 0, decoded);
    return composited;
  }
  return decoded;
}

} // namespace qmltui::examples

#include "PreviewImageProvider.moc"
