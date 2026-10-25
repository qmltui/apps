/** @param {Runner} app */
function main(app) {
    app.waitForText("RETRO SCREENSAVER");
    app.expectText("STARFIELD");
    app.marker("Starfield");
    app.sleep(700);

    const modes = [
        { key: "2", label: "MODE: MYSTIFY", marker: "Mystify", ms: 650 },
        { key: "3", label: "MODE: DOOM FIRE", marker: "Doom Fire", ms: 750 },
        { key: "4", label: "MODE: 3D TESSERACT", marker: "3D Tesseract", ms: 700 },
        { key: "5", label: "MODE: PLASMA", marker: "Plasma", ms: 650 },
        { key: "6", label: "MODE: MATRIX RAIN", marker: "Matrix Rain", ms: 700 },
        { key: "7", label: "MODE: 3D PIPES", marker: "3D Pipes", ms: 700 },
        { key: "8", label: "MODE: ASCII DONUT", marker: "ASCII Donut", ms: 700 },
        { key: "9", label: "MODE: GAME OF LIFE", marker: "Game of Life", ms: 650 },
        { key: "0", label: "MODE: DVD LOGO", marker: "DVD Logo", ms: 650 },
        { key: "l", label: "MODE: LORENZ ATTRACTOR", marker: "Lorenz Attractor", ms: 750 }
    ];

    for (const m of modes) {
        app.marker(m.marker);
        app.press(m.key);
        app.waitForText(m.label);
        app.sleep(m.ms);
    }

    // Cycle color themes on Lorenz Attractor
    app.marker("Theme Cycling");
    app.press("c");
    app.waitForText("EMERALD");
    app.sleep(550);
    app.press("c");
    app.waitForText("AMBER");
    app.sleep(550);
    app.stopRecording();
}
