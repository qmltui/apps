/** @param {Runner} app */
function main(app) {
    app.waitForText("[■ STANDBY]");
    app.expectText("STATUS: READY FOR OPERATOR COMMANDS");
    app.marker("Standby Dashboard");
    app.sleep(900);

    // Start station and step simulation telemetry
    app.marker("Start Processing");
    app.press("S");
    app.waitForText("[● RUNNING]");
    for (let i = 0; i < 3; ++i) {
        app.sleep(1000);
    }

    // Enter Maintenance Mode via confirmation modal
    app.marker("Maintenance Mode");
    app.press("M");
    app.waitForText("MAINTENANCE MODE");
    app.sleep(700);
    app.press("Y");
    app.waitForText("[⚙ MAINT]");
    app.sleep(1000);

    // Exit Maintenance Mode
    app.press("M");
    app.waitForText("Exit maintenance mode");
    app.sleep(600);
    app.press("Y");
    app.waitForText("[■ STANDBY]");
    app.sleep(700);

    // Inspect Alarms (F2) and Diagnostics (F3)
    app.marker("Alarms & Diagnostics");
    app.press("F2");
    app.waitForText("ALARM HISTORY");
    app.sleep(800);

    app.press("F3");
    app.waitForText("PLC HARDWARE & SUBSYSTEM DIAGNOSTICS");
    app.sleep(900);

    // Lower temperature fault threshold in Settings (F4)
    app.marker("Safety Threshold & Fault");
    app.press("F4");
    app.waitForText("SYSTEM PARAMETERS & SAFETY THRESHOLDS");
    app.sleep(600);
    app.press("Tab");
    app.press("Tab");
    app.press("A", ["Ctrl"]);
    app.typeText("30");
    app.sleep(400);
    app.press("Enter");
    app.waitForText("LIMIT TEMP -> 30.0C");
    app.sleep(700);

    // Return to Dashboard (F1), trigger start, and observe fault alarm blinking
    app.press("F1");
    app.press("S");
    for (let i = 0; i < 3; ++i) {
        app.sleep(1000);
        if (app.frameText().includes("[▲ FAULT!]")) {
            break;
        }
    }
    app.expectText("[▲ FAULT!]");
    app.sleep(600);
    app.sleep(600);

    // Acknowledge and reset fault
    app.marker("Fault Reset");
    app.press("R");
    app.waitForText("CLEAR FAULT ALARM");
    app.sleep(700);
    app.press("Y");
    app.waitForText("STATE: IDLE (ALARM RESET)");
    app.sleep(1000);
    app.stopRecording();
}
