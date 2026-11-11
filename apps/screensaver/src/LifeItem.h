#pragma once

#include <QmlTui/Core/Items/Item.h>
#include <QString>
#include <vector>
#include <array>

namespace screensaver {

class LifeItem : public qmltui::Item {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal speedMultiplier READ speedMultiplier WRITE setSpeedMultiplier NOTIFY speedMultiplierChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    explicit LifeItem(QObject *parent = nullptr);
    ~LifeItem() override = default;

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

    Q_INVOKABLE void stepLife();
    Q_INVOKABLE void resetLife();

protected:
    void paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
               const qmltui::ClipRect &clipRect) const override;

signals:
    void activeChanged();
    void pausedChanged();
    void speedMultiplierChanged();
    void themeChanged();

private:
    struct LifeCell {
        uint8_t alive = 0;
        uint8_t age = 0;
        uint8_t decay = 0;
    };

    struct Palette {
        uint32_t newborn = 0xFFFFFFFF;
        uint32_t young = 0;
        uint32_t prime = 0;
        uint32_t veteran = 0;
        uint32_t decayHigh = 0;
        uint32_t decayMid = 0;
        uint32_t decayLow = 0;
    };

    void resizeGrid();
    void updatePalette();
    void injectPattern(int patternType);

    bool m_active = true;
    bool m_paused = false;
    qreal m_speedMultiplier = 1.0;
    QString m_theme = QStringLiteral("cyber");

    int m_gridWidth = 0;
    int m_gridHeight = 0;
    std::vector<LifeCell> m_current;
    std::vector<LifeCell> m_next;
    Palette m_pal;

    int m_stagnantGenerations = 0;
    uint32_t m_rng = 0x13579bdf;
    uint32_t fastRand() {
        m_rng ^= m_rng << 13;
        m_rng ^= m_rng >> 17;
        m_rng ^= m_rng << 5;
        return m_rng;
    }
};

} // namespace screensaver
