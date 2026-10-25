/** @param {Runner} app */
function main(app) {
    app.waitForText("SNAKE");
    app.expectText("SCORE");
    const game = app.rootObject();
    app.marker("Gameplay Start");
    app.sleep(400);

    function getNextMove(headX, headY, foodX, foodY, currentDir) {
        const preferred = [];
        if (headX < foodX) preferred.push("RIGHT");
        else if (headX > foodX) preferred.push("LEFT");
        if (headY < foodY) preferred.push("DOWN");
        else if (headY > foodY) preferred.push("UP");

        for (const m of ["RIGHT", "DOWN", "LEFT", "UP"]) {
            if (preferred.indexOf(m) === -1)
                preferred.push(m);
        }

        const opposites = { RIGHT: "LEFT", LEFT: "RIGHT", UP: "DOWN", DOWN: "UP" };
        const deltas = { RIGHT: [1, 0], LEFT: [-1, 0], UP: [0, -1], DOWN: [0, 1] };

        for (const p of preferred) {
            if (opposites[p] === currentDir)
                continue;
            const nx = (headX + deltas[p][0] + game.gridWidth) % game.gridWidth;
            const ny = (headY + deltas[p][1] + game.gridHeight) % game.gridHeight;
            if (!game.isSnakeAt(nx, ny, game.snakeModel.count - 1))
                return p;
        }
        return currentDir;
    }

    for (let step = 0; step < 55; ++step) {
        if (game.gameOver || game.score >= 30)
            break;

        if (step === 25) {
            app.marker("Pause Overlay");
            app.press("SPACE");
            app.waitForText("PAUSED");
            app.sleep(700);
            app.press("SPACE");
            app.expectNoText("SPACE to continue");
        }

        const currentDir = game.direction.toUpperCase();
        const move = getNextMove(game.headX, game.headY, game.foodX, game.foodY, currentDir);
        if (move !== currentDir)
            app.press(move);
        app.sleep(game.speedInterval);
    }

    app.sleep(800);
    app.stopRecording();
}
