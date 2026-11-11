#pragma once

#include <QmlTui/Core/Items/Canvas.h>

#include <QList>
#include <QQmlEngine>

#include <deque>

// Artistic waveform renderer for a Braille Canvas. Like WaveformTrace it is a
// CanvasShape: QML binds the sample arrays and all drawing runs in C++.

// Phosphor persistence: keeps the last `depth` traces and draws them oldest first,
// fading from `fadeColor` to `color`, so fast material leaves glowing trails like an
// analog CRT scope.
class PersistenceTrace : public qmltui::CanvasShape
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QList<double> samples READ samples WRITE setSamples NOTIFY samplesChanged)
    Q_PROPERTY(int depth READ depth WRITE setDepth NOTIFY depthChanged)
    Q_PROPERTY(qreal amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(QString fadeColor READ fadeColor WRITE setFadeColor NOTIFY fadeColorChanged)

public:
    explicit PersistenceTrace(QObject* parent = nullptr);

    QList<double> samples() const { return m_history.empty() ? QList<double>() : m_history.back(); }
    void setSamples(const QList<double>& samples);
    int depth() const { return m_depth; }
    void setDepth(int depth);
    qreal amplitude() const { return m_amplitude; }
    void setAmplitude(qreal amplitude);
    QString fadeColor() const { return m_fadeColor; }
    void setFadeColor(const QString& color);

    void draw(qmltui::CanvasPainter& painter) const override;

signals:
    void samplesChanged();
    void depthChanged();
    void amplitudeChanged();
    void fadeColorChanged();

private:
    std::deque<QList<double>> m_history;
    int m_depth = 8;
    qreal m_amplitude = 0.9;
    QString m_fadeColor = QStringLiteral("ansi:black");
};
