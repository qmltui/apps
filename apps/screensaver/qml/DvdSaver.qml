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

    readonly property int logoWidth: 16
    readonly property int logoHeight: 4

    property real logoX: 3.0
    property real logoY: 2.0
    property real vx: 0.65
    property real vy: 0.38

    property int colorIndex: 0
    property int totalBounces: 0
    property int cornerHits: 0
    property int flashTimer: 0

    property var trails: []

    function getPalette() {
        if (theme === "amber") {
            return ["#f59e0b", "#fbbf24", "#f97316", "#fde047", "#ea580c", "#ffffff"];
        } else if (theme === "emerald") {
            return ["#10b981", "#34d399", "#6ee7b7", "#4ade80", "#22c55e", "#ffffff"];
        } else if (theme === "cga") {
            return ["#55ffff", "#ff55ff", "#ffff55", "#55ff55", "#ff5555", "#ffffff"];
        } else { // cyber
            return ["#38bdf8", "#f43f5e", "#a855f7", "#22d3ee", "#e879f9", "#ffffff"];
        }
    }

    readonly property string currentColor: {
        let pal = getPalette();
        return pal[colorIndex % pal.length];
    }

    Timer {
        id: frameTimer
        interval: 33
        running: root.active && !root.paused
        repeat: true
        onTriggered: {
            let w = root.width;
            let h = root.height;
            if (w <= root.logoWidth + 2 || h <= root.logoHeight + 2)
                return;
            let pal = root.getPalette();
            let speed = 1.0 * root.speedMultiplier;

            let nextX = root.logoX + root.vx * speed;
            let nextY = root.logoY + root.vy * speed;
            let hitX = false;
            let hitY = false;

            if (nextX <= 0) {
                nextX = 0;
                root.vx = Math.abs(root.vx);
                hitX = true;
            } else if (nextX >= w - root.logoWidth) {
                nextX = w - root.logoWidth;
                root.vx = -Math.abs(root.vx);
                hitX = true;
            }

            if (nextY <= 0) {
                nextY = 0;
                root.vy = Math.abs(root.vy);
                hitY = true;
            } else if (nextY >= h - root.logoHeight) {
                nextY = h - root.logoHeight;
                root.vy = -Math.abs(root.vy);
                hitY = true;
            }

            if (hitX || hitY) {
                root.totalBounces++;
                root.colorIndex = (root.colorIndex + 1) % pal.length;
            }

            if (hitX && hitY) {
                root.cornerHits++;
                root.flashTimer = 20;
            }

            if (root.flashTimer > 0)
                root.flashTimer--;

            // Push old position into trails
            let currentSnap = {
                x: Math.round(root.logoX),
                y: Math.round(root.logoY),
                col: root.currentColor
            };
            root.trails.push(currentSnap);
            if (root.trails.length > 3)
                root.trails.shift();

            root.logoX = nextX;
            root.logoY = nextY;
        }
    }

    // Motion Echo Trail 1
    Rectangle {
        visible: root.trails.length >= 2
        x: root.trails.length >= 2 ? root.trails[0].x : 0
        y: root.trails.length >= 2 ? root.trails[0].y : 0
        width: root.logoWidth
        height: root.logoHeight
        border.style: Border.Single
        border.color: "#1e293b"
        color: "transparent"
    }

    // Motion Echo Trail 2
    Rectangle {
        visible: root.trails.length >= 3
        x: root.trails.length >= 3 ? root.trails[1].x : 0
        y: root.trails.length >= 3 ? root.trails[1].y : 0
        width: root.logoWidth
        height: root.logoHeight
        border.style: Border.Single
        border.color: "#334155"
        color: "transparent"
    }

    // Main Bouncing DVD Logo Box
    Rectangle {
        id: dvdBadge
        x: Math.round(root.logoX)
        y: Math.round(root.logoY)
        width: root.logoWidth
        height: root.logoHeight
        border.style: root.flashTimer > 0 ? Border.Rounded : Border.Double
        border.color: root.flashTimer > 0 ? "#ffffff" : root.currentColor
        color: root.flashTimer > 0 ? "#881337" : "#050b14"

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 1
            text: qsTr("📀  D V D")
            color: root.flashTimer > 0 ? "#ffffff" : root.currentColor
            bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 2
            text: qsTr("V I D E O")
            color: root.flashTimer > 0 ? "#ffffff" : root.currentColor
        }
    }

    // Celebratory Corner Hit Banner
    Text {
        anchors.centerIn: parent
        visible: root.flashTimer > 0
        text: qsTr("★ ★ ★  C O R N E R   H I T !  ★ ★ ★")
        color: "#facc15"
        bold: true
    }

    // Subtle Hit Counter
    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 1
        anchors.bottomMargin: 0
        text: qsTr("BOUNCES: %1  ·  CORNER HITS: %2").arg(root.totalBounces).arg(root.cornerHits)
        color: "#475569"
    }
}
