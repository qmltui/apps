/** @param {Runner} app */
function main(app) {
    app.waitForText("DOCKER FLEET MONITOR");
    app.marker("Fleet Overview");
    app.sleep(900);

    // Browse nodes in the fleet table
    app.press("Down");
    app.sleep(500);
    app.press("Down");
    app.sleep(500);
    app.press("Down");
    app.sleep(600);

    // Open live container logs drawer and sort by CPU Load
    app.marker("Container Logs & Sorting");
    app.press("L");
    app.waitForFrame();
    app.sleep(800);
    app.clickText("CPU LOAD");
    app.sleep(800);

    // Filter tabs: Running -> Critical
    app.marker("Filter Running & Critical");
    app.press("2");
    app.waitForText("RUNNING");
    app.sleep(800);

    app.press("3");
    app.waitForText("CRITICAL");
    app.sleep(900);

    // Restart all critical nodes and return to All view
    app.marker("Recover Critical Nodes");
    app.press("A");
    app.sleep(700);
    app.press("1");
    app.sleep(800);

    // Live search filtering
    app.marker("Live Search");
    app.press("/");
    app.sleep(400);
    app.typeText("redis");
    app.sleep(900);
    app.press("Escape");
    app.sleep(900);
    app.stopRecording();
}
