/** @param {Runner} app */
function main(app) {
    app.expectText("Ask anything");
    app.expectScrollbackText("QmlTui");
    app.expectScrollbackText("agent stream demo");
    app.marker("Agent Stream Ready");
    app.sleep(800);

    // 1. Open command autocomplete palette
    app.marker("Command Palette");
    app.typeText("/");
    app.expectText("Commands");
    app.sleep(600);
    app.press("Down");
    app.sleep(400);
    app.press("Down");
    app.sleep(500);
    app.press("Escape");
    app.press("C", ["Ctrl"]);
    app.expectText("Ask anything");
    app.sleep(400);

    // 2. Open Model Picker modal (F2)
    app.marker("Model Selector");
    app.press("F2");
    app.expectText("Neural model selector");
    app.sleep(700);
    app.press("Down");
    app.sleep(500);
    app.press("Enter");
    app.expectText("Ask anything");
    app.sleep(500);

    // 3. Open Diff Inspector modal (F3)
    app.marker("Diff Inspector");
    app.press("F3");
    app.expectText("InlineRenderer.cpp");
    app.sleep(900);
    app.press("Escape");
    app.expectText("Ask anything");
    app.sleep(500);

    // 4. Run streaming agent scenario (/demo 1)
    app.marker("Streaming Agent Execution");
    app.typeText("/demo 1");
    app.sleep(500);
    app.press("Enter");

    // Step through streaming thoughts, tool calls, and diff cards to permission prompt
    for (let i = 0; i < 8; ++i) {
        app.sleep(600);
    }
    app.waitForText("Allow once", 7000);
    app.marker("Permission Approval");
    app.sleep(900);

    // Approve and finish streaming response
    app.press("1");
    for (let i = 0; i < 6; ++i) {
        app.sleep(650);
    }
    app.expectText("Ask anything");
    app.sleep(1000);
    app.stopRecording();
}
