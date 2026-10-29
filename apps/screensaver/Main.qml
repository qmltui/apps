import QtQml
import QmlTui

Terminal {
    id: rootWindow
    color: "#000000"

    property int currentMode: 0
    readonly property var modeNames: [
        qsTr("STARFIELD"),
        qsTr("MYSTIFY"),
        qsTr("DOOM FIRE"),
        qsTr("3D TESSERACT"),
        qsTr("PLASMA"),
        qsTr("MATRIX RAIN"),
        qsTr("3D PIPES"),
        qsTr("ASCII DONUT"),
        qsTr("GAME OF LIFE"),
        qsTr("DVD LOGO"),
        qsTr("LORENZ ATTRACTOR")
    ]

    property int themeIndex: 0
    readonly property var themes: ["cyber", "emerald", "amber", "cga"]
    readonly property string currentTheme: themes[themeIndex]

    property real speedMultiplier: 1.0
    property bool isPaused: false
    property bool showHud: true

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || event.key === Qt.Key_Escape) {
            Qt.quit()
            event.accepted = true
        } else if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
            currentMode = event.key - Qt.Key_1
            event.accepted = true
        } else if (event.key === Qt.Key_0) {
            currentMode = 9
            event.accepted = true
        } else if (event.key === Qt.Key_L) {
            currentMode = 10
            event.accepted = true
        } else if (event.key === Qt.Key_Tab) {
            currentMode = (currentMode + 1) % modeNames.length
            event.accepted = true
        } else if (event.key === Qt.Key_Backtab) {
            currentMode = (currentMode + modeNames.length - 1) % modeNames.length
            event.accepted = true
        } else if (event.key === Qt.Key_Space) {
            isPaused = !isPaused
            event.accepted = true
        } else if (event.key === Qt.Key_Plus || event.key === Qt.Key_Equal) {
            speedMultiplier = Math.min(3.0, speedMultiplier + 0.25)
            event.accepted = true
        } else if (event.key === Qt.Key_Minus || event.key === Qt.Key_Underscore) {
            speedMultiplier = Math.max(0.25, speedMultiplier - 0.25)
            event.accepted = true
        } else if (event.key === Qt.Key_C) {
            themeIndex = (themeIndex + 1) % themes.length
            event.accepted = true
        } else if (event.key === Qt.Key_H) {
            showHud = !showHud
            event.accepted = true
        }
    }

    Item {
        anchors.fill: parent

        // Top Header
        Rectangle {
            id: header
            width: parent.width
            height: rootWindow.showHud ? 4 : 0
            visible: rootWindow.showHud
            color: "#050b14"
            border.style: Border.Single
            border.color: "#0284c7"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.top: parent.top
                text: parent.width < 80
                      ? qsTr("RETRO SCREENSAVER")
                      : qsTr("RETRO SCREENSAVER // DEMOSCENE SHOWCASE")
                color: "#38bdf8"
                bold: true
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 1
                anchors.top: parent.top
                text: qsTr("MODE: %1").arg(rootWindow.modeNames[rootWindow.currentMode])
                color: "#f43f5e"
                bold: true
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.bottom: parent.bottom
                text: parent.width < 90
                      ? qsTr("[1-0,L] SELECT · TAB: CYCLE (%1/%2)").arg(rootWindow.currentMode + 1).arg(rootWindow.modeNames.length)
                      : qsTr("[1]STAR [2]MYST [3]FIRE [4]CUBE [5]PLAS [6]RAIN [7]PIPE [8]DONUT [9]LIFE [0]DVD [L]LRNZ")
                color: "#94a3b8"
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 1
                anchors.bottom: parent.bottom
                text: parent.width < 75
                      ? qsTr("%1 · %2").arg(rootWindow.currentTheme.toUpperCase()).arg(rootWindow.isPaused ? qsTr("PAUSED") : qsTr("RUN"))
                      : qsTr("%1 · %2x · %3")
                            .arg(rootWindow.currentTheme.toUpperCase())
                            .arg(rootWindow.speedMultiplier.toFixed(2))
                            .arg(rootWindow.isPaused ? qsTr("PAUSED") : qsTr("RUNNING"))
                color: rootWindow.isPaused ? "#f59e0b" : "#4ade80"
                bold: true
            }
        }

        // Screensaver Canvas Area
        Rectangle {
            id: displayArea
            width: parent.width
            height: parent.height - (rootWindow.showHud ? (header.height + footer.height) : 0)
            y: rootWindow.showHud ? header.height : 0
            color: "#000000"
            border.style: rootWindow.showHud ? Border.Single : Border.None
            border.color: "#1e293b"

            Item {
                anchors.fill: parent

                StarfieldSaver {
                    visible: rootWindow.currentMode === 0
                    active: rootWindow.currentMode === 0
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                MystifySaver {
                    visible: rootWindow.currentMode === 1
                    active: rootWindow.currentMode === 1
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                DoomFireSaver {
                    visible: rootWindow.currentMode === 2
                    active: rootWindow.currentMode === 2
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                Wireframe3DSaver {
                    visible: rootWindow.currentMode === 3
                    active: rootWindow.currentMode === 3
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                PlasmaSaver {
                    visible: rootWindow.currentMode === 4
                    active: rootWindow.currentMode === 4
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                MatrixRainSaver {
                    visible: rootWindow.currentMode === 5
                    active: rootWindow.currentMode === 5
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                PipesSaver {
                    visible: rootWindow.currentMode === 6
                    active: rootWindow.currentMode === 6
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                DonutSaver {
                    visible: rootWindow.currentMode === 7
                    active: rootWindow.currentMode === 7
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                LifeSaver {
                    visible: rootWindow.currentMode === 8
                    active: rootWindow.currentMode === 8
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                DvdSaver {
                    visible: rootWindow.currentMode === 9
                    active: rootWindow.currentMode === 9
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }

                LorenzSaver {
                    visible: rootWindow.currentMode === 10
                    active: rootWindow.currentMode === 10
                    paused: rootWindow.isPaused
                    speedMultiplier: rootWindow.speedMultiplier
                    theme: rootWindow.currentTheme
                }
            }
        }

        // Bottom Footer
        Rectangle {
            id: footer
            width: parent.width
            height: rootWindow.showHud ? 1 : 0
            y: parent.height - height
            visible: rootWindow.showHud
            color: "#050b14"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.verticalCenter: parent.verticalCenter
                text: parent.width < 75
                      ? qsTr("1-0,L: MODE · TAB · SPACE · +/- · C · H · Q")
                      : qsTr("1-0, L: MODE  ·  TAB: CYCLE  ·  SPACE: PAUSE  ·  +/-: SPD  ·  C: THEME  ·  H: HUD  ·  Q: QUIT")
                color: "#64748b"
            }
        }
    }
}
