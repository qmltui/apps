/** @param {Runner} app */
function main(app) {
    app.waitForText("2048 TUI");
    app.expectText("SCORE");
    app.expectText("BEST");
    app.marker("Initial Board");
    app.sleep(700);

    app.marker("Tile Merges");
    const moves = [
        "UP", "LEFT", "DOWN", "RIGHT",
        "UP", "LEFT", "UP", "RIGHT",
        "DOWN", "LEFT", "DOWN", "RIGHT",
        "UP", "LEFT", "DOWN", "LEFT"
    ];
    for (const move of moves) {
        app.press(move);
        app.sleep(320);
    }

    app.sleep(900);
    app.stopRecording();
}
