pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls

Panel {
    id: panel

    property bool active: true
    property int animationDuration: 150
    property bool ready: false

    clip: true
    visible: width > 0 && height > 0 && active
    color: "#0b0e0f"
    border.style: Border.Rounded
    border.color: "#56604a"

    Component.onCompleted: ready = true

    Behavior on width {
        NumberAnimation {
            duration: panel.ready ? panel.animationDuration : 0
            easing: Easing.InOutQuad
        }
    }

    Behavior on height {
        NumberAnimation {
            duration: panel.ready ? panel.animationDuration : 0
            easing: Easing.InOutQuad
        }
    }
}
