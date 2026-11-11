#include "DonutItem.h"
#include <QmlTui/Core/Primitives/TextGridBuffer.h>
#include <cmath>
#include <algorithm>

namespace screensaver {

namespace {

constexpr const char LuminanceRamp[] = ".,-~:;=!*#$@";

constexpr std::array<uint32_t, 12> CyberPalette = {
    0xFF082F49, 0xFF0E7490, 0xFF06B6D4, 0xFF22D3EE,
    0xFF38BDF8, 0xFF67E8F9, 0xFFA5F3FC, 0xFFE879F9,
    0xFFF43F5E, 0xFFFB7185, 0xFFFED7AA, 0xFFFFFFFF
};

constexpr std::array<uint32_t, 12> EmeraldPalette = {
    0xFF052E16, 0xFF064E3B, 0xFF065F46, 0xFF047857,
    0xFF059669, 0xFF10B981, 0xFF34D399, 0xFF6EE7B7,
    0xFF86EFAC, 0xFFBBF7D0, 0xFFDCFCE7, 0xFFFFFFFF
};

constexpr std::array<uint32_t, 12> AmberPalette = {
    0xFF381400, 0xFF541E00, 0xFF782800, 0xFF9C3400,
    0xFFC2410C, 0xFFEA580C, 0xFFF97316, 0xFFF59E0B,
    0xFFFBBF24, 0xFFFDE047, 0xFFFEF08A, 0xFFFFFFFF
};

constexpr std::array<uint32_t, 12> CgaPalette = {
    0xFF0000AA, 0xFF00AA00, 0xFF00AAAA, 0xFFAA0000,
    0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA, 0xFF5555FF,
    0xFF55FF55, 0xFF55FFFF, 0xFFFFFF55, 0xFFFFFFFF
};

} // namespace

DonutItem::DonutItem(QObject *parent)
    : qmltui::Item(parent)
{
    updatePalette();
}

void DonutItem::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    emit activeChanged();
}

void DonutItem::setPaused(bool paused) {
    if (m_paused == paused) return;
    m_paused = paused;
    emit pausedChanged();
}

void DonutItem::setSpeedMultiplier(qreal speed) {
    if (qFuzzyCompare(m_speedMultiplier, speed)) return;
    m_speedMultiplier = std::clamp(speed, 0.25, 4.0);
    emit speedMultiplierChanged();
}

void DonutItem::setTheme(const QString &theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    updatePalette();
    emit themeChanged();
    markDirty();
}

void DonutItem::updatePalette() {
    if (m_theme == QLatin1StringView("amber"))
        m_palette = AmberPalette;
    else if (m_theme == QLatin1StringView("emerald"))
        m_palette = EmeraldPalette;
    else if (m_theme == QLatin1StringView("cga"))
        m_palette = CgaPalette;
    else
        m_palette = CyberPalette;
}

void DonutItem::setWidth(int width) {
    if (this->width() == width) return;
    qmltui::Item::setWidth(width);
    resizeBuffers();
}

void DonutItem::setHeight(int height) {
    if (this->height() == height) return;
    qmltui::Item::setHeight(height);
    resizeBuffers();
}

void DonutItem::resizeBuffers() {
    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) return;

    m_bufferWidth = w;
    m_bufferHeight = h;
    const size_t total = static_cast<size_t>(w * h);
    m_cells.assign(total, DonutCell{});
    m_zBuffer.assign(total, 0.0f);
}

void DonutItem::stepDonut() {
    const int w = m_bufferWidth;
    const int h = m_bufferHeight;
    if (w <= 0 || h <= 0 || m_cells.empty()) return;

    const float spd = static_cast<float>(m_speedMultiplier);
    m_angleA += 0.05f * spd;
    m_angleB += 0.03f * spd;

    const size_t total = static_cast<size_t>(w * h);
    std::fill(m_cells.begin(), m_cells.end(), DonutCell{});
    std::fill(m_zBuffer.begin(), m_zBuffer.end(), 0.0f);

    const float cosA = std::cos(m_angleA), sinA = std::sin(m_angleA);
    const float cosB = std::cos(m_angleB), sinB = std::sin(m_angleB);

    const float R1 = 1.0f;
    const float R2 = 2.0f;
    const float K2 = 5.0f;

    const float cx = w * 0.5f;
    const float cy = h * 0.5f;
    const float K1 = std::min(w * 0.42f, h * 0.85f) * K2 / (R1 + R2);

    const float theta_step = 0.07f;
    const float phi_step = 0.025f;

    for (float theta = 0.0f; theta < 6.28318f; theta += theta_step) {
        const float costheta = std::cos(theta), sintheta = std::sin(theta);

        for (float phi = 0.0f; phi < 6.28318f; phi += phi_step) {
            const float cosphi = std::cos(phi), sinphi = std::sin(phi);

            const float circlex = R2 + R1 * costheta;
            const float circley = R1 * sintheta;

            const float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            const float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            const float z = K2 + cosA * circlex * sinphi + circley * sinA;
            const float ooz = 1.0f / z;

            const int xp = static_cast<int>(cx + K1 * ooz * x);
            const int yp = static_cast<int>(cy + (K1 * 0.52f) * ooz * y);

            const float L = cosphi * costheta * sinB - cosA * costheta * sinphi -
                            sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi);

            if (L > 0.0f) {
                if (xp >= 0 && xp < w && yp >= 0 && yp < h) {
                    const size_t idx = static_cast<size_t>(yp * w + xp);
                    if (idx < total && ooz > m_zBuffer[idx]) {
                        m_zBuffer[idx] = ooz;
                        int lumIdx = std::clamp(static_cast<int>(L * 8.0f), 0, 11);
                        m_cells[idx].ch = LuminanceRamp[lumIdx];
                        m_cells[idx].color = m_palette[static_cast<size_t>(lumIdx)];
                    }
                }
            }
        }
    }

    markDirty();
}

void DonutItem::paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
                      const qmltui::ClipRect &clipRect) const {
    if (!visible()) return;

    const int w = m_bufferWidth;
    const int h = m_bufferHeight;
    if (w <= 0 || h <= 0 || m_cells.empty()) return;

    for (int y = 0; y < h; ++y) {
        const int screenY = absY + y;
        for (int x = 0; x < w; ++x) {
            const int screenX = absX + x;
            if (!clipRect.contains(screenX, screenY))
                continue;

            const size_t idx = static_cast<size_t>(y * w + x);
            if (idx >= m_cells.size()) continue;

            const auto &cellData = m_cells[idx];
            if (cellData.ch == ' ') continue;

            qmltui::Cell *cell = buffer.tryCellAt(screenX, screenY);
            if (!cell) continue;

            cell->displayWidth = 1;
            cell->continuation = false;
            char chStr[2] = { cellData.ch, '\0' };
            cell->setCharacter(QByteArrayView(chStr, 1));
            cell->fgColor = cellData.color;
            cell->bgColor = 0x00000000;
        }
    }
}

} // namespace screensaver
