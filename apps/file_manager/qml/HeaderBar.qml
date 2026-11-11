pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: root

    required property string breadcrumbs
    required property string diskUsage
    property string filterText: ""

    height: 1
    color: Theme.bgHeader
    border.style: Border.None

    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 2
        spacing: 2

        // Path Breadcrumbs
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "📁" + root.breadcrumbs
            color: Theme.accentBlue
            bold: true
        }

        // Active Filter badge (when filtered)
        Text {
            visible: root.filterText !== ""
            anchors.verticalCenter: parent.verticalCenter
            text: "🔍 " + root.filterText
            color: Theme.accentPink
            bold: true
        }
    }

    // Disk Usage
    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 2
        text: root.diskUsage
        color: Theme.textDim
    }
}
