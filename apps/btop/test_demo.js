/** @param {Runner} app */
function main(app) {
    app.expectText("btop");
    app.expectText("PROCESSES");
    app.expectText("NETWORK");
    app.expectText("MEMORY");
    app.expectText("¹cpu");
    app.marker("System Overview");
    app.sleep(1000);

    // Sort processes by memory, PID, then CPU
    app.marker("Process Sorting");
    app.press("m");
    app.waitForText("sort: memory");
    app.sleep(700);
    app.press("p");
    app.waitForText("sort: pid");
    app.sleep(700);
    app.press("c");
    app.waitForText("sort: cpu");
    app.sleep(700);

    // Animated collapse into CPU focus mode
    app.marker("CPU Focus Mode");
    app.press("2");
    app.sleep(400);
    app.press("3");
    app.sleep(400);
    app.press("4");
    app.sleep(500);
    app.expectText("preset *");
    app.sleep(900);

    // Restore Memory, Network, and Processes panels with smooth Behavior animations
    app.marker("Restore Dashboard Panels");
    app.press("2");
    app.sleep(400);
    app.press("3");
    app.sleep(400);
    app.press("4");
    app.sleep(500);
    app.waitForText("PROCESSES");
    app.sleep(700);

    // Pause and resume telemetry
    app.marker("Pause & Resume");
    app.press("Space");
    app.expectText("PAUSED");
    app.sleep(700);
    app.press("Space");
    app.expectNoText("PAUSED");
    app.sleep(1000);
    app.stopRecording();
}
