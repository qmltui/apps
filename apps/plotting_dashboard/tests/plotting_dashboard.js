/** @param {Runner} terminal */
function main(terminal) {
    terminal.waitForText("REAL-TIME VECTOR PLOTTING DASHBOARD");
    terminal.expectText("OSCILLOSCOPE WAVEFORM VIEW");
    terminal.expectText("XY PHASE ORBIT PLOT");
    terminal.expectText("WAVE: SINE");

    terminal.press("Space");
    terminal.expectText("▶ RESUME");

    terminal.press("Tab");
    terminal.press("Return");
    terminal.expectText("WAVE: SQUARE");

    terminal.captureText("plotting_dashboard_final.txt");
}
