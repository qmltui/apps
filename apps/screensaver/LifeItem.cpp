#include "LifeItem.h"
#include <QmlTui/Core/Primitives/TextGridBuffer.h>
#include <algorithm>

namespace screensaver {

LifeItem::LifeItem(QObject *parent)
    : qmltui::Item(parent)
{
    updatePalette();
}

void LifeItem::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    emit activeChanged();
}

void LifeItem::setPaused(bool paused) {
    if (m_paused == paused) return;
    m_paused = paused;
    emit pausedChanged();
}

void LifeItem::setSpeedMultiplier(qreal speed) {
    if (qFuzzyCompare(m_speedMultiplier, speed)) return;
    m_speedMultiplier = std::clamp(speed, 0.25, 4.0);
    emit speedMultiplierChanged();
}

void LifeItem::setTheme(const QString &theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    updatePalette();
    emit themeChanged();
    markDirty();
}

void LifeItem::updatePalette() {
    if (m_theme == QLatin1StringView("amber")) {
        m_pal.newborn   = 0xFFFFFFFF;
        m_pal.young     = 0xFFFEF08A;
        m_pal.prime     = 0xFFF59E0B;
        m_pal.veteran   = 0xFFD97706;
        m_pal.decayHigh = 0xFF92400E;
        m_pal.decayMid  = 0xFF78350F;
        m_pal.decayLow  = 0xFF451A03;
    } else if (m_theme == QLatin1StringView("emerald")) {
        m_pal.newborn   = 0xFFFFFFFF;
        m_pal.young     = 0xFF86EFAC;
        m_pal.prime     = 0xFF10B981;
        m_pal.veteran   = 0xFF059669;
        m_pal.decayHigh = 0xFF047857;
        m_pal.decayMid  = 0xFF064E3B;
        m_pal.decayLow  = 0xFF052E16;
    } else if (m_theme == QLatin1StringView("cga")) {
        m_pal.newborn   = 0xFFFFFFFF;
        m_pal.young     = 0xFFFFFF55;
        m_pal.prime     = 0xFF55FFFF;
        m_pal.veteran   = 0xFFFF55FF;
        m_pal.decayHigh = 0xFF00AAAA;
        m_pal.decayMid  = 0xFFAA00AA;
        m_pal.decayLow  = 0xFF005555;
    } else { // cyber
        m_pal.newborn   = 0xFFFFFFFF;
        m_pal.young     = 0xFFA5F3FC;
        m_pal.prime     = 0xFF22D3EE;
        m_pal.veteran   = 0xFF818CF8;
        m_pal.decayHigh = 0xFF0284C7;
        m_pal.decayMid  = 0xFF1E3A8A;
        m_pal.decayLow  = 0xFF0F172A;
    }
}

void LifeItem::setWidth(int width) {
    if (this->width() == width) return;
    qmltui::Item::setWidth(width);
    resizeGrid();
}

void LifeItem::setHeight(int height) {
    if (this->height() == height) return;
    qmltui::Item::setHeight(height);
    resizeGrid();
}

void LifeItem::resizeGrid() {
    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) return;

    m_gridWidth = w;
    m_gridHeight = h;
    resetLife();
}

void LifeItem::resetLife() {
    const int w = m_gridWidth;
    const int h = m_gridHeight;
    if (w <= 0 || h <= 0) return;

    const size_t total = static_cast<size_t>(w * h);
    m_current.assign(total, LifeCell{});
    m_next.assign(total, LifeCell{});
    m_stagnantGenerations = 0;

    // Seed ~20% random live cells
    for (size_t i = 0; i < total; ++i) {
        if ((fastRand() % 100) < 22) {
            m_current[i].alive = 1;
            m_current[i].age = static_cast<uint8_t>(fastRand() % 5);
        }
    }

    // Inject 2-3 gliders in open areas
    injectPattern(0);
    injectPattern(1);

    markDirty();
}

void LifeItem::injectPattern(int patternType) {
    const int w = m_gridWidth;
    const int h = m_gridHeight;
    if (w < 10 || h < 10) return;

    int ox = static_cast<int>(fastRand() % static_cast<uint32_t>(w - 8)) + 2;
    int oy = static_cast<int>(fastRand() % static_cast<uint32_t>(h - 8)) + 2;

    auto setCell = [this, w, h](int x, int y) {
        int gx = (x % w + w) % w;
        int gy = (y % h + h) % h;
        size_t idx = static_cast<size_t>(gy * w + gx);
        if (idx < m_current.size()) {
            m_current[idx].alive = 1;
            m_current[idx].age = 0;
            m_current[idx].decay = 0;
        }
    };

    if (patternType == 0) {
        // Glider
        setCell(ox + 1, oy + 0);
        setCell(ox + 2, oy + 1);
        setCell(ox + 0, oy + 2);
        setCell(ox + 1, oy + 2);
        setCell(ox + 2, oy + 2);
    } else if (patternType == 1) {
        // Lightweight Spaceship (LWSS)
        setCell(ox + 1, oy + 0);
        setCell(ox + 4, oy + 0);
        setCell(ox + 0, oy + 1);
        setCell(ox + 0, oy + 2);
        setCell(ox + 4, oy + 2);
        setCell(ox + 0, oy + 3);
        setCell(ox + 1, oy + 3);
        setCell(ox + 2, oy + 3);
        setCell(ox + 3, oy + 3);
    } else if (patternType == 2) {
        // Pulsar seed / cross
        for (int i = 0; i < 7; ++i) {
            setCell(ox + i, oy + 3);
            setCell(ox + 3, oy + i);
        }
    } else {
        // R-pentomino
        setCell(ox + 1, oy + 0);
        setCell(ox + 2, oy + 0);
        setCell(ox + 0, oy + 1);
        setCell(ox + 1, oy + 1);
        setCell(ox + 1, oy + 2);
    }
}

void LifeItem::stepLife() {
    const int w = m_gridWidth;
    const int h = m_gridHeight;
    if (w <= 0 || h <= 0 || m_current.empty()) return;

    int changedCount = 0;
    int livingCount = 0;

    for (int y = 0; y < h; ++y) {
        const int ym = (y == 0 ? h - 1 : y - 1) * w;
        const int y0 = y * w;
        const int yp = (y == h - 1 ? 0 : y + 1) * w;

        for (int x = 0; x < w; ++x) {
            const int xm = (x == 0 ? w - 1 : x - 1);
            const int xp = (x == w - 1 ? 0 : x + 1);

            const int neighbors =
                m_current[static_cast<size_t>(ym + xm)].alive +
                m_current[static_cast<size_t>(ym + x )].alive +
                m_current[static_cast<size_t>(ym + xp)].alive +
                m_current[static_cast<size_t>(y0 + xm)].alive +
                m_current[static_cast<size_t>(y0 + xp)].alive +
                m_current[static_cast<size_t>(yp + xm)].alive +
                m_current[static_cast<size_t>(yp + x )].alive +
                m_current[static_cast<size_t>(yp + xp)].alive;

            const size_t idx = static_cast<size_t>(y0 + x);
            const auto &cur = m_current[idx];
            auto &nxt = m_next[idx];

            if (cur.alive) {
                if (neighbors == 2 || neighbors == 3) {
                    nxt.alive = 1;
                    nxt.age = static_cast<uint8_t>(std::min(255, cur.age + 1));
                    nxt.decay = 0;
                    ++livingCount;
                } else {
                    nxt.alive = 0;
                    nxt.age = 0;
                    nxt.decay = 3;
                    ++changedCount;
                }
            } else {
                if (neighbors == 3) {
                    nxt.alive = 1;
                    nxt.age = 0;
                    nxt.decay = 0;
                    ++changedCount;
                    ++livingCount;
                } else {
                    nxt.alive = 0;
                    nxt.age = 0;
                    nxt.decay = (cur.decay > 0) ? static_cast<uint8_t>(cur.decay - 1) : 0;
                }
            }
        }
    }

    m_current.swap(m_next);

    // Stagnation detection
    if (changedCount < 4 || livingCount < 12) {
        ++m_stagnantGenerations;
        if (m_stagnantGenerations > 20) {
            m_stagnantGenerations = 0;
            if (livingCount < 8) {
                resetLife();
            } else {
                injectPattern(static_cast<int>(fastRand() % 4));
            }
        }
    } else {
        m_stagnantGenerations = 0;
    }

    markDirty();
}

void LifeItem::paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
                     const qmltui::ClipRect &clipRect) const {
    if (!visible()) return;

    const int w = m_gridWidth;
    const int h = m_gridHeight;
    if (w <= 0 || h <= 0 || m_current.empty()) return;

    for (int y = 0; y < h; ++y) {
        const int screenY = absY + y;
        for (int x = 0; x < w; ++x) {
            const int screenX = absX + x;
            if (!clipRect.contains(screenX, screenY))
                continue;

            const size_t idx = static_cast<size_t>(y * w + x);
            const auto &c = m_current[idx];

            if (!c.alive && c.decay == 0)
                continue;

            qmltui::Cell *cell = buffer.tryCellAt(screenX, screenY);
            if (!cell) continue;

            cell->displayWidth = 1;
            cell->continuation = false;
            cell->bgColor = 0x00000000;

            if (c.alive) {
                if (c.age == 0) {
                    cell->setCharacter("■");
                    cell->fgColor = m_pal.newborn;
                } else if (c.age <= 3) {
                    cell->setCharacter("■");
                    cell->fgColor = m_pal.young;
                } else if (c.age <= 12) {
                    cell->setCharacter("▣");
                    cell->fgColor = m_pal.prime;
                } else {
                    cell->setCharacter("◆");
                    cell->fgColor = m_pal.veteran;
                }
            } else {
                // Phosphor decay trail
                cell->setCharacter("·");
                if (c.decay == 3)
                    cell->fgColor = m_pal.decayHigh;
                else if (c.decay == 2)
                    cell->fgColor = m_pal.decayMid;
                else
                    cell->fgColor = m_pal.decayLow;
            }
        }
    }
}

} // namespace screensaver
