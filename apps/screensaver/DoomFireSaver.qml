import QtQml
import QmlTui
import Screensaver

DoomFireItem {
    id: root
    anchors.fill: parent

    Timer {
        id: frameTimer
        interval: 33
        running: root.active && !root.paused
        repeat: true
        onTriggered: root.stepFire()
    }
}
