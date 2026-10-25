#include "PipesItem.h"
#include <QmlTui/Core/Primitives/TextGridBuffer.h>
#include <algorithm>
#include <array>

namespace screensaver {

namespace {

constexpr int MaxZ = 4;
constexpr int NumPipes = 6;

} // namespace

PipesItem::PipesItem(QObject *parent)
    : qmltui::Item(parent)
{
    m_pipes.resize(NumPipes);
}

void PipesItem::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    emit activeChanged();
}

void PipesItem::setPaused(bool paused) {
    if (m_paused == paused) return;
    m_paused = paused;
    emit pausedChanged();
}

void PipesItem::setSpeedMultiplier(qreal speed) {
    if (qFuzzyCompare(m_speedMultiplier, speed)) return;
    m_speedMultiplier = std::clamp(speed, 0.25, 4.0);
    emit speedMultiplierChanged();
}

void PipesItem::setTheme(const QString &theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    emit themeChanged();
    resetPipes();
}

void PipesItem::setWidth(int width) {
    if (this->width() == width) return;
    qmltui::Item::setWidth(width);
    resizeGrid();
}

void PipesItem::setHeight(int height) {
    if (this->height() == height) return;
    qmltui::Item::setHeight(height);
    resizeGrid();
}

std::vector<uint32_t> PipesItem::themePalette() const {
    if (m_theme == QLatin1StringView("amber")) {
        return { 0xFFF59E0B, 0xFFFBBF24, 0xFFF97316, 0xFFFEF08A, 0xFFEA580C, 0xFFFDE047 };
    } else if (m_theme == QLatin1StringView("emerald")) {
        return { 0xFF10B981, 0xFF34D399, 0xFF4ADE80, 0xFF059669, 0xFF6EE7B7, 0xFF22D3EE };
    } else if (m_theme == QLatin1StringView("cga")) {
        return { 0xFF55FFFF, 0xFFFF55FF, 0xFFFFFFFF, 0xFFFFFF55, 0xFFFF5555, 0xFF55FF55 };
    } else { // cyber
        return { 0xFF00FFFF, 0xFFFF007F, 0xFFA855F7, 0xFF38BDF8, 0xFFF43F5E, 0xFF67E8F9 };
    }
}

uint32_t PipesItem::applyDepthShading(uint32_t argb, int z) const {
    float factor = 0.45f + 0.55f * (static_cast<float>(z) / MaxZ);
    uint32_t a = (argb >> 24) & 0xFF;
    uint32_t r = static_cast<uint32_t>(((argb >> 16) & 0xFF) * factor);
    uint32_t g = static_cast<uint32_t>(((argb >> 8) & 0xFF) * factor);
    uint32_t b = static_cast<uint32_t>((argb & 0xFF) * factor);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void PipesItem::resizeGrid() {
    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) return;

    m_gridWidth = w;
    m_gridHeight = h;
    resetPipes();
}

void PipesItem::resetPipes() {
    if (m_gridWidth <= 0 || m_gridHeight <= 0) return;

    m_grid.assign(static_cast<size_t>(m_gridWidth * m_gridHeight), PipeCell{});
    m_occupiedCount = 0;

    auto pal = themePalette();
    for (size_t i = 0; i < m_pipes.size(); ++i) {
        m_pipes[i].baseColor = pal[i % pal.size()];
        respawnPipe(m_pipes[i]);
    }
    markDirty();
}

void PipesItem::respawnPipe(Pipe &pipe) {
    if (m_gridWidth <= 0 || m_gridHeight <= 0) return;

    pipe.x = static_cast<int>(fastRand() % static_cast<uint32_t>(m_gridWidth));
    pipe.y = static_cast<int>(fastRand() % static_cast<uint32_t>(m_gridHeight));
    pipe.z = static_cast<int>(fastRand() % (MaxZ + 1));
    pipe.stepsSinceTurn = 0;
    pipe.alive = true;

    // Pick a random initial direction (horizontal or vertical)
    int dir = static_cast<int>(fastRand() % 4);
    switch (dir) {
        case 0: pipe.dx = 1;  pipe.dy = 0;  pipe.dz = 0; break;
        case 1: pipe.dx = -1; pipe.dy = 0;  pipe.dz = 0; break;
        case 2: pipe.dx = 0;  pipe.dy = 1;  pipe.dz = 0; break;
        default: pipe.dx = 0; pipe.dy = -1; pipe.dz = 0; break;
    }

    size_t idx = static_cast<size_t>(pipe.y * m_gridWidth + pipe.x);
    if (idx < m_grid.size()) {
        m_grid[idx].glyph = "●";
        m_grid[idx].color = applyDepthShading(pipe.baseColor, pipe.z);
        m_grid[idx].z = pipe.z;
        m_grid[idx].occupied = true;
        ++m_occupiedCount;
    }
}

void PipesItem::stepPipes() {
    if (m_gridWidth <= 0 || m_gridHeight <= 0 || m_grid.empty())
        return;

    // If screen is getting saturated, reset smoothly
    const int maxOccupied = static_cast<int>(m_gridWidth * m_gridHeight * 0.35);
    if (m_occupiedCount >= maxOccupied) {
        resetPipes();
        return;
    }

    auto pal = themePalette();

    for (size_t i = 0; i < m_pipes.size(); ++i) {
        auto &pipe = m_pipes[i];
        if (!pipe.alive) {
            pipe.baseColor = pal[(i + fastRand()) % pal.size()];
            respawnPipe(pipe);
            continue;
        }

        int oldDx = pipe.dx;
        int oldDy = pipe.dy;
        int oldDz = pipe.dz;

        // Decide whether to turn
        bool shouldTurn = (pipe.stepsSinceTurn >= 3 && (fastRand() % 100 < 22));

        // Predictive boundary check: must turn if next step is out of bounds
        int testX = pipe.x + pipe.dx;
        int testY = pipe.y + pipe.dy;
        if (testX < 0 || testX >= m_gridWidth || testY < 0 || testY >= m_gridHeight) {
            shouldTurn = true;
        }

        if (shouldTurn) {
            std::vector<std::array<int, 3>> validDirs;

            if (oldDx != 0) {
                // Moving horizontally: can turn up, down, or depth
                validDirs.push_back({0, 1, 0});
                validDirs.push_back({0, -1, 0});
                if (pipe.z < MaxZ) validDirs.push_back({0, 0, 1});
                if (pipe.z > 0)    validDirs.push_back({0, 0, -1});
            } else if (oldDy != 0) {
                // Moving vertically: can turn left, right, or depth
                validDirs.push_back({1, 0, 0});
                validDirs.push_back({-1, 0, 0});
                if (pipe.z < MaxZ) validDirs.push_back({0, 0, 1});
                if (pipe.z > 0)    validDirs.push_back({0, 0, -1});
            } else {
                // Moving in depth: can turn any 2D direction
                validDirs.push_back({1, 0, 0});
                validDirs.push_back({-1, 0, 0});
                validDirs.push_back({0, 1, 0});
                validDirs.push_back({0, -1, 0});
            }

            // Filter out directions that immediately go out of bounds
            std::vector<std::array<int, 3>> viableDirs;
            for (const auto &d : validDirs) {
                int nx = pipe.x + d[0];
                int ny = pipe.y + d[1];
                int nz = pipe.z + d[2];
                if (nx >= 0 && nx < m_gridWidth && ny >= 0 && ny < m_gridHeight && nz >= 0 && nz <= MaxZ) {
                    viableDirs.push_back(d);
                }
            }

            if (!viableDirs.empty()) {
                auto chosen = viableDirs[fastRand() % viableDirs.size()];
                pipe.dx = chosen[0];
                pipe.dy = chosen[1];
                pipe.dz = chosen[2];
                pipe.stepsSinceTurn = 0;

                // Place the appropriate turn elbow glyph at current (pipe.x, pipe.y)
                size_t curIdx = static_cast<size_t>(pipe.y * m_gridWidth + pipe.x);
                if (curIdx < m_grid.size()) {
                    std::string elbow = "◆";
                    if (oldDx > 0 && pipe.dy > 0) elbow = "┐";
                    else if (oldDx > 0 && pipe.dy < 0) elbow = "┘";
                    else if (oldDx < 0 && pipe.dy > 0) elbow = "┌";
                    else if (oldDx < 0 && pipe.dy < 0) elbow = "└";
                    else if (oldDy > 0 && pipe.dx > 0) elbow = "└";
                    else if (oldDy > 0 && pipe.dx < 0) elbow = "┘";
                    else if (oldDy < 0 && pipe.dx > 0) elbow = "┌";
                    else if (oldDy < 0 && pipe.dx < 0) elbow = "┐";
                    else if (pipe.dz != 0 || oldDz != 0) elbow = "◆";

                    m_grid[curIdx].glyph = elbow;
                    m_grid[curIdx].color = applyDepthShading(pipe.baseColor, pipe.z);
                    m_grid[curIdx].z = pipe.z;
                }
            } else {
                // Stuck! Cap with terminal flange and kill
                size_t curIdx = static_cast<size_t>(pipe.y * m_gridWidth + pipe.x);
                if (curIdx < m_grid.size()) {
                    m_grid[curIdx].glyph = "■";
                }
                pipe.alive = false;
                continue;
            }
        }

        // Advance pipe
        int nextX = pipe.x + pipe.dx;
        int nextY = pipe.y + pipe.dy;
        int nextZ = std::clamp(pipe.z + pipe.dz, 0, MaxZ);

        if (nextX < 0 || nextX >= m_gridWidth || nextY < 0 || nextY >= m_gridHeight) {
            size_t curIdx = static_cast<size_t>(pipe.y * m_gridWidth + pipe.x);
            if (curIdx < m_grid.size()) {
                m_grid[curIdx].glyph = "■";
            }
            pipe.alive = false;
            continue;
        }

        size_t nextIdx = static_cast<size_t>(nextY * m_gridWidth + nextX);
        if (nextIdx >= m_grid.size()) {
            pipe.alive = false;
            continue;
        }

        auto &targetCell = m_grid[nextIdx];
        if (targetCell.occupied) {
            if (targetCell.z < nextZ) {
                // Current pipe is in front: draw over
                targetCell.glyph = (pipe.dx != 0) ? "─" : ((pipe.dy != 0) ? "│" : "○");
                targetCell.color = applyDepthShading(pipe.baseColor, nextZ);
                targetCell.z = nextZ;
            } else if (targetCell.z == nextZ) {
                // Same depth collision: place cross or cap
                if (fastRand() % 100 < 50) {
                    targetCell.glyph = "┼";
                } else {
                    targetCell.glyph = "■";
                    pipe.alive = false;
                    continue;
                }
            }
            // If targetCell.z > nextZ, target is in front, so current pipe slips behind without overwriting
        } else {
            // Unoccupied cell
            targetCell.occupied = true;
            targetCell.glyph = (pipe.dx != 0) ? "─" : ((pipe.dy != 0) ? "│" : "○");
            targetCell.color = applyDepthShading(pipe.baseColor, nextZ);
            targetCell.z = nextZ;
            ++m_occupiedCount;
        }

        pipe.x = nextX;
        pipe.y = nextY;
        pipe.z = nextZ;
        pipe.stepsSinceTurn++;
    }

    markDirty();
}

void PipesItem::paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
                      const qmltui::ClipRect &clipRect) const {
    if (!visible()) return;

    const int w = m_gridWidth;
    const int h = m_gridHeight;
    if (w <= 0 || h <= 0 || m_grid.empty()) return;

    for (int y = 0; y < h; ++y) {
        int screenY = absY + y;
        for (int x = 0; x < w; ++x) {
            int screenX = absX + x;
            if (!clipRect.contains(screenX, screenY))
                continue;

            size_t idx = static_cast<size_t>(y * w + x);
            if (idx >= m_grid.size() || !m_grid[idx].occupied)
                continue;

            const auto &cellData = m_grid[idx];
            qmltui::Cell *cell = buffer.tryCellAt(screenX, screenY);
            if (!cell) continue;

            cell->displayWidth = 1;
            cell->continuation = false;
            cell->setCharacter(QByteArrayView(cellData.glyph.data(), cellData.glyph.size()));
            cell->fgColor = cellData.color;
            cell->bgColor = 0x00000000;
        }
    }

    // Paint active pipe heads with glowing white spark
    for (const auto &pipe : m_pipes) {
        if (!pipe.alive) continue;
        int screenX = absX + pipe.x;
        int screenY = absY + pipe.y;
        if (!clipRect.contains(screenX, screenY)) continue;

        qmltui::Cell *cell = buffer.tryCellAt(screenX, screenY);
        if (cell) {
            cell->setCharacter("●");
            cell->fgColor = 0xFFFFFFFF; // bright white head
        }
    }
}

} // namespace screensaver
