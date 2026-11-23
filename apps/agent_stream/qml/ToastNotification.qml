pragma ComponentBehavior: Bound

import QmlTui

Rectangle {
    id: toast

    property string message: ""
    property string status: "success"
    property int displayDuration: 3500

    function notify(msg: string, stat: string): void {
        toast.message = msg;
        if (stat.length > 0)
            toast.status = stat;
        toast.visible = true;
        dismissTimer.restart();
    }

    width: Math.min(parent ? Math.max(1, parent.width - 2) : 60, toastRow.implicitWidth)
    height: 1
    color: toastMouse.containsMouse ? Theme.pressedBg : Theme.hoverBg
    visible: false

    Timer {
        id: dismissTimer
        interval: toast.displayDuration
        repeat: false
        onTriggered: toast.visible = false
    }

    Row {
        id: toastRow
        anchors.fill: parent
        spacing: 1

        StatusBadge {
            text: toast.status
            status: toast.status
            variant: "subtle"
            showIcon: false
        }

        Text {
            text: toast.message
            color: "white"
            bold: true
        }
    }

    MouseArea {
        id: toastMouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: toast.visible = false
    }
}
