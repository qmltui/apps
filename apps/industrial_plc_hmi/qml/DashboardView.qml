pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Item {
    id: root

    property int machineState: PlcTypes.Idle
    property int traceMode: PlcTypes.AllTraces
    property int speed: 0
    property int targetSpeed: 1500
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property double pressure: 1.0
    property double peakTemperature: 28.5
    property double busVoltage: 24.1
    property double sensorLoopMa: 5.7
    property int cycles: 0
    property int errorCount: 0
    property int pulseFrame: 0
    property bool valveA: false
    property bool valveB: false
    property var tempHistory: []
    property var speedHistory: []
    property var pressureHistory: []

    readonly property bool tallDashboard: height >= 26
    readonly property int leftColWidth: Math.max(25, Math.floor(width * 0.27))
    readonly property int rightColWidth: Math.max(25, Math.floor(width * 0.25))
    readonly property int mimicHeight: tallDashboard ? 10 : 7
    readonly property bool hasInputFocus: controlsPanel.hasInputFocus

    signal targetSpeedRequested(string input)
    signal tempLimitRequested(string input)
    signal startRequested
    signal stopRequested
    signal resetRequested
    signal maintRequested
    signal traceCycleRequested

    anchors.fill: parent

    TelemetryPanel {
        id: telemetryPanel
        width: root.leftColWidth
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        machineState: root.machineState
        speed: root.speed
        temperature: root.temperature
        maxTempLimit: root.maxTempLimit
        pressure: root.pressure
        peakTemperature: root.peakTemperature
        busVoltage: root.busVoltage
        sensorLoopMa: root.sensorLoopMa
        cycles: root.cycles
        errorCount: root.errorCount
        pulseFrame: root.pulseFrame
        valveA: root.valveA
        valveB: root.valveB
        tallLayout: root.tallDashboard
        speedHistory: root.speedHistory
        tempHistory: root.tempHistory
        pressureHistory: root.pressureHistory
    }

    Column {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: telemetryPanel.right
        anchors.right: controlsPanel.left
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        TrendScopePanel {
            width: parent.width
            height: Math.max(10, parent.height - root.mimicHeight - 1)
            machineState: root.machineState
            traceMode: root.traceMode
            temperature: root.temperature
            maxTempLimit: root.maxTempLimit
            tempHistory: root.tempHistory
            speedHistory: root.speedHistory
            onTraceCycleRequested: root.traceCycleRequested()
        }

        ProcessMimicPanel {
            width: parent.width
            height: root.mimicHeight
            machineState: root.machineState
            speed: root.speed
            temperature: root.temperature
            maxTempLimit: root.maxTempLimit
            pressure: root.pressure
            valveA: root.valveA
            valveB: root.valveB
            pulseFrame: root.pulseFrame
        }
    }

    ControlsPanel {
        id: controlsPanel
        width: root.rightColWidth
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        machineState: root.machineState
        targetSpeed: root.targetSpeed
        maxTempLimit: root.maxTempLimit
        tallLayout: root.tallDashboard
        onTargetSpeedRequested: input => root.targetSpeedRequested(input)
        onTempLimitRequested: input => root.tempLimitRequested(input)
        onStartRequested: root.startRequested()
        onStopRequested: root.stopRequested()
        onResetRequested: root.resetRequested()
        onMaintRequested: root.maintRequested()
    }
}
