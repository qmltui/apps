/** @param {Runner} terminal */
function main(terminal) {
    terminal.waitForText("SNAKE");
    terminal.captureText("snake/ai_start.txt");

    let game = terminal.rootObject();

    function getNextMove(headX, headY, foodX, foodY, currentDir) {
        let dx = foodX - headX;
        let dy = foodY - headY;

        let preferred = [];
        if (Math.abs(dx) >= Math.abs(dy)) {
            if (dx > 0) preferred.push("RIGHT");
            else if (dx < 0) preferred.push("LEFT");
            if (dy > 0) preferred.push("DOWN");
            else if (dy < 0) preferred.push("UP");
        } else {
            if (dy > 0) preferred.push("DOWN");
            else if (dy < 0) preferred.push("UP");
            if (dx > 0) preferred.push("RIGHT");
            else if (dx < 0) preferred.push("LEFT");
        }

        const allMoves = ["RIGHT", "DOWN", "LEFT", "UP"];
        for (let i = 0; i < allMoves.length; ++i) {
            if (preferred.indexOf(allMoves[i]) === -1)
                preferred.push(allMoves[i]);
        }

        let opposites = {
            "RIGHT": "LEFT",
            "LEFT": "RIGHT",
            "UP": "DOWN",
            "DOWN": "UP"
        };

        let deltas = {
            "RIGHT": [1, 0],
            "LEFT": [-1, 0],
            "UP": [0, -1],
            "DOWN": [0, 1]
        };

        for (let i = 0; i < preferred.length; ++i) {
            let p = preferred[i];
            if (opposites[p] === currentDir)
                continue;
            let nx = (headX + deltas[p][0] + game.gridWidth) % game.gridWidth;
            let ny = (headY + deltas[p][1] + game.gridHeight) % game.gridHeight;
            if (!game.isSnakeAt(nx, ny, game.snakeModel.count - 1))
                return p;
        }
        return currentDir;
    }

    // Drive the snake to dynamically navigate towards food targets
    for (let step = 0; step < 80; ++step) {
        if (game.gameOver || game.score >= 30) {
            break;
        }

        let headX = game.headX;
        let headY = game.headY;
        let foodX = game.foodX;
        let foodY = game.foodY;
        let currentDir = game.direction.toUpperCase();

        let move = getNextMove(headX, headY, foodX, foodY, currentDir);
        if (move !== currentDir) {
            terminal.press(move);
        }
        terminal.sleep(game.speedInterval);
    }

    if (game.score < 10) {
        terminal.fail("AI failed to score; final score: " + game.score);
    }
    terminal.expectNoText("SCORE 0 ");
    terminal.captureText("snake/ai_gameplay.txt");
    terminal.finish();
}
