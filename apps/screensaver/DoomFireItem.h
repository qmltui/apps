#pragma once

#include <QmlTui/Core/Items/Item.h>
#include <QmlTui/Core/Primitives/Subpixel.h>
#include <QTimer>
#include <QString>
#include <vector>
#include <array>

namespace screensaver {

class DoomFireItem : public qmltui::Item {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal speedMultiplier READ speedMultiplier WRITE setSpeedMultiplier NOTIFY speedMultiplierChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    explicit DoomFireItem(QObject *parent = nullptr);
    ~DoomFireItem() override = default;

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

    Q_INVOKABLE void stepFire();

protected:
    void paint(qmltui::TextGridBuffer &buffer, int absX, int absY,
               const qmltui::ClipRect &clipRect) const override;

signals:
    void activeChanged();
    void pausedChanged();
    void speedMultiplierChanged();
    void themeChanged();

private:
    void resizeBuffer();
    void updatePalette();

    bool m_active = true;
    bool m_paused = false;
    qreal m_speedMultiplier = 1.0;
    QString m_theme = QStringLiteral("cyber");

    int m_bufferWidth = 0;
    int m_bufferHeight = 0; // subpixel height = height * 2
    std::vector<uint8_t> m_firePixels;
    std::array<uint32_t, 36> m_palette{};
    uint32_t m_rngState = 0x12345678;

    uint32_t fastRand() {
        m_rngState ^= m_rngState << 13;
        m_rngState ^= m_rngState >> 17;
        m_rngState ^= m_rngState << 5;
        return m_rngState;
    }
};

} // namespace screensaver
