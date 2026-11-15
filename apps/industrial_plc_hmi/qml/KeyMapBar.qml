pragma ComponentBehavior: Bound

import QmlTui
import IndustrialPlcHmi

Rectangle {
    id: root

    property int machineState: PlcTypes.Idle
    property int currentTab: PlcTypes.Dashboard
    property int traceMode: PlcTypes.AllTraces

    readonly property bool compact: width < 100
    readonly property bool narrow: width < 86

    signal startRequested
    signal stopRequested
    signal resetRequested
    signal maintRequested
    signal traceCycleRequested
    signal skinCycleRequested
    signal quitRequested

    height: 1
    color: HmiTheme.footerBg

    Row {
        anchors.centerIn: parent
        spacing: root.compact ? 1 : 2

        Text {
            visible: !root.narrow
            text: "KEYS:"
            color: HmiTheme.textDim
            bold: true
        }

        KeyHint {
            key: "[F1-F4]"
            label: "Views"
            activeColor: HmiTheme.textHighlight
        }

        KeyHint {
            key: "[S]"
            label: "Start"
            active: root.machineState === PlcTypes.Processing
            activeColor: HmiTheme.textSuccess
            onActivated: root.startRequested()
        }

        KeyHint {
            key: "[X]"
            label: "Stop"
            active: root.machineState === PlcTypes.Idle
            activeColor: HmiTheme.textSecondary
            onActivated: root.stopRequested()
        }

        KeyHint {
            key: "[R]"
            label: "Reset"
            active: root.machineState === PlcTypes.Error
            activeColor: HmiTheme.textDanger
            onActivated: root.resetRequested()
        }

        KeyHint {
            key: "[M]"
            label: "Maint"
            active: root.machineState === PlcTypes.Maintenance
            activeColor: HmiTheme.textWarning
            onActivated: root.maintRequested()
        }

        KeyHint {
            visible: !root.narrow && root.currentTab === PlcTypes.Dashboard
            key: "[T]"
            label: root.traceMode === PlcTypes.AllTraces ? "All" : (root.traceMode === PlcTypes.TempOnly ? "Temp" : "RPM")
            active: root.traceMode !== PlcTypes.AllTraces
            activeColor: HmiTheme.traceTemp
            onActivated: root.traceCycleRequested()
        }

        KeyHint {
            visible: !root.compact
            key: "[K]"
            label: HmiTheme.skinName
            active: HmiTheme.skinIndex !== 0
            activeColor: HmiTheme.textHighlight
            onActivated: root.skinCycleRequested()
        }

        KeyHint {
            key: "[Q]"
            label: "Quit"
            activeColor: HmiTheme.textDanger
            onActivated: root.quitRequested()
        }
    }
}
