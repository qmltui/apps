/** @param {Runner} app */
function main(app) {
    // Wall-clock sleep() calls below pace the CLOCK REAL Asciinema recording and
    // let PlcController's real QML Timers (simTimer 1000ms, flowPulseTimer 140ms,
    // blinkTimer 500ms) advance telemetry, P&ID pipe flow, and fault blinking.
    app.waitForText("[■ STANDBY]");
    app.expectText("STATUS: READY FOR OPERATOR COMMANDS");
    app.expectText("P&ID MIMIC");
    app.marker("Standby Dashboard");
    app.sleep(800);

    // Start station and step simulation telemetry + P&ID reactor flow
    app.marker("Start Processing");
    app.press("S");
    app.waitForText("[● RUNNING]");
    for (let i = 0; i < 4; ++i) {
        app.sleep(950);
    }

    // Cycle multi-trace Braille trend scope ([T]: TEMP+RPM -> TEMP ONLY -> RPM ONLY)
    app.marker("Braille Scope Traces");
    app.press("T");
    app.waitForText("[T] ●TEMP ONLY");
    app.sleep(550);
    app.press("T");
    app.waitForText("[T] ●RPM ONLY");
    app.sleep(550);
    app.press("T");
    app.waitForText("[T] ●TEMP ●RPM");
    app.sleep(450);

    // Showcase switchable SCADA skins ([K]: ISA-SLATE -> AMBER CRT -> PHOSPHOR -> ISA-SLATE)
    app.marker("SCADA Skins");
    app.press("K");
    app.waitForText("AMBER CRT");
    app.sleep(750);
    app.press("K");
    app.waitForText("PHOSPHOR");
    app.sleep(750);
    app.press("K");
    app.waitForText("ISA-SLATE");
    app.sleep(500);

    // Enter Maintenance Mode via dimmed Popup safety modal
    app.marker("Maintenance Mode");
    app.press("M");
    app.waitForText("MAINTENANCE MODE");
    app.sleep(700);
    app.press("Y");
    app.waitForText("[⚙ MAINT]");
    app.sleep(900);

    // Exit Maintenance Mode
    app.press("M");
    app.waitForText("Exit maintenance mode");
    app.sleep(500);
    app.press("Y");
    app.waitForText("[■ STANDBY]");
    app.sleep(550);

    // Inspect TableView Audit Log (F2) and modular S7-1500 I/O Rack Diagnostics (F3)
    app.marker("Alarms & Diagnostics");
    app.press("F2");
    app.waitForText("ALARM HISTORY");
    app.sleep(850);

    app.press("F3");
    app.waitForText("PLC HARDWARE & SUBSYSTEM DIAGNOSTICS");
    app.sleep(950);

    // Adjust RPM preset and lower temperature fault threshold in Calibration Console (F4)
    app.marker("Safety Threshold & Fault");
    app.press("F4");
    app.waitForText("SYSTEM PARAMETERS & SAFETY THRESHOLDS");
    app.sleep(500);
    app.clickText("2400");
    app.waitForText("SET RPM -> 2400");
    app.sleep(450);
    app.press("Tab");
    app.press("Tab");
    app.press("A", ["Ctrl"]);
    app.typeText("30");
    app.sleep(350);
    app.press("Enter");
    app.waitForText("LIMIT TEMP -> 30.0C");
    app.sleep(600);

    // Return to Dashboard (F1), trigger start, and observe fault interlock trip
    app.press("F1");
    app.press("S");
    app.waitForText("[▲ FAULT!]", 7000);
    app.sleep(1150);

    // Acknowledge fault lockout and restore default calibration
    app.marker("Fault Reset");
    app.press("R");
    app.waitForText("CLEAR FAULT ALARM");
    app.sleep(700);
    app.press("Y");
    app.waitForText("STATE: IDLE (ALARM RESET)");
    app.sleep(600);
    app.press("D");
    app.waitForText("CALIBRATION RESTORED");
    app.sleep(800);
    app.stopRecording();
}
