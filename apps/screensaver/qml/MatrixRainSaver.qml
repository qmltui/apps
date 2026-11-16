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

    property var columns: []
    property int currentWidth: 0

    function initColumns() {
        let pw = canvas.pixelWidth;
        let ph = canvas.pixelHeight;
        if (pw <= 0 || ph <= 0)
            return;
        currentWidth = pw;
        let cols = [];
        for (let x = 0; x < pw; ++x) {
            cols.push({
                y: Math.random() * ph * 1.5 - ph * 0.5,
                speed: 0.6 + Math.random() * 1.4,
                length: Math.floor(10 + Math.random() * 22)
            });
        }
        columns = cols;
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.HalfBlock
        autoClear: false
        fillColor: "transparent"

        onPixelWidthChanged: {
            if (pixelWidth > 0 && pixelWidth !== root.currentWidth)
                root.initColumns();
        }
    }

    Component.onCompleted: initColumns()

    function getPalette() {
        if (theme === "amber") {
            return {
                head: "#ffffff",
                lead: "#fef08a",
                body: ["#fde047", "#facc15", "#eab308", "#ca8a04", "#a16207", "#854d0e"],
                tail: ["#713f12", "#451a03", "#260e00"]
            };
        } else if (theme === "cga") {
            return {
                head: "#ffffff",
                lead: "#55ffff",
                body: ["#55ffff", "#00aaaa", "#00aaaa", "#008888", "#006666", "#005555"],
                tail: ["#004444", "#003333", "#002222"]
            };
        } else if (theme === "cyber") {
            return {
                head: "#ffffff",
                lead: "#a5f3fc",
                body: ["#67e8f9", "#22d3ee", "#06b6d4", "#0891b2", "#0e7490", "#155e75"],
                tail: ["#164e63", "#082f49", "#021526"]
            };
        } else { // emerald (classic matrix)
            return {
                head: "#ffffff",
                lead: "#bbf7d0",
                body: ["#86efac", "#4ade80", "#22c55e", "#16a34a", "#15803d", "#166534"],
                tail: ["#14532d", "#064e3b", "#022c22"]
            };
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
            if (pw <= 0 || ph <= 0 || root.columns.length === 0)
                return;
            canvas.clear();
            let pal = root.getPalette();
            let speedMod = root.speedMultiplier;

            for (let x = 0; x < pw; ++x) {
                let col = root.columns[x];
                if (!col)
                    continue;
                col.y += col.speed * speedMod;
                let headY = Math.round(col.y);

                // If entire trail has passed bottom of screen, reset
                if (headY - col.length > ph) {
                    col.y = -Math.random() * 20;
                    col.speed = 0.6 + Math.random() * 1.4;
                    col.length = Math.floor(10 + Math.random() * 22);
                    continue;
                }

                // Draw drop trail upwards from head
                for (let i = 0; i < col.length; ++i) {
                    let py = headY - i;
                    if (py < 0 || py >= ph)
                        continue;
                    let color;
                    if (i === 0) {
                        color = pal.head;
                    } else if (i === 1) {
                        color = pal.lead;
                    } else if (i < col.length - 3) {
                        let bodyIdx = Math.min(pal.body.length - 1, Math.floor((i - 2) * pal.body.length / (col.length - 5)));
                        color = pal.body[bodyIdx];
                    } else {
                        let tailIdx = Math.min(pal.tail.length - 1, i - (col.length - 3));
                        color = pal.tail[tailIdx];
                    }

                    canvas.setPixelColor(x, py, color);
                }
            }
        }
    }
}
