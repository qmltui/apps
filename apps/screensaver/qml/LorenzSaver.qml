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

    property real curX: 0.1
    property real curY: 0.0
    property real curZ: 0.0
    property real rotAngle: 0.0

    readonly property int maxTrail: 400
    property var trail: []

    function initLorenz() {
        curX = 0.1;
        curY = 0.0;
        curZ = 0.0;
        rotAngle = 0.0;

        let pts = [];
        let x = 0.1, y = 0.0, z = 0.0;
        let sigma = 10.0, rho = 28.0, beta = 8.0 / 3.0;
        let dt = 0.012;

        for (let i = 0; i < maxTrail; ++i) {
            let dx = sigma * (y - x) * dt;
            let dy = (x * (rho - z) - y) * dt;
            let dz = (x * y - beta * z) * dt;
            x += dx;
            y += dy;
            z += dz;
            pts.push({
                x: x,
                y: y,
                z: z
            });
        }
        curX = x;
        curY = y;
        curZ = z;
        trail = pts;
    }

    Component.onCompleted: initLorenz()

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.Braille
        autoClear: false
        fillColor: "transparent"

        onPixelWidthChanged: {
            if (pixelWidth > 0 && root.trail.length === 0)
                root.initLorenz();
        }
    }

    function getColors() {
        if (theme === "amber") {
            return {
                head: "#ffffff",
                high: "#fef08a",
                mid: "#f59e0b",
                low: "#b45309",
                deep: "#78350f"
            };
        } else if (theme === "emerald") {
            return {
                head: "#ffffff",
                high: "#86efac",
                mid: "#10b981",
                low: "#047857",
                deep: "#064e3b"
            };
        } else if (theme === "cga") {
            return {
                head: "#ffffff",
                high: "#55ffff",
                mid: "#ff55ff",
                low: "#00aaaa",
                deep: "#005555"
            };
        } else { // cyber
            return {
                head: "#ffffff",
                high: "#a5f3fc",
                mid: "#38bdf8",
                low: "#7c3aed",
                deep: "#1e1b4b"
            };
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
            if (pw <= 0 || ph <= 0 || root.trail.length === 0)
                return;
            let spd = root.speedMultiplier;
            let sigma = 10.0, rho = 28.0, beta = 8.0 / 3.0;
            let dt = 0.012 * spd;

            // Step equations 3 times per frame for fluid curves
            for (let s = 0; s < 3; ++s) {
                let dx = sigma * (root.curY - root.curX) * dt;
                let dy = (root.curX * (rho - root.curZ) - root.curY) * dt;
                let dz = (root.curX * root.curY - beta * root.curZ) * dt;
                root.curX += dx;
                root.curY += dy;
                root.curZ += dz;
                root.trail.push({
                    x: root.curX,
                    y: root.curY,
                    z: root.curZ
                });
                if (root.trail.length > root.maxTrail)
                    root.trail.shift();
            }

            root.rotAngle += 0.02 * spd;

            canvas.clear();

            let cx = pw * 0.5;
            let cy = ph * 0.52;
            let rad = root.rotAngle;
            let cosA = Math.cos(rad);
            let sinA = Math.sin(rad);
            let scale = Math.min(pw * 0.024, ph * 0.02);
            let cols = root.getColors();

            let prevPx = -1, prevPy = -1;
            let total = root.trail.length;

            for (let i = 0; i < total; ++i) {
                let pt = root.trail[i];

                // Rotate around Y-axis (center Z around 25)
                let cz = pt.z - 25.0;
                let rx = pt.x * cosA - cz * sinA;
                let rz = pt.x * sinA + cz * cosA;
                let ry = pt.y;

                // Perspective scaling
                let dist = 55.0;
                let fov = dist / (dist + rz * 0.6);
                let px = Math.round(cx + rx * scale * fov * 2.2);
                let py = Math.round(cy - (ry - 2.0) * scale * fov);

                if (prevPx >= 0 && prevPy >= 0) {
                    let ratio = i / total;
                    let col;
                    if (ratio > 0.85)
                        col = cols.high;
                    else if (ratio > 0.55)
                        col = cols.mid;
                    else if (ratio > 0.25)
                        col = cols.low;
                    else
                        col = cols.deep;

                    root.drawLine(prevPx, prevPy, px, py, col);
                }

                prevPx = px;
                prevPy = py;

                // Bright glowing head
                if (i === total - 1) {
                    if (px >= 0 && px < pw && py >= 0 && py < ph) {
                        canvas.setPixelColor(px, py, cols.head);
                        if (px + 1 < pw)
                            canvas.setPixelColor(px + 1, py, cols.head);
                        if (py + 1 < ph)
                            canvas.setPixelColor(px, py + 1, cols.head);
                    }
                }
            }
        }
    }
}
