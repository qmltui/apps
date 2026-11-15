pragma ComponentBehavior: Bound

import QtQml
import IndustrialPlcHmi

QtObject {
    id: plc

    property int activeTab: PlcTypes.Dashboard
    property int machineState: PlcTypes.Idle
    property int traceMode: PlcTypes.AllTraces
    property int speed: 0
    property int targetSpeed: 1500
    property double temperature: 28.5
    property double maxTempLimit: 85.0
    property double pressure: 1.0
    property double busVoltage: 24.10
    property double sensorLoopMa: 5.7
    property double peakTemperature: 28.5
    property int cycles: 0
    property int errorCount: 0
    property int pulseFrame: 0
    property bool valveA: false
    property bool valveB: false
    property bool alarmBlinking: false
    property int pendingAction: PlcTypes.NoAction
    property string latestStatus: "READY FOR OPERATOR COMMANDS"
    property string clockText: Qt.formatTime(new Date(), "HH:mm:ss")
    property int nextEventSequence: 4
    property bool deterministicSimulation: false
    property int deterministicNoiseIndex: 0
    property list<real> deterministicNoise: [0.25, 0.75, 0.5, 0.9, 0.1, 0.6]

    readonly property int historyLength: 60
    property var tempHistory: Array(60).fill(28.5)
    property var speedHistory: Array(60).fill(0)
    property var pressureHistory: Array(60).fill(1.0)
    property var busVoltageHistory: Array(40).fill(24.1)
    property var sensorLoopHistory: Array(40).fill(5.7)
    property var packetRateHistory: Array(40).fill(24)

    property var eventRows: [["#03", "00:00:00", "SYS-I003", "[INFO]", "READY FOR OPERATOR COMMANDS"], ["#02", "00:00:00", "PLC-I002", "[INFO]", "STATE CHANGED TO: IDLE"], ["#01", "00:00:00", "SYS-I001", "[INFO]", "SYSTEM INITIALIZED"]]

    signal confirmationRequested(string dialogTitle, string dialogMessage, bool isFault)

    onMachineStateChanged: {
        if (!plc.isKnownMachineState(plc.machineState))
            plc.enterFault("INVALID PLC STATE DETECTED");
    }

    function eventCodeFor(severity: string): string {
        if (severity === "CRITICAL")
            return "ALM-E" + String(plc.errorCount).padStart(3, "0");
        if (severity === "WARNING")
            return "CFG-W" + String(plc.nextEventSequence).padStart(3, "0");
        return "PLC-I" + String(plc.nextEventSequence).padStart(3, "0");
    }

    function logEvent(severity: string, message: string) {
        const seqText = "#" + String(plc.nextEventSequence).padStart(2, "0");
        const stamp = Qt.formatTime(new Date(), "HH:mm:ss");
        const code = plc.eventCodeFor(severity);
        const nextRows = [[seqText, stamp, code, "[" + severity + "]", message]].concat(plc.eventRows);
        if (nextRows.length > 100)
            nextRows.length = 100;
        plc.eventRows = nextRows;
        plc.nextEventSequence += 1;
        plc.latestStatus = message;
    }

    function pushHistory(newTemp: real, newSpeed: real, newPressure: real, newBusV: real, newLoopMa: real, newPkts: real) {
        const nextTemps = plc.tempHistory.slice(1);
        nextTemps.push(newTemp);
        plc.tempHistory = nextTemps;

        const nextSpeeds = plc.speedHistory.slice(1);
        nextSpeeds.push(newSpeed);
        plc.speedHistory = nextSpeeds;

        const nextPressures = plc.pressureHistory.slice(1);
        nextPressures.push(newPressure);
        plc.pressureHistory = nextPressures;

        const nextBus = plc.busVoltageHistory.slice(1);
        nextBus.push(newBusV);
        plc.busVoltageHistory = nextBus;

        const nextLoop = plc.sensorLoopHistory.slice(1);
        nextLoop.push(newLoopMa);
        plc.sensorLoopHistory = nextLoop;

        const nextPkts = plc.packetRateHistory.slice(1);
        nextPkts.push(newPkts);
        plc.packetRateHistory = nextPkts;
    }

    function simulationNoise(minimum: real, maximum: real): real {
        let unitValue = Math.random();
        if (plc.deterministicSimulation) {
            unitValue = plc.deterministicNoise[plc.deterministicNoiseIndex % plc.deterministicNoise.length];
            plc.deterministicNoiseIndex += 1;
        }
        return minimum + unitValue * (maximum - minimum);
    }

    function isKnownMachineState(state: int): bool {
        return state === PlcTypes.Idle || state === PlcTypes.Processing || state === PlcTypes.Error || state === PlcTypes.Maintenance;
    }

    function setTargetSpeed(input: string): bool {
        const trimmed = input.trim();
        const newSpeed = Number(trimmed);
        if (trimmed.length === 0 || !Number.isFinite(newSpeed) || !Number.isInteger(newSpeed) || newSpeed < 0 || newSpeed > 3000) {
            plc.logEvent("WARNING", "REJECTED RPM SETPOINT: " + input);
            return false;
        }
        plc.targetSpeed = newSpeed;
        plc.logEvent("INFO", "SET RPM -> " + newSpeed);
        return true;
    }

    function setMaxTempLimit(input: string): bool {
        const trimmed = input.trim();
        const newLimit = Number(trimmed);
        if (trimmed.length === 0 || !Number.isFinite(newLimit) || newLimit < 30.0 || newLimit > 110.0) {
            plc.logEvent("WARNING", "REJECTED TEMP LIMIT: " + input);
            return false;
        }
        plc.maxTempLimit = newLimit;
        plc.logEvent("INFO", "LIMIT TEMP -> " + newLimit.toFixed(1) + "C");
        return true;
    }

    function restoreDefaults() {
        plc.targetSpeed = 1500;
        plc.maxTempLimit = 85.0;
        plc.logEvent("INFO", "CALIBRATION RESTORED (1500 RPM / 85.0C)");
    }

    function cycleTraceMode() {
        plc.traceMode = (plc.traceMode + 1) % 3;
    }

    function enterFault(message: string) {
        plc.machineState = PlcTypes.Error;
        plc.speed = 0;
        plc.valveA = false;
        plc.valveB = false;
        plc.pendingAction = PlcTypes.NoAction;
        plc.errorCount += 1;
        plc.logEvent("CRITICAL", message);
    }

    function triggerStart() {
        if (plc.machineState !== PlcTypes.Error) {
            plc.machineState = PlcTypes.Processing;
            plc.logEvent("INFO", "STATE: PROCESSING (START CMD)");
        }
    }

    function triggerStop() {
        if (plc.machineState !== PlcTypes.Error) {
            plc.machineState = PlcTypes.Idle;
            plc.valveA = false;
            plc.valveB = false;
            plc.logEvent("INFO", "STATE: IDLE (STOP CMD)");
        }
    }

    function requestReset() {
        if (plc.machineState === PlcTypes.Error) {
            plc.pendingAction = PlcTypes.Reset;
            plc.confirmationRequested("CLEAR FAULT ALARM", "Acknowledge over-temperature fault and release safety lockout?", true);
        }
    }

    function requestMaintenance() {
        if (plc.machineState !== PlcTypes.Error) {
            plc.pendingAction = PlcTypes.ToggleMaintenance;
            plc.confirmationRequested("MAINTENANCE MODE", plc.machineState === PlcTypes.Maintenance ? "Exit maintenance mode and return station to IDLE?" : "Switch PLC station into low-speed 150 RPM MAINTENANCE jog?", false);
        }
    }

    function cancelPendingAction() {
        plc.pendingAction = PlcTypes.NoAction;
    }

    function executeConfirmedAction() {
        if (plc.pendingAction === PlcTypes.Reset) {
            plc.machineState = PlcTypes.Idle;
            plc.temperature = 35.0;
            plc.logEvent("INFO", "STATE: IDLE (ALARM RESET)");
        } else if (plc.pendingAction === PlcTypes.ToggleMaintenance) {
            if (plc.machineState === PlcTypes.Maintenance) {
                plc.machineState = PlcTypes.Idle;
                plc.valveA = false;
                plc.valveB = false;
                plc.logEvent("INFO", "STATE: IDLE (MAINT RUN COMPLETED)");
            } else {
                plc.machineState = PlcTypes.Maintenance;
                plc.speed = 150;
                plc.valveA = true;
                plc.valveB = false;
                plc.logEvent("INFO", "STATE: MAINTENANCE ACTIVE");
            }
        }
        plc.pendingAction = PlcTypes.NoAction;
    }

    readonly property Timer simTimer: Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            plc.clockText = Qt.formatTime(new Date(), "HH:mm:ss");

            if (plc.machineState === PlcTypes.Processing) {
                plc.cycles += 1;

                if (plc.speed < plc.targetSpeed)
                    plc.speed = Math.min(plc.targetSpeed, plc.speed + 200);
                else if (plc.speed > plc.targetSpeed)
                    plc.speed = Math.max(plc.targetSpeed, plc.speed - 200);

                const tempFactor = (plc.speed / 1000.0) * 1.8;
                plc.temperature = Math.min(110.0, plc.temperature + tempFactor + plc.simulationNoise(-0.5, 1.0));

                plc.pressure = 1.0 + (plc.temperature - 25.0) * 0.06 + plc.simulationNoise(-0.05, 0.05);

                plc.valveA = (plc.cycles % 8 < 4);
                plc.valveB = (plc.cycles % 8 >= 4);

                if (plc.temperature >= plc.maxTempLimit)
                    plc.enterFault("OVER-TEMPERATURE FAULT");
            } else if (plc.machineState === PlcTypes.Idle || plc.machineState === PlcTypes.Error) {
                if (plc.speed > 0)
                    plc.speed = Math.max(0, plc.speed - 350);
                if (plc.temperature > 28.5)
                    plc.temperature = Math.max(28.5, plc.temperature - 2.5);
                if (plc.pressure > 1.0)
                    plc.pressure = Math.max(1.0, plc.pressure - 0.3);
                plc.valveA = false;
                plc.valveB = false;
            } else if (plc.machineState === PlcTypes.Maintenance) {
                plc.speed = 150;
                plc.temperature = 31.0;
                plc.pressure = 1.1;
                plc.valveA = true;
                plc.valveB = false;
            }

            if (plc.temperature > plc.peakTemperature)
                plc.peakTemperature = plc.temperature;

            const loadDrop = (plc.speed / 3000.0) * 0.22;
            plc.busVoltage = 24.12 - loadDrop + plc.simulationNoise(-0.06, 0.06);
            plc.sensorLoopMa = 4.0 + Math.max(0.0, Math.min(1.0, (plc.temperature - 20.0) / 90.0)) * 16.0;
            const pktRate = plc.machineState === PlcTypes.Processing ? 95 + plc.simulationNoise(-12, 28) : (plc.machineState === PlcTypes.Maintenance ? 52 : 24 + plc.simulationNoise(-4, 6));

            if (plc.machineState === PlcTypes.Processing && plc.cycles % 5 === 0) {
                plc.logEvent("INFO", "PROCESS CYCLE " + plc.cycles + " | T: " + plc.temperature.toFixed(1) + "C");
            }

            plc.pushHistory(plc.temperature, plc.speed, plc.pressure, plc.busVoltage, plc.sensorLoopMa, pktRate);
        }
    }

    readonly property Timer flowPulseTimer: Timer {
        interval: plc.machineState === PlcTypes.Maintenance ? 450 : 140
        running: plc.activeTab === PlcTypes.Dashboard && (plc.machineState === PlcTypes.Processing || plc.machineState === PlcTypes.Maintenance)
        repeat: true
        onTriggered: plc.pulseFrame = (plc.pulseFrame + 1) % 24
    }

    readonly property Timer blinkTimer: Timer {
        interval: 500
        running: plc.machineState === PlcTypes.Error
        repeat: true
        onTriggered: plc.alarmBlinking = !plc.alarmBlinking
    }
}
