import QtQml
import QmlTui
import Screensaver

LifeItem {
    id: root
    anchors.fill: parent

    Timer {
        id: frameTimer
        interval: 66
        running: root.active && !root.paused
        repeat: true
        onTriggered: root.stepLife()
    }
}
