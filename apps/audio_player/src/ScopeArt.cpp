#include "ScopeArt.h"

#include <QmlTui/Core/Primitives/Color.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

using qmltui::CanvasPainter;

namespace {

void requestRepaint(qmltui::Item* shape)
{
    if (shape->parentItem())
        shape->parentItem()->markDirty();
}

void bresenham(CanvasPainter& painter, int x0, int y0, int x1, int y1, uint32_t ink)
{
    const int dx = std::abs(x1 - x0);
    const int dy = -std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        painter.setPixelColorHex(x0, y0, ink);
        if (x0 == x1 && y0 == y1)
            break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

uint32_t mixColor(uint32_t from, uint32_t to, double t)
{
    from = qmltui::Color::toArgb(from);
    to = qmltui::Color::toArgb(to);
    const auto ch = [&](int shift) {
        const int a = (from >> shift) & 0xFF;
        const int b = (to >> shift) & 0xFF;
        return uint32_t(std::clamp(int(std::lround(a + (b - a) * t)), 0, 255)) << shift;
    };
    return (to & 0xFF000000u) | ch(16) | ch(8) | ch(0);
}

// Picks an ANSI palette step when `to` is an ANSI entry so persistence trails
// stay strictly on the terminal's 16-color ANSI palette; falls back to ARGB
// interpolation only for literal RGB inputs.
uint32_t shadeTraceColor(uint32_t from, uint32_t to, double t)
{
    if (qmltui::Color::isAnsiIndex(to) || qmltui::Color::isDefault(to)) {
        if (t >= 0.85)
            return to;
        if (t >= 0.45 && qmltui::Color::isAnsiIndex(to)) {
            const int baseIdx = qmltui::Color::ansiIndex(to) & 0x07;
            return qmltui::Color::fromAnsiIndex(baseIdx).argb32();
        }
        return qmltui::Color::isAnsiIndex(from)
                   ? from
                   : qmltui::Color::fromAnsiIndex(8).argb32(); // ansi:brightblack
    }
    return mixColor(from, to, 0.25 + 0.75 * t);
}

uint32_t argbFromString(const QString& text, uint32_t fallback)
{
    const qmltui::Color c = qmltui::Color::fromString(text);
    return c.isValid() ? c.argb32() : fallback;
}

bool silent(const QList<double>& samples, double threshold = 0.004)
{
    for (double v : samples)
        if (std::abs(v) >= threshold)
            return false;
    return true;
}

// Sample value at fractional position `pos` (0..n-1) with linear interpolation.
double sampleAt(const QList<double>& s, double pos)
{
    const int n = int(s.size());
    const int i0 = std::clamp(int(pos), 0, n - 1);
    const int i1 = std::min(n - 1, i0 + 1);
    const double t = pos - i0;
    return s[i0] * (1.0 - t) + s[i1] * t;
}

void drawPolyline(CanvasPainter& painter, const QList<double>& s, int x0, int x1,
                  double midY, double amp, uint32_t ink)
{
    const int n = int(s.size());
    const int width = x1 - x0 + 1;
    if (n < 2 || width < 2)
        return;
    const double step = double(n - 1) / (width - 1);
    int prevY = 0;
    for (int x = x0; x <= x1; ++x) {
        const int py = int(std::lround(midY - sampleAt(s, (x - x0) * step) * amp));
        if (x == x0)
            prevY = py;
        bresenham(painter, x == x0 ? x0 : x - 1, prevY, x, py, ink);
        prevY = py;
    }
}

} // namespace

// ---------------------------------------------------------------------------
// PersistenceTrace
// ---------------------------------------------------------------------------

PersistenceTrace::PersistenceTrace(QObject* parent)
    : qmltui::CanvasShape(parent)
{
}

void PersistenceTrace::setSamples(const QList<double>& samples)
{
    m_history.push_back(samples);
    while (int(m_history.size()) > std::max(1, m_depth))
        m_history.pop_front();
    emit samplesChanged();
    requestRepaint(this);
}

void PersistenceTrace::setDepth(int depth)
{
    depth = std::clamp(depth, 1, 64);
    if (depth == m_depth)
        return;
    m_depth = depth;
    while (int(m_history.size()) > m_depth)
        m_history.pop_front();
    emit depthChanged();
    requestRepaint(this);
}

void PersistenceTrace::setAmplitude(qreal amplitude)
{
    const qreal clamped = std::clamp(amplitude, 0.0, 1.0);
    if (qFuzzyCompare(m_amplitude, clamped))
        return;
    m_amplitude = clamped;
    emit amplitudeChanged();
    requestRepaint(this);
}

void PersistenceTrace::setFadeColor(const QString& color)
{
    if (m_fadeColor == color)
        return;
    m_fadeColor = color;
    emit fadeColorChanged();
    requestRepaint(this);
}

void PersistenceTrace::draw(CanvasPainter& painter) const
{
    const int pw = painter.pixelWidth();
    const int ph = painter.pixelHeight();
    if (pw <= 1 || ph <= 0 || m_history.empty())
        return;

    bool anySignal = false;
    for (const QList<double>& s : m_history)
        anySignal = anySignal || !silent(s);
    if (!anySignal)
        return;

    const uint32_t ink = resolvedColor(painter);
    const uint32_t fade = argbFromString(m_fadeColor, qmltui::Color::fromAnsiIndex(8).argb32());
    const double midY = ph / 2.0;
    const double amp = ph * 0.5 * m_amplitude;
    const int count = int(m_history.size());
    int index = 0;
    for (const QList<double>& s : m_history) {
        // Oldest frames sit close to the fade color, the newest is the pure ink.
        const double t = count > 1 ? double(index + 1) / count : 1.0;
        drawPolyline(painter, s, 0, pw - 1, midY, amp, shadeTraceColor(fade, ink, t));
        ++index;
    }
}

// ---------------------------------------------------------------------------
// RadialScopeTrace
// ---------------------------------------------------------------------------

namespace {

constexpr int kRadialSampleCount = 512;
constexpr int kRadialSeamBlend = 16;
constexpr double kRadialBaseOffset = 100.0;
constexpr double kRadialBassGain = 60.0;
constexpr double kRadialSampleGain = 50.0;
constexpr double kRadialMaxRef = kRadialBaseOffset + kRadialBassGain + kRadialSampleGain; // 210.0

// Extracts 512 samples and blends the last 16 samples with the first 16 so the
// circular waveform closes smoothly at angle 0 == 2*PI without a seam jump.
std::array<double, kRadialSampleCount> prepareRadialSamples(const QList<double>& input)
{
    std::array<double, kRadialSampleCount> raw{};
    const int n = int(input.size());
    if (n <= 0)
        return raw;

    if (n == kRadialSampleCount) {
        for (int i = 0; i < kRadialSampleCount; ++i)
            raw[size_t(i)] = std::clamp(input[i], -1.0, 1.0);
    } else {
        const double step = double(n - 1) / double(kRadialSampleCount - 1);
        for (int i = 0; i < kRadialSampleCount; ++i)
            raw[size_t(i)] = std::clamp(sampleAt(input, i * step), -1.0, 1.0);
    }

    std::array<double, kRadialSampleCount> out = raw;
    for (int k = 0; k < kRadialSeamBlend; ++k) {
        const double alpha = 0.5 * (1.0 - double(k) / double(kRadialSeamBlend));
        const double first = raw[size_t(k)];
        const double last = raw[size_t(kRadialSampleCount - 1 - k)];
        out[size_t(k)] = (1.0 - alpha) * first + alpha * last;
        out[size_t(kRadialSampleCount - 1 - k)] = (1.0 - alpha) * last + alpha * first;
    }
    return out;
}

void drawRadialRing(CanvasPainter& painter,
                    const std::array<double, kRadialSampleCount>& samples,
                    double centerX, double centerY,
                    double rBase, double scale, double radiusBiasDots,
                    uint32_t ink)
{
    int firstX = 0;
    int firstY = 0;
    int prevX = 0;
    int prevY = 0;

    for (int i = 0; i < kRadialSampleCount; ++i) {
        const double angle = (2.0 * std::numbers::pi * double(i)) / double(kRadialSampleCount);
        const double rRef = rBase + samples[size_t(i)] * kRadialSampleGain;
        const double r = std::max(1.0, rRef * scale + radiusBiasDots);
        const int x = int(std::lround(centerX + std::cos(angle) * r));
        const int y = int(std::lround(centerY + std::sin(angle) * r));
        if (i == 0) {
            firstX = prevX = x;
            firstY = prevY = y;
        } else {
            bresenham(painter, prevX, prevY, x, y, ink);
            prevX = x;
            prevY = y;
        }
    }
    bresenham(painter, prevX, prevY, firstX, firstY, ink);
}

} // namespace

RadialScopeTrace::RadialScopeTrace(QObject* parent)
    : qmltui::CanvasShape(parent)
{
}

void RadialScopeTrace::setSamples(const QList<double>& samples)
{
    m_history.push_back(Frame{ samples, double(m_bassEnergy) });
    while (int(m_history.size()) > std::max(1, m_depth))
        m_history.pop_front();
    emit samplesChanged();
    requestRepaint(this);
}

void RadialScopeTrace::setBassEnergy(qreal energy)
{
    const qreal clamped = std::clamp(energy, 0.0, 1.0);
    if (qFuzzyCompare(m_bassEnergy, clamped))
        return;
    m_bassEnergy = clamped;
    if (!m_history.empty())
        m_history.back().bassEnergy = double(m_bassEnergy);
    emit bassEnergyChanged();
    requestRepaint(this);
}

void RadialScopeTrace::setDepth(int depth)
{
    depth = std::clamp(depth, 1, 32);
    if (depth == m_depth)
        return;
    m_depth = depth;
    while (int(m_history.size()) > m_depth)
        m_history.pop_front();
    emit depthChanged();
    requestRepaint(this);
}

void RadialScopeTrace::setFadeColor(const QString& color)
{
    if (m_fadeColor == color)
        return;
    m_fadeColor = color;
    emit fadeColorChanged();
    requestRepaint(this);
}

void RadialScopeTrace::draw(CanvasPainter& painter) const
{
    const int pw = painter.pixelWidth();
    const int ph = painter.pixelHeight();
    if (pw <= 3 || ph <= 3 || m_history.empty())
        return;

    bool anySignal = false;
    for (const Frame& frame : m_history)
        anySignal = anySignal || !silent(frame.samples);
    if (!anySignal)
        return;

    const uint32_t ink = resolvedColor(painter);
    const uint32_t fade = argbFromString(m_fadeColor, qmltui::Color::fromAnsiIndex(8).argb32());
    const double centerX = (pw - 1) * 0.5;
    const double centerY = (ph - 1) * 0.5;
    const double maxRadiusDots = 0.46 * std::min(pw, ph);
    const double scale = maxRadiusDots / kRadialMaxRef;

    const int count = int(m_history.size());
    int index = 0;
    for (const Frame& frame : m_history) {
        if (silent(frame.samples)) {
            ++index;
            continue;
        }
        const std::array<double, kRadialSampleCount> blended = prepareRadialSamples(frame.samples);
        const double rBase = kRadialBaseOffset + std::clamp(frame.bassEnergy, 0.0, 1.0) * kRadialBassGain;
        const bool isNewest = (index == count - 1);
        const double t = count > 1 ? double(index + 1) / count : 1.0;

        if (isNewest) {
            // Glowing stroke: draw a 1-dot outer/inner halo in the mid-intensity ANSI shade
            // before drawing the bright core stroke on top.
            const uint32_t haloColor = shadeTraceColor(fade, ink, 0.55);
            drawRadialRing(painter, blended, centerX, centerY, rBase, scale, -1.0, haloColor);
            drawRadialRing(painter, blended, centerX, centerY, rBase, scale, 1.0, haloColor);
            drawRadialRing(painter, blended, centerX, centerY, rBase, scale, 0.0, ink);
        } else {
            drawRadialRing(painter, blended, centerX, centerY, rBase, scale, 0.0,
                           shadeTraceColor(fade, ink, t * 0.75));
        }
        ++index;
    }
}
