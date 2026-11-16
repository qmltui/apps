pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Item {
    id: root
    anchors.fill: parent

    property bool active: true
    property bool paused: false
    property real speedMultiplier: 1.0
    property string theme: "cyber"

    readonly property int starCount: 160
    property var stars: []

    function initStars() {
        let arr = [];
        for (let i = 0; i < starCount; ++i) {
            arr.push({
                x: (Math.random() - 0.5) * 2000,
                y: (Math.random() - 0.5) * 1600,
                z: Math.random() * 1000 + 1,
                pz: 1000
            });
        }
        stars = arr;
    }

    Component.onCompleted: initStars()

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.HalfBlock
        autoClear: false
        fillColor: "transparent"

        onPixelWidthChanged: {
            if (pixelWidth > 0 && root.stars.length === 0)
                root.initStars();
        }
    }

    function getStarColor(z) {
        if (theme === "amber") {
            if (z < 250)
                return "#ffffff";
            if (z < 500)
                return "#fef08a";
            if (z < 750)
                return "#f59e0b";
            return "#78350f";
        } else if (theme === "emerald") {
            if (z < 250)
                return "#ffffff";
            if (z < 500)
                return "#86efac";
            if (z < 750)
                return "#16a34a";
            return "#064e3b";
        } else if (theme === "cga") {
            if (z < 250)
                return "#ffffff";
            if (z < 500)
                return "#ffff55";
            if (z < 750)
                return "#ff55ff";
            return "#55ffff";
        } else { // cyber
            if (z < 220)
                return "#ffffff";
            if (z < 480)
                return "#a5f3fc";
            if (z < 750)
                return "#38bdf8";
            return "#1e3a8a";
        }
    }

    function drawLine(x0, y0, x1, y1, color) {
        let dx = Math.abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        let dy = -Math.abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        let err = dx + dy;
        let x = x0, y = y0;
        let w = canvas.pixelWidth;
        let h = canvas.pixelHeight;
        while (true) {
            if (x >= 0 && x < w && y >= 0 && y < h)
                canvas.setPixelColor(x, y, color);
            if (x === x1 && y === y1)
                break;
            let e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y += sy;
            }
        }
    }

    Timer {
        id: frameTimer
        interval: 33
        running: root.active && !root.paused
        repeat: true
        onTriggered: {
            let pw = canvas.pixelWidth;
            let ph = canvas.pixelHeight;
            if (pw <= 0 || ph <= 0 || root.stars.length === 0)
                return;
            canvas.clear();

            let cx = pw / 2;
            let cy = ph / 2;
            let speed = 22 * root.speedMultiplier;

            for (let i = 0; i < root.stars.length; ++i) {
                let star = root.stars[i];
                star.pz = star.z;
                star.z -= speed;

                if (star.z <= 10) {
                    star.x = (Math.random() - 0.5) * 2000;
                    star.y = (Math.random() - 0.5) * 1600;
                    star.z = 1000;
                    star.pz = 1000;
                }

                let sx = Math.round(cx + (star.x / star.z) * cx);
                let sy = Math.round(cy + (star.y / star.z) * cy);

                if (sx < 0 || sx >= pw || sy < 0 || sy >= ph) {
                    star.x = (Math.random() - 0.5) * 2000;
                    star.y = (Math.random() - 0.5) * 1600;
                    star.z = 1000;
                    star.pz = 1000;
                    continue;
                }

                let col = root.getStarColor(star.z);
                let px = Math.round(cx + (star.x / star.pz) * cx);
                let py = Math.round(cy + (star.y / star.pz) * cy);

                if ((px !== sx || py !== sy) && star.z < 650) {
                    root.drawLine(px, py, sx, sy, col);
                } else {
                    canvas.setPixelColor(sx, sy, col);
                }
            }
        }
    }
}
