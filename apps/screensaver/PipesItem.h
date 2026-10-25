#pragma once

#include <QmlTui/Core/Items/Item.h>
#include <QString>
#include <vector>
#include <string>

namespace screensaver {

class PipesItem : public qmltui::Item {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal speedMultiplier READ speedMultiplier WRITE setSpeedMultiplier NOTIFY speedMultiplierChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    explicit PipesItem(QObject *parent = nullptr);
    ~PipesItem() override = default;

    bool active() const { return m_active; }
    void setActive(bool active);

    bool paused() const { return m_paused; }
    void setPaused(bool paused);

    qreal speedMultiplier() const { return m_speedMultiplier; }
    void setSpeedMultiplier(qreal speed);

    QString theme() const { return m_theme; }
    void setTheme(const QString &theme);

    void setWidth(int width) override;
    void setHeight(int height) override;

    Q_INVOKABLE void stepPipes();
    Q_INVOKABLE void resetPipes();

protected:
    void paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
               const qmltui::ClipRect &clipRect) const override;

signals:
    void activeChanged();
    void pausedChanged();
    void speedMultiplierChanged();
    void themeChanged();

private:
    struct PipeCell {
        std::string glyph;
        uint32_t color = 0;
        int z = 0;
        bool occupied = false;
    };

    struct Pipe {
        int x = 0;
        int y = 0;
        int z = 2;
        int dx = 1;
        int dy = 0;
        int dz = 0;
        uint32_t baseColor = 0;
        int stepsSinceTurn = 0;
        bool alive = true;
    };

    void resizeGrid();
    void respawnPipe(Pipe &pipe);
    uint32_t applyDepthShading(uint32_t argb, int z) const;
    std::vector<uint32_t> themePalette() const;

    bool m_active = true;
    bool m_paused = false;
    qreal m_speedMultiplier = 1.0;
    QString m_theme = QStringLiteral("cyber");

    int m_gridWidth = 0;
    int m_gridHeight = 0;
    std::vector<PipeCell> m_grid;
    std::vector<Pipe> m_pipes;
    int m_occupiedCount = 0;

    uint32_t m_rng = 0x9e3779b9;
    uint32_t fastRand() {
        m_rng ^= m_rng << 13;
        m_rng ^= m_rng >> 17;
        m_rng ^= m_rng << 5;
        return m_rng;
    }
};

} // namespace screensaver
