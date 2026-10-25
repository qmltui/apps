#include "DoomFireItem.h"
#include <algorithm>

namespace screensaver {

namespace {

constexpr std::array<uint32_t, 36> ClassicPalette = {
    0xFF070707, 0xFF1f0707, 0xFF2f0f07, 0xFF470f07, 0xFF571707, 0xFF671f07, 0xFF771f07,
    0xFF8f2707, 0xFF9f2f07, 0xFFaf3f07, 0xFFbf4707, 0xFFc74707, 0xFFdf4f07, 0xFFdf5707,
    0xFFdf5707, 0xFFd75f07, 0xFFd7670f, 0xFFcf6f0f, 0xFFcf770f, 0xFFcf7f0f, 0xFFcf8717,
    0xFFc78717, 0xFFc78f17, 0xFFc7971f, 0xFFbf9f1f, 0xFFbf9f1f, 0xFFbfa727, 0xFFbfa727,
    0xFFbfaf2f, 0xFFb7af2f, 0xFFb7b72f, 0xFFb7b737, 0xFFcfcf6f, 0xFFdfdf9f, 0xFFefefc7,
    0xFFffffff
};

constexpr std::array<uint32_t, 36> IcePalette = {
    0xFF020617, 0xFF0f172a, 0xFF1e1b4b, 0xFF312e81, 0xFF3730a3, 0xFF4338ca, 0xFF4f46e5,
    0xFF6366f1, 0xFF4338ca, 0xFF3b82f6, 0xFF2563eb, 0xFF1d4ed8, 0xFF0284c7, 0xFF0369a1,
    0xFF0ea5e9, 0xFF38bdf8, 0xFF7dd3fc, 0xFFa5f3fc, 0xFFcffafe, 0xFFe0f2fe, 0xFFf0fdf4,
    0xFF38bdf8, 0xFF06b6d4, 0xFF0891b2, 0xFF0e7490, 0xFF155e75, 0xFF22d3ee, 0xFF67e8f9,
    0xFFa5f3fc, 0xFFbae6fd, 0xFFe0f2fe, 0xFFf8fafc, 0xFFffffff, 0xFFffffff, 0xFFffffff,
    0xFFffffff
};

constexpr std::array<uint32_t, 36> EmeraldPalette = {
    0xFF05130b, 0xFF062314, 0xFF064e3b, 0xFF047857, 0xFF059669, 0xFF10b981, 0xFF34d399,
    0xFF064e3b, 0xFF047857, 0xFF059669, 0xFF10b981, 0xFF34d399, 0xFF6ee7b7, 0xFFa7f3d0,
    0xFF059669, 0xFF10b981, 0xFF34d399, 0xFF6ee7b7, 0xFFa7f3d0, 0xFFd1fae5, 0xFFecfdf5,
    0xFF34d399, 0xFF6ee7b7, 0xFFa7f3d0, 0xFFd1fae5, 0xFFbbf7d0, 0xFF86efac, 0xFF4ade80,
    0xFF22c55e, 0xFF16a34a, 0xFF86efac, 0xFFbbf7d0, 0xFFdcfce7, 0xFFf0fdf4, 0xFFffffff,
    0xFFffffff
};

constexpr std::array<uint32_t, 36> AmberPalette = {
    0xFF110700, 0xFF260e00, 0xFF451a03, 0xFF78350f, 0xFF92400e, 0xFFb45309, 0xFFd97706,
    0xFFf59e0b, 0xFF78350f, 0xFF92400e, 0xFFb45309, 0xFFd97706, 0xFFf59e0b, 0xFFfbbf24,
    0xFFfcd34d, 0xFFfde68a, 0xFFfef3c7, 0xFFfffbeb, 0xFFf59e0b, 0xFFfbbf24, 0xFFfcd34d,
    0xFFfde68a, 0xFFfef08a, 0xFFfef9c3, 0xFFfef08a, 0xFFfde047, 0xFFfacc15, 0xFFeab308,
    0xFFca8a04, 0xFFa16207, 0xFFfef08a, 0xFFfef9c3, 0xFFfefce8, 0xFFffffff, 0xFFffffff,
    0xFFffffff
};

} // namespace

DoomFireItem::DoomFireItem(QObject *parent)
    : qmltui::Item(parent)
{
    updatePalette();
}

void DoomFireItem::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    emit activeChanged();
}

void DoomFireItem::setPaused(bool paused) {
    if (m_paused == paused) return;
    m_paused = paused;
    emit pausedChanged();
}

void DoomFireItem::setSpeedMultiplier(qreal speed) {
    if (qFuzzyCompare(m_speedMultiplier, speed)) return;
    m_speedMultiplier = std::clamp(speed, 0.25, 4.0);
    emit speedMultiplierChanged();
}

void DoomFireItem::setTheme(const QString &theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    updatePalette();
    emit themeChanged();
    markDirty();
}

void DoomFireItem::updatePalette() {
    if (m_theme == QLatin1StringView("amber"))
        m_palette = AmberPalette;
    else if (m_theme == QLatin1StringView("emerald"))
        m_palette = EmeraldPalette;
    else if (m_theme == QLatin1StringView("cga"))
        m_palette = IcePalette;
    else
        m_palette = ClassicPalette;
}

void DoomFireItem::setWidth(int width) {
    if (this->width() == width) return;
    qmltui::Item::setWidth(width);
    resizeBuffer();
}

void DoomFireItem::setHeight(int height) {
    if (this->height() == height) return;
    qmltui::Item::setHeight(height);
    resizeBuffer();
}

void DoomFireItem::resizeBuffer() {
    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) return;

    m_bufferWidth = w;
    m_bufferHeight = h * qmltui::half_block::SubpixelsY; // 2 subpixels per cell row
    m_firePixels.assign(static_cast<size_t>(m_bufferWidth * m_bufferHeight), 0);

    // Seed bottom row with max heat (intensity 35)
    const int bottomRow = (m_bufferHeight - 1) * m_bufferWidth;
    for (int x = 0; x < m_bufferWidth; ++x) {
        m_firePixels[static_cast<size_t>(bottomRow + x)] = 35;
    }
}

void DoomFireItem::stepFire() {
    const int w = m_bufferWidth;
    const int h = m_bufferHeight;
    if (w <= 0 || h <= 0 || m_firePixels.empty())
        return;

    const int decayRange = (m_speedMultiplier >= 1.5) ? 2 : ((m_speedMultiplier <= 0.6) ? 4 : 3);

    // Propagate fire upwards
    for (int x = 0; x < w; ++x) {
        for (int y = 1; y < h; ++y) {
            const int from = y * w + x;
            const uint8_t pixel = m_firePixels[static_cast<size_t>(from)];
            if (pixel == 0) {
                m_firePixels[static_cast<size_t>(from - w)] = 0;
            } else {
                const int randIdx = static_cast<int>(fastRand() % 3);
                const int dst = from - randIdx + 1;
                if (dst >= w) {
                    const int decay = static_cast<int>(fastRand() % decayRange);
                    m_firePixels[static_cast<size_t>(dst - w)] = static_cast<uint8_t>(std::max(0, pixel - decay));
                }
            }
        }
    }

    // Keep bottom row burning
    const int bottomRow = (h - 1) * w;
    for (int x = 0; x < w; ++x) {
        m_firePixels[static_cast<size_t>(bottomRow + x)] = (fastRand() % 100 > 8) ? 35 : 28;
    }

    markDirty();
}

void DoomFireItem::paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
                         const qmltui::ClipRect &clipRect) const {
    if (!visible())
        return;

    const int w = width();
    const int h = height();
    const int subW = m_bufferWidth;
    const int subH = m_bufferHeight;

    if (w <= 0 || h <= 0 || subW <= 0 || subH <= 0 || m_firePixels.empty())
        return;

    for (int cellY = 0; cellY < h; ++cellY) {
        const int screenY = absY + cellY;
        const int topSubY = cellY * qmltui::half_block::SubpixelsY;
        const int bottomSubY = cellY * qmltui::half_block::SubpixelsY + 1;

        for (int cellX = 0; cellX < w; ++cellX) {
            const int screenX = absX + cellX;
            if (!clipRect.contains(screenX, screenY))
                continue;

            const size_t topIdx = static_cast<size_t>(topSubY * subW + cellX);
            const size_t bottomIdx = static_cast<size_t>(bottomSubY * subW + cellX);

            const uint8_t topVal = (topSubY < subH && cellX < subW && topIdx < m_firePixels.size())
                ? m_firePixels[topIdx] : 0;
            const uint8_t bottomVal = (bottomSubY < subH && cellX < subW && bottomIdx < m_firePixels.size())
                ? m_firePixels[bottomIdx] : 0;

            if (topVal <= 1 && bottomVal <= 1)
                continue;

            qmltui::Cell *cell = buffer.tryCellAt(screenX, screenY);
            if (!cell)
                continue;

            cell->displayWidth = 1;
            cell->continuation = false;

            const uint32_t topColor = m_palette[std::min<size_t>(35, topVal)];
            const uint32_t bottomColor = m_palette[std::min<size_t>(35, bottomVal)];

            if (topVal > 1 && bottomVal > 1) {
                cell->setCharacter(qmltui::block_glyphs::UpperHalf);
                cell->fgColor = topColor;
                cell->bgColor = bottomColor;
            } else if (topVal > 1) {
                cell->setCharacter(qmltui::block_glyphs::UpperHalf);
                cell->fgColor = topColor;
                cell->bgColor = 0x00000000;
            } else {
                cell->setCharacter(qmltui::block_glyphs::LowerHalf);
                cell->fgColor = bottomColor;
                cell->bgColor = 0x00000000;
            }
        }
    }
}

} // namespace screensaver
