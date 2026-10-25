#pragma once

#include <QmlTui/Core/Items/Item.h>
#include <QString>
#include <vector>
#include <array>

namespace screensaver {

class DonutItem : public qmltui::Item {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal speedMultiplier READ speedMultiplier WRITE setSpeedMultiplier NOTIFY speedMultiplierChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    explicit DonutItem(QObject *parent = nullptr);
    ~DonutItem() override = default;

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

    Q_INVOKABLE void stepDonut();

protected:
    void paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
               const qmltui::ClipRect &clipRect) const override;

signals:
    void activeChanged();
    void pausedChanged();
    void speedMultiplierChanged();
    void themeChanged();

private:
    struct DonutCell {
        char ch = ' ';
        uint32_t color = 0;
    };

    void resizeBuffers();
    void updatePalette();

    bool m_active = true;
    bool m_paused = false;
    qreal m_speedMultiplier = 1.0;
    QString m_theme = QStringLiteral("cyber");

    int m_bufferWidth = 0;
    int m_bufferHeight = 0;
    float m_angleA = 0.0f;
    float m_angleB = 0.0f;

    std::vector<DonutCell> m_cells;
    std::vector<float> m_zBuffer;
    std::array<uint32_t, 12> m_palette{};
};

} // namespace screensaver
