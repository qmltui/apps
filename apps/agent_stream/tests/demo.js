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

    // 4. Inspect workspace hierarchy in scrollback (/tree)
    app.marker("Workspace Tree");
    app.typeText("/tree");
    app.sleep(350);
    app.press("Enter");
    app.waitForScrollbackText("QmlTui Workspace", 3000);
    app.sleep(700);

    // 5. Run streaming agent scenario (/demo 1)
    app.marker("Streaming Agent Execution");
    app.typeText("/demo 1");
    app.sleep(450);
    app.press("Enter");

    // Step through streaming thoughts, tool calls, and diff cards to permission prompt
    app.waitForText("Allow once", 8000);
    app.marker("Permission Approval");
    app.sleep(900);

    // Approve and finish streaming response
    app.press("1");
    app.waitForText("Ask anything", 8000);
    app.sleep(1000);
    app.stopRecording();
}
