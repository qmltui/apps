/** @param {Runner} app */
function main(app) {
    app.waitForText("REAL-TIME VECTOR PLOTTING DASHBOARD");
    app.expectText("OSCILLOSCOPE WAVEFORM VIEW");
    app.expectText("XY PHASE ORBIT PLOT");
    app.marker("Sine Wave & Lissajous");
    app.sleep(900);

    // Focus Wave button and cycle waveform shapes
    app.marker("Waveform Shapes");
    app.press("Tab");
    app.press("Return");
    app.expectText("WAVE: SQUARE");
    app.sleep(800);

    app.press("Return");
    app.expectText("WAVE: TRIANGLE");
    app.sleep(800);

    app.press("Return");
    app.expectText("WAVE: SAWTOOTH");
    app.sleep(800);

    app.press("Return");
    app.expectText("WAVE: SINE");
    app.sleep(600);

    // Increase frequency (Tab -> FREQ - -> Tab -> FREQ +)
    app.marker("Frequency & Noise Modulation");
    app.press("Tab");
    app.press("Tab");
    app.press("Return");
    app.sleep(500);
    app.press("Return");
    app.sleep(600);

    // Add noise (Tab -> NOISE)
    app.press("Tab");
    app.press("Return");
    app.expectText("NOISE: 15%");
    app.sleep(800);

    // Adjust Lissajous X ratio (Tab -> X: - -> Tab -> X: +)
    app.marker("Lissajous Phase Ratio");
    app.press("Tab");
    app.press("Tab");
    app.press("Return");
    app.expectText("Ratio: 4 : 4");
    app.sleep(700);
    app.press("Return");
    app.expectText("Ratio: 5 : 4");
    app.sleep(900);

    // Pause and resume
    app.clickText("⏸ PAUSE");
    app.expectText("▶ RESUME");
    app.sleep(600);
    app.clickText("▶ RESUME");
    app.expectText("⏸ PAUSE");
    app.sleep(800);
    app.stopRecording();
}
