pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Rectangle {
    id: root

    property int currentTab: PlcTypes.Dashboard
    property int alarmCount: 0

    signal tabSelected(int tab)

    height: 1
    color: HmiTheme.recessedBg

    Row {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: 4

            delegate: Rectangle {
                id: tabItem

                required property int index

                readonly property bool active: root.currentTab === index
                readonly property bool hasFaultAlert: index === PlcTypes.Alarms && root.alarmCount > 0 && !active
                readonly property int baseWidth: Math.floor(root.width / 4)

                width: index === 3 ? (root.width - baseWidth * 3) : baseWidth
                height: 1
                color: active ? HmiTheme.borderActive : (hasFaultAlert ? HmiTheme.headerBgError : HmiTheme.panelAltBg)

                Text {
                    anchors.left: parent.left
                    visible: tabItem.index > 0 && !tabItem.active
                    text: "│"
                    color: HmiTheme.borderSubtle
                }

                Text {
                    anchors.centerIn: parent
                    text: {
                        const prefix = tabItem.active ? "▸ " : "  ";
                        if (tabItem.index === 0)
                            return prefix + "[F1] DASHBOARD";
                        if (tabItem.index === 1)
                            return prefix + "[F2] ALARMS (" + root.alarmCount + ")";
                        if (tabItem.index === 2)
                            return prefix + "[F3] DIAGNOSTICS";
                        return prefix + "[F4] SETTINGS";
                    }
                    color: tabItem.active ? HmiTheme.badgeInk : (tabItem.hasFaultAlert ? HmiTheme.textDanger : HmiTheme.textSecondary)
                    bold: tabItem.active || tabItem.hasFaultAlert
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.tabSelected(tabItem.index)
                }
            }
        }
    }
}
