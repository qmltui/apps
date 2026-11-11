/** @param {Runner} app */
function main(app) {
    app.waitForText("2048 TUI");
    app.expectText("SCORE");
    app.expectText("BEST");
    const root = app.rootObject();
    app.marker("Opening Moves");
    app.sleep(600);

    const openingMoves = ["UP", "LEFT", "DOWN", "LEFT", "UP", "LEFT"];
    for (const move of openingMoves) {
        app.press(move);
        app.sleep(280);
    }

    // Seed an active mid-game board showcasing the full 2048 tile color palette
    app.marker("Mid-Game Tile Cascade");
    root.score = 6840;
    root.highScore = 6840;
    root.board = [
        512, 256, 128, 64,
        512, 256,  64, 32,
         16,   8,   4,  2,
         16,   8,   4,  2
    ];
    app.waitForFrame();
    app.sleep(750);

    // Merge vertical pairs (512+512->1024, 256+256->512, 16+16->32, 8+8->16, 4+4->8, 2+2->4)
    app.press("UP");
    app.sleep(500);

    // Set up final 1024 + 1024 merge to reach 2048
    app.marker("Reaching 2048");
    root.board = [
        1024, 1024, 512, 256,
         128,   64,  32,  16,
           8,    4,   2,   0,
           0,    0,   0,   2
    ];
    app.waitForFrame();
    app.sleep(700);

    app.press("LEFT");
    app.waitForText("You Win!");
    app.sleep(900);

    // Dismiss win overlay and continue playing
    app.press("C");
    app.expectNoText("You Win!");
    app.sleep(800);
    app.stopRecording();
}
