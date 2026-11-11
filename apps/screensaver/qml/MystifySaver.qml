import QtQml
import QmlTui

Item {
    id: root
    anchors.fill: parent

    property bool active: true
    property bool paused: false
    property real speedMultiplier: 1.0
    property string theme: "cyber"

    readonly property int maxHistory: 14

    property var poly1: []
    property var poly2: []
    property var history1: []
    property var history2: []

    function initPolygons() {
        let pw = Math.max(40, canvas.pixelWidth)
        let ph = Math.max(30, canvas.pixelHeight)

        poly1 = [
            { x: pw * 0.2, y: ph * 0.2, vx: 1.8, vy: 1.2 },
            { x: pw * 0.8, y: ph * 0.3, vx: -1.4, vy: 1.9 },
            { x: pw * 0.7, y: ph * 0.8, vx: -1.7, vy: -1.5 },
            { x: pw * 0.3, y: ph * 0.7, vx: 1.3, vy: -1.8 }
        ]

        poly2 = [
            { x: pw * 0.4, y: ph * 0.1, vx: -1.5, vy: 1.6 },
            { x: pw * 0.9, y: ph * 0.5, vx: -1.8, vy: -1.2 },
            { x: pw * 0.5, y: ph * 0.9, vx: 1.4, vy: -1.7 },
            { x: pw * 0.1, y: ph * 0.4, vx: 1.9, vy: 1.4 }
        ]

        history1 = []
        history2 = []
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.HalfBlock
        autoClear: false
        fillColor: "transparent"

        onPixelWidthChanged: {
            if (pixelWidth > 0 && poly1.length === 0)
                initPolygons()
        }
    }

    Component.onCompleted: initPolygons()

    function drawLine(x0, y0, x1, y1, color) {
        let dx = Math.abs(x1 - x0), sx = x0 < x1 ? 1 : -1
        let dy = -Math.abs(y1 - y0), sy = y0 < y1 ? 1 : -1
        let err = dx + dy
        let x = x0, y = y0
        let w = canvas.pixelWidth
        let h = canvas.pixelHeight
        while (true) {
            if (x >= 0 && x < w && y >= 0 && y < h)
                canvas.setPixelColor(x, y, color)
            if (x === x1 && y === y1) break
            let e2 = 2 * err
            if (e2 >= dy) { err += dy; x += sx }
            if (e2 <= dx) { err += dx; y += sy }
        }
    }

    function updatePoly(poly, w, h, speed) {
        for (let i = 0; i < poly.length; ++i) {
            let pt = poly[i]
            pt.x += pt.vx * speed
            pt.y += pt.vy * speed

            if (pt.x <= 0) {
                pt.x = 0
                pt.vx = Math.abs(pt.vx) + (Math.random() - 0.5) * 0.4
            } else if (pt.x >= w - 1) {
                pt.x = w - 1
                pt.vx = -Math.abs(pt.vx) + (Math.random() - 0.5) * 0.4
            }

            if (pt.y <= 0) {
                pt.y = 0
                pt.vy = Math.abs(pt.vy) + (Math.random() - 0.5) * 0.4
            } else if (pt.y >= h - 1) {
                pt.y = h - 1
                pt.vy = -Math.abs(pt.vy) + (Math.random() - 0.5) * 0.4
            }

            // Clamp velocity
            pt.vx = Math.max(-3.5, Math.min(3.5, pt.vx))
            pt.vy = Math.max(-3.5, Math.min(3.5, pt.vy))
            if (Math.abs(pt.vx) < 0.6) pt.vx = pt.vx < 0 ? -1.2 : 1.2
            if (Math.abs(pt.vy) < 0.6) pt.vy = pt.vy < 0 ? -1.2 : 1.2
        }
    }

    function getFadeColor(polyIndex, step, total) {
        let ratio = (step + 1) / total // 0.0 to 1.0
        if (theme === "amber") {
            if (polyIndex === 0) {
                if (ratio > 0.8) return "#ffffff"
                if (ratio > 0.5) return "#fde047"
                if (ratio > 0.25) return "#ca8a04"
                return "#713f12"
            } else {
                if (ratio > 0.8) return "#fed7aa"
                if (ratio > 0.5) return "#f97316"
                if (ratio > 0.25) return "#c2410c"
                return "#431407"
            }
        } else if (theme === "emerald") {
            if (polyIndex === 0) {
                if (ratio > 0.8) return "#ffffff"
                if (ratio > 0.5) return "#4ade80"
                if (ratio > 0.25) return "#16a34a"
                return "#14532d"
            } else {
                if (ratio > 0.8) return "#a7f3d0"
                if (ratio > 0.5) return "#2dd4bf"
                if (ratio > 0.25) return "#0f766e"
                return "#134e4a"
            }
        } else if (theme === "cga") {
            if (polyIndex === 0) {
                if (ratio > 0.7) return "#55ffff"
                if (ratio > 0.3) return "#00aaaa"
                return "#005555"
            } else {
                if (ratio > 0.7) return "#ff55ff"
                if (ratio > 0.3) return "#aa00aa"
                return "#550055"
            }
        } else { // cyber
            if (polyIndex === 0) {
                if (ratio > 0.85) return "#ffffff"
                if (ratio > 0.6) return "#22d3ee"
                if (ratio > 0.3) return "#0891b2"
                return "#164e63"
            } else {
                if (ratio > 0.85) return "#ffffff"
                if (ratio > 0.6) return "#f43f5e"
                if (ratio > 0.3) return "#be123c"
                return "#4c0519"
            }
        }
    }

    Timer {
        id: frameTimer
        interval: 33
        running: root.active && !root.paused
        repeat: true
        onTriggered: {
            let pw = canvas.pixelWidth
            let ph = canvas.pixelHeight
            if (pw <= 0 || ph <= 0 || poly1.length === 0)
                return

            canvas.clear()

            let speed = 1.0 * root.speedMultiplier
            updatePoly(poly1, pw, ph, speed)
            updatePoly(poly2, pw, ph, speed)

            // Clone points for history
            let snap1 = poly1.map(p => ({ x: Math.round(p.x), y: Math.round(p.y) }))
            let snap2 = poly2.map(p => ({ x: Math.round(p.x), y: Math.round(p.y) }))

            history1.push(snap1)
            if (history1.length > maxHistory) history1.shift()

            history2.push(snap2)
            if (history2.length > maxHistory) history2.shift()

            // Draw history trails
            for (let h = 0; h < history1.length; ++h) {
                let pts = history1[h]
                let col = getFadeColor(0, h, history1.length)
                for (let i = 0; i < pts.length; ++i) {
                    let next = pts[(i + 1) % pts.length]
                    drawLine(pts[i].x, pts[i].y, next.x, next.y, col)
                }
            }

            for (let h = 0; h < history2.length; ++h) {
                let pts = history2[h]
                let col = getFadeColor(1, h, history2.length)
                for (let i = 0; i < pts.length; ++i) {
                    let next = pts[(i + 1) % pts.length]
                    drawLine(pts[i].x, pts[i].y, next.x, next.y, col)
                }
            }
        }
    }
}
