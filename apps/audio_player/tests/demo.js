/** @param {Runner} app */
function main(app) {
    app.waitForText("AUDIO PLAYER");
    app.marker("Docked Studio Decks");
    // Wall time: C++ DemoSynth timer drives real-time audio visualization
    app.sleep(600);
    app.expectText("DEMO");
    app.expectText("SYNTHWAVE RUNNER");
    app.expectText("PLAYLIST");
    app.expectText("SPECTRUM // 24-BAND FFT");
    app.expectText("VISUALIZER // STEREO SCOPE");

    // Playlist track selection
    app.marker("Playlist Track Selection");
    app.press("Down");
    app.sleep(400);
    app.press("Return");
    app.expectText("ACID OVERDRIVE");
    app.sleep(700);

    // Load Media modal (L opens, Escape closes)
    app.marker("Load Media Dialog");
    app.typeText("l");
    app.expectText("LOAD / ADD MEDIA");
    app.sleep(800);
    app.press("Escape");
    app.expectNoText("LOAD / ADD MEDIA");
    app.sleep(500);

    // Visualizer modes (W): Stereo Scope -> Phosphor Scope -> Radial Scope -> Stereo Scope
    app.marker("Phosphor Scope");
    app.typeText("w");
    app.expectText("VISUALIZER // PHOSPHOR SCOPE");
    app.sleep(800);

    app.marker("Radial Scope");
    app.typeText("w");
    app.expectText("VISUALIZER // RADIAL SCOPE");
    app.sleep(800);

    app.typeText("w");
    app.expectText("VISUALIZER // STEREO SCOPE");
    app.sleep(700);
    app.stopRecording();
}
