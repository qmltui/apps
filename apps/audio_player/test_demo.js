/** @param {Runner} app */
function main(app) {
    app.waitForText("AUDIO PLAYER");
    app.marker("Docked Studio Decks");
    // Wall time: C++ DemoSynth timer drives real-time audio visualization
    app.sleep(600);
    app.expectText("DEMO");
    app.expectText("SYNTHWAVE RUNNER");
    app.expectText("PLAYLIST");
    app.expectText("VISUALIZER // SPECTRUM + SCOPE");

    // Playlist track selection
    app.marker("Playlist Track Selection");
    app.press("Down");
    app.sleep(400);
    app.press("Return");
    app.expectText("ACID OVERDRIVE");
    app.sleep(700);

    // Skin cycling (K): CLASSIC -> AMBER AMP -> CYBER NEON -> CLASSIC
    app.marker("Skin Cycling");
    app.typeText("k");
    app.expectText("[AMBER AMP]");
    app.sleep(700);
    app.typeText("k");
    app.expectText("[CYBER NEON]");
    app.sleep(700);
    app.typeText("k");
    app.expectText("[CLASSIC]");
    app.sleep(500);

    // Load Media modal (L opens, Escape closes)
    app.marker("Load Media Dialog");
    app.typeText("l");
    app.expectText("LOAD / ADD MEDIA");
    app.sleep(800);
    app.press("Escape");
    app.expectNoText("LOAD / ADD MEDIA");
    app.sleep(500);

    // Visualizer modes (W): 24-Band Spectrum -> Phosphor Scope -> Combo
    app.marker("24-Band Spectrum");
    app.typeText("w");
    app.expectText("VISUALIZER // 24-BAND SPECTRUM");
    app.sleep(800);

    app.marker("Phosphor Scope");
    app.typeText("w");
    app.expectText("VISUALIZER // PHOSPHOR SCOPE");
    app.sleep(800);

    app.typeText("w");
    app.expectText("VISUALIZER // SPECTRUM + SCOPE");
    app.sleep(700);
    app.stopRecording();
}
