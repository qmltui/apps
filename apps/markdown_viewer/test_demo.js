/** @param {Runner} app */
function main(app) {
    app.waitForText("Modern Style Showcase");
    app.marker("Modern Showcase");
    app.sleep(800);

    // Jump through headings into tables and syntax-highlighted code
    app.marker("Section Navigation");
    app.press("Tab");
    app.sleep(500);
    app.press("Tab");
    app.sleep(500);
    app.press("Tab");
    app.sleep(500);
    app.press("Tab");
    app.expectText("9001");
    app.sleep(700);
    app.press("Tab");
    app.expectText("module Main where");
    app.sleep(800);

    // Toggle line numbers
    app.marker("Line Numbers & Styles");
    app.press("l");
    app.waitForText("Line numbers: ON");
    app.sleep(600);

    // Cycle document styles: classic -> minimal -> custom live editor -> modern
    app.press("s");
    app.waitForText("Style: classic");
    app.sleep(600);
    app.press("s");
    app.waitForText("Style: minimal");
    app.sleep(600);
    app.press("s");
    app.waitForText("Style: custom");
    app.expectText("LIVE STYLE EDITOR");
    app.sleep(700);
    app.press("Down");
    app.press("Right");
    app.sleep(600);
    app.press("s");
    app.waitForText("Style: modern");
    app.sleep(500);

    // Search query highlight
    app.marker("Live Search Highlight");
    app.press("/");
    app.sleep(300);
    app.typeText("Haskell");
    app.waitForText("Match 1 of");
    app.sleep(800);
    app.press("Escape");
    app.sleep(400);

    // Cycle themes and documents
    app.marker("Themes & Documents");
    app.press("t");
    app.waitForText("tokyo_night");
    app.sleep(600);
    app.press("t");
    app.waitForText("dracula");
    app.sleep(600);

    app.press("o");
    app.waitForText("Welcome to QmlTui Markdown Viewer");
    app.sleep(900);
    app.stopRecording();
}
