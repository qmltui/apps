import QtQml
import QmlTui

Item {
    id: root
    anchors.fill: parent

    property bool active: true
    property bool paused: false
    property real speedMultiplier: 1.0
    property string theme: "cyber"

    property real angleX: 0.0
    property real angleY: 0.0
    property real angleZ: 0.0

    readonly property var cubeVertices: [
        [-1, -1, -1], [ 1, -1, -1], [ 1,  1, -1], [-1,  1, -1],
        [-1, -1,  1], [ 1, -1,  1], [ 1,  1,  1], [-1,  1,  1]
    ]

    readonly property var cubeEdges: [
        [0, 1], [1, 2], [2, 3], [3, 0], // front/back faces
        [4, 5], [5, 6], [6, 7], [7, 4],
        [0, 4], [1, 5], [2, 6], [3, 7]  // connecting edges
    ]

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.HalfBlock
        autoClear: false
        fillColor: "transparent"
    }

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

    function projectVertex(v, scaleFactor, ax, ay, az, cx, cy, radius) {
        let x = v[0] * scaleFactor
        let y = v[1] * scaleFactor
        let z = v[2] * scaleFactor

        // Rotate X
        let radX = ax
        let y1 = y * Math.cos(radX) - z * Math.sin(radX)
        let z1 = y * Math.sin(radX) + z * Math.cos(radX)

        // Rotate Y
        let radY = ay
        let x2 = x * Math.cos(radY) + z1 * Math.sin(radY)
        let z2 = -x * Math.sin(radY) + z1 * Math.cos(radY)

        // Rotate Z
        let radZ = az
        let x3 = x2 * Math.cos(radZ) - y1 * Math.sin(radZ)
        let y3 = x2 * Math.sin(radZ) + y1 * Math.cos(radZ)

        let distance = 3.2
        let fov = distance / (distance + z2)
        return {
            x: Math.round(cx + x3 * fov * radius),
            y: Math.round(cy + y3 * fov * radius),
            z: z2
        }
    }

    function getColors() {
        if (theme === "amber") {
            return { outer: "#f59e0b", inner: "#fef08a", strut: "#b45309", vertex: "#ffffff" }
        } else if (theme === "emerald") {
            return { outer: "#10b981", inner: "#6ee7b7", strut: "#047857", vertex: "#ffffff" }
        } else if (theme === "cga") {
            return { outer: "#55ffff", inner: "#ff55ff", strut: "#ffffff", vertex: "#ffff55" }
        } else { // cyber
            return { outer: "#00ffff", inner: "#ff007f", strut: "#7000ff", vertex: "#ffffff" }
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
            if (pw <= 0 || ph <= 0) return

            let speed = 0.03 * root.speedMultiplier
            root.angleX += speed * 1.1
            root.angleY += speed * 1.4
            root.angleZ += speed * 0.7

            canvas.clear()

            let cx = pw / 2
            let cy = ph / 2
            let radius = Math.min(cx, cy) * 0.75
            let cols = getColors()

            // Project outer cube vertices
            let outerProj = []
            for (let i = 0; i < cubeVertices.length; ++i) {
                outerProj.push(projectVertex(cubeVertices[i], 1.0, root.angleX, root.angleY, root.angleZ, cx, cy, radius))
            }

            // Project inner cube vertices (tesseract nested)
            let innerProj = []
            for (let i = 0; i < cubeVertices.length; ++i) {
                innerProj.push(projectVertex(cubeVertices[i], 0.52, root.angleX, root.angleY, root.angleZ, cx, cy, radius))
            }

            // Draw struts connecting outer and inner vertices (tesseract hypercube edges)
            for (let i = 0; i < 8; ++i) {
                drawLine(outerProj[i].x, outerProj[i].y, innerProj[i].x, innerProj[i].y, cols.strut)
            }

            // Draw outer cube edges
            for (let i = 0; i < cubeEdges.length; ++i) {
                let e = cubeEdges[i]
                drawLine(outerProj[e[0]].x, outerProj[e[0]].y, outerProj[e[1]].x, outerProj[e[1]].y, cols.outer)
            }

            // Draw inner cube edges
            for (let i = 0; i < cubeEdges.length; ++i) {
                let e = cubeEdges[i]
                drawLine(innerProj[e[0]].x, innerProj[e[0]].y, innerProj[e[1]].x, innerProj[e[1]].y, cols.inner)
            }

            // Draw glowing vertex points
            for (let i = 0; i < 8; ++i) {
                canvas.setPixelColor(outerProj[i].x, outerProj[i].y, cols.vertex)
                canvas.setPixelColor(innerProj[i].x, innerProj[i].y, cols.vertex)
            }
        }
    }
}
