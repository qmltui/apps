pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import IndustrialPlcHmi

Terminal {
    id: rootWindow

    color: HmiTheme.canvasBg

    PlcController {
        id: plc
        onActiveTabChanged: {
            if (plc.activeTab === PlcTypes.Alarms)
                alarmsView.focusAuditTable();
            else
                rootWindow.forceActiveFocus();
        }
        onConfirmationRequested: (dialogTitle, dialogMessage, isFault) => {
            confirmDlg.showPrompt(dialogTitle, dialogMessage, isFault);
        }
    }

    Keys.onPressed: event => {
        if (confirmDlg.visible) {
            if (event.key === Qt.Key_Y)
                confirmDlg.accept();
            else if (event.key === Qt.Key_N || event.key === Qt.Key_Escape)
                confirmDlg.reject();
            event.accepted = true;
            return;
        }

        const textFieldFocused = dashboardView.hasInputFocus || settingsView.hasInputFocus;

        if (event.key === Qt.Key_Escape) {
            if (textFieldFocused) {
                rootWindow.focus = true;
            } else {
                Qt.quit();
            }
            event.accepted = true;
            return;
        }

        // Function-key navigation always works
        if (event.key === Qt.Key_F1) {
            plc.activeTab = PlcTypes.Dashboard;
            event.accepted = true;
            return;
        } else if (event.key === Qt.Key_F2) {
            plc.activeTab = PlcTypes.Alarms;
            event.accepted = true;
            return;
        } else if (event.key === Qt.Key_F3) {
            plc.activeTab = PlcTypes.Diagnostics;
            event.accepted = true;
            return;
        } else if (event.key === Qt.Key_F4) {
            plc.activeTab = PlcTypes.Settings;
            event.accepted = true;
            return;
        }

        if (textFieldFocused)
            return;

        if (event.key === Qt.Key_Q) {
            Qt.quit();
            event.accepted = true;
        } else if (event.key === Qt.Key_1) {
            plc.activeTab = PlcTypes.Dashboard;
            event.accepted = true;
        } else if (event.key === Qt.Key_2) {
            plc.activeTab = PlcTypes.Alarms;
            event.accepted = true;
        } else if (event.key === Qt.Key_3) {
            plc.activeTab = PlcTypes.Diagnostics;
            event.accepted = true;
        } else if (event.key === Qt.Key_4) {
            plc.activeTab = PlcTypes.Settings;
            event.accepted = true;
        } else if (event.key === Qt.Key_S) {
            plc.triggerStart();
            event.accepted = true;
        } else if (event.key === Qt.Key_X) {
            plc.triggerStop();
            event.accepted = true;
        } else if (event.key === Qt.Key_R) {
            plc.requestReset();
            event.accepted = true;
        } else if (event.key === Qt.Key_M) {
            plc.requestMaintenance();
            event.accepted = true;
        } else if (event.key === Qt.Key_T) {
            plc.cycleTraceMode();
            event.accepted = true;
        } else if (event.key === Qt.Key_K) {
            HmiTheme.nextSkin();
            event.accepted = true;
        } else if (event.key === Qt.Key_D) {
            plc.restoreDefaults();
            event.accepted = true;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: plc.machineState === PlcTypes.Error ? HmiTheme.bgError : HmiTheme.canvasBg

        // 1. TOP SCADA MASTHEAD (1 row)
        HeaderPanel {
            id: headerPanel
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            machineState: plc.machineState
            stationId: "STN#42"
            cycles: plc.cycles
            alarmBlinking: plc.alarmBlinking
            clockText: plc.clockText
            onSkinCycleRequested: HmiTheme.nextSkin()
        }

        // 2. NAVIGATION TAB STRIP (1 row)
        NavTabBar {
            id: navBar
            anchors.top: headerPanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            currentTab: plc.activeTab
            alarmCount: plc.errorCount
            onTabSelected: tab => {
                plc.activeTab = tab;
            }
        }

        // 3. MAIN PAGE WORKSPACE (F1-F4)
        Item {
            anchors.top: navBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: statusTicker.top

            // PAGE 1 [F1]: SCADA OPERATOR DASHBOARD
            DashboardView {
                id: dashboardView
                visible: plc.activeTab === PlcTypes.Dashboard
                machineState: plc.machineState
                traceMode: plc.traceMode
                speed: plc.speed
                targetSpeed: plc.targetSpeed
                temperature: plc.temperature
                maxTempLimit: plc.maxTempLimit
                pressure: plc.pressure
                peakTemperature: plc.peakTemperature
                busVoltage: plc.busVoltage
                sensorLoopMa: plc.sensorLoopMa
                cycles: plc.cycles
                errorCount: plc.errorCount
                pulseFrame: plc.pulseFrame
                valveA: plc.valveA
                valveB: plc.valveB
                tempHistory: plc.tempHistory
                speedHistory: plc.speedHistory
                pressureHistory: plc.pressureHistory
                onTargetSpeedRequested: input => plc.setTargetSpeed(input)
                onTempLimitRequested: input => plc.setMaxTempLimit(input)
                onStartRequested: plc.triggerStart()
                onStopRequested: plc.triggerStop()
                onResetRequested: plc.requestReset()
                onMaintRequested: plc.requestMaintenance()
                onTraceCycleRequested: plc.cycleTraceMode()
            }

            // PAGE 2 [F2]: ALARMS & AUDIT LOG
            AlarmsView {
                id: alarmsView
                visible: plc.activeTab === PlcTypes.Alarms
                eventRows: plc.eventRows
                errorCount: plc.errorCount
                machineState: plc.machineState
            }

            // PAGE 3 [F3]: PLC RACK DIAGNOSTICS
            DiagnosticsView {
                visible: plc.activeTab === PlcTypes.Diagnostics
                cycles: plc.cycles
                valveA: plc.valveA
                valveB: plc.valveB
                machineState: plc.machineState
                busVoltage: plc.busVoltage
                sensorLoopMa: plc.sensorLoopMa
                busVoltageHistory: plc.busVoltageHistory
                sensorLoopHistory: plc.sensorLoopHistory
                packetRateHistory: plc.packetRateHistory
            }

            // PAGE 4 [F4]: SYSTEM PARAMETERS & CALIBRATION
            SettingsView {
                id: settingsView
                visible: plc.activeTab === PlcTypes.Settings
                targetSpeed: plc.targetSpeed
                maxTempLimit: plc.maxTempLimit
                onTargetSpeedRequested: input => plc.setTargetSpeed(input)
                onTempLimitRequested: input => plc.setMaxTempLimit(input)
                onSkinCycleRequested: HmiTheme.nextSkin()
                onDefaultsRequested: plc.restoreDefaults()
            }
        }

        // 4. STATUS / FAULT ANNUNCIATOR STRIP (1 row)
        StatusAnnunciatorBar {
            id: statusTicker
            anchors.bottom: keyMapBar.top
            anchors.left: parent.left
            anchors.right: parent.right
            machineState: plc.machineState
            alarmBlinking: plc.alarmBlinking
            temperature: plc.temperature
            maxTempLimit: plc.maxTempLimit
            targetSpeed: plc.targetSpeed
            latestStatus: plc.latestStatus
        }

        // 5. RESPONSIVE SHORTCUT KEYMAP BAR (1 row)
        KeyMapBar {
            id: keyMapBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            machineState: plc.machineState
            currentTab: plc.activeTab
            traceMode: plc.traceMode
            onStartRequested: plc.triggerStart()
            onStopRequested: plc.triggerStop()
            onResetRequested: plc.requestReset()
            onMaintRequested: plc.requestMaintenance()
            onTraceCycleRequested: plc.cycleTraceMode()
            onSkinCycleRequested: HmiTheme.nextSkin()
            onQuitRequested: Qt.quit()
        }

        // 6. MODAL SAFETY INTERLOCK POPUP
        ConfirmDialog {
            id: confirmDlg
            onConfirmed: plc.executeConfirmedAction()
            onCancelled: plc.cancelPendingAction()
        }
    }
}
