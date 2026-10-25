import QtQml
import QmlTui

Item {
    id: root
    anchors.fill: parent

    property bool active: true
    property bool paused: false
    property real speedMultiplier: 1.0
    property string theme: "cyber"

    property real timeVal: 0.0
    property var sinTable: []

    readonly property var cyberPalette: [
        "#0f051d", "#1e0b36", "#371359", "#581c87", "#7e22ce", "#a855f7",
        "#c084fc", "#e879f9", "#f43f5e", "#fb7185", "#38bdf8", "#06b6d4",
        "#22d3ee", "#67e8f9", "#a5f3fc", "#ffffff"
    ]

    readonly property var amberPalette: [
        "#1c0a00", "#381400", "#541e00", "#782800", "#9c3400", "#c2410c",
        "#ea580c", "#f97316", "#fb923c", "#f59e0b", "#fbbf24", "#fcd34d",
        "#fde68a", "#fef08a", "#fefce8", "#ffffff"
    ]

    readonly property var emeraldPalette: [
        "#021b10", "#052e16", "#064e3b", "#065f46", "#047857", "#059669",
        "#10b981", "#34d399", "#6ee7b7", "#a7f3d0", "#bbf7d0", "#dcfce7",
        "#22d3ee", "#67e8f9", "#ecfdf5", "#ffffff"
    ]

    readonly property var cgaPalette: [
        "#000000", "#0000aa", "#00aa00", "#00aaaa", "#aa0000", "#aa00aa",
        "#aa5500", "#aaaaaa", "#555555", "#5555ff", "#55ff55", "#55ffff",
        "#ff5555", "#ff55ff", "#ffff55", "#ffffff"
    ]

    function getPalette() {
        if (theme === "amber") return amberPalette
        if (theme === "emerald") return emeraldPalette
        if (theme === "cga") return cgaPalette
        return cyberPalette
    }

    function initSinTable() {
        let arr = []
        for (let i = 0; i < 512; ++i) {
            arr.push(Math.sin((i / 512) * Math.PI * 2))
        }
        sinTable = arr
    }

    Component.onCompleted: initSinTable()

    Canvas {
        id: canvas
        anchors.fill: parent
        renderMode: Canvas.HalfBlock
        autoClear: false
        fillColor: "transparent"
    }

    Timer {
        id: frameTimer
        interval: 33
        running: root.active && !root.paused
        repeat: true
        onTriggered: {
            let pw = canvas.pixelWidth
            let ph = canvas.pixelHeight
            if (pw <= 0 || ph <= 0 || sinTable.length === 0)
                return

            root.timeVal += 0.05 * root.speedMultiplier
            let t = root.timeVal
            let pal = getPalette()
            let palMask = pal.length - 1

            let cx = pw / 2
            let cy = ph / 2

            // Use step = 2 if resolution is huge (>120 width), else step = 1
            let step = (pw > 140 || ph > 90) ? 2 : 1

            for (let y = 0; y < ph; y += step) {
                let v2 = Math.sin(y * 0.12 - t * 1.3)
                for (let x = 0; x < pw; x += step) {
                    let v1 = Math.sin(x * 0.1 + t)
                    let v3 = Math.sin((x + y) * 0.08 + t * 0.7)
                    let dx = x - cx
                    let dy = y - cy
                    let dist = Math.sqrt(dx * dx + dy * dy)
                    let v4 = Math.sin(dist * 0.12 + t * 1.6)

                    let val = (v1 + v2 + v3 + v4 + 4) * 2.0
                    let idx = Math.floor(val) & palMask
                    let col = pal[idx]

                    if (step === 1) {
                        canvas.setPixelColor(x, y, col)
                    } else {
                        canvas.setPixelColor(x, y, col)
                        if (x + 1 < pw) canvas.setPixelColor(x + 1, y, col)
                        if (y + 1 < ph) canvas.setPixelColor(x, y + 1, col)
                        if (x + 1 < pw && y + 1 < ph) canvas.setPixelColor(x + 1, y + 1, col)
                    }
                }
            }
        }
    }
}
