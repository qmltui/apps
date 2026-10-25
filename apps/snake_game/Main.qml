pragma ComponentBehavior: Bound

import QtQml
import QtQml.Models
import QmlTui

Terminal {
    id: rootWindow

    property int score: 0
    property int highScore: 0
    property int level: 1
    property bool gameOver: false
    property bool won: false
    property bool isPaused: false
    property bool initialized: false
    property bool resizing: false

    property int foodX: 0
    property int foodY: 0
    property bool foodPulse: false
    property alias snakeModel: snakeModel
    property int headX: 0
    property int headY: 0

    property string direction: "right"
    property list<string> directionQueue: []
    property int movementTick: 0

    readonly property int cellPixelWidth: 2
    readonly property int cellPixelHeight: 2
    readonly property int gridWidth: Math.floor(gameCanvas.pixelWidth / cellPixelWidth)
    readonly property int gridHeight: Math.floor(gameCanvas.pixelHeight / cellPixelHeight)
    readonly property bool playable: gridWidth >= 6 && gridHeight >= 5
    readonly property int speedInterval: Math.max(48, 105 - ((level - 1) * 6))

    function isSnakeAt(cellX: int, cellY: int, limit: int): bool {
        for (let i = 0; i < limit; ++i) {
            let segment = snakeModel.get(i);
            if (segment.cellX === cellX && segment.cellY === cellY)
                return true;
        }
        return false;
    }

    function paintCell(cellX: int, cellY: int, cellColor: string) {
        let originX = cellX * cellPixelWidth;
        let originY = cellY * cellPixelHeight;
        for (let pixelY = 0; pixelY < cellPixelHeight; ++pixelY) {
            for (let pixelX = 0; pixelX < cellPixelWidth; ++pixelX)
                gameCanvas.setPixelColor(originX + pixelX, originY + pixelY, cellColor);
        }
    }

    // Erasing is setPixel(false), not painting "transparent": on a Canvas
    // "transparent" is the terminal's default ink, so a "transparent" pixel is
    // still set and renders as a block in the foreground colour.
    function eraseCell(cellX: int, cellY: int) {
        let originX = cellX * cellPixelWidth;
        let originY = cellY * cellPixelHeight;
        for (let pixelY = 0; pixelY < cellPixelHeight; ++pixelY) {
            for (let pixelX = 0; pixelX < cellPixelWidth; ++pixelX)
                gameCanvas.setPixel(originX + pixelX, originY + pixelY, false);
        }
    }

    function redrawCanvas() {
        gameCanvas.clear();
        if (!playable || snakeModel.count === 0)
            return;

        if (!gameOver) {
            paintCell(foodX, foodY, foodPulse ? "#ffb000" : "#ff4365");
            gameCanvas.setPixelColor(foodX * cellPixelWidth + 1, foodY * cellPixelHeight, "#b7f34a");
        }

        for (let i = snakeModel.count - 1; i >= 0; --i) {
            let segment = snakeModel.get(i);
            let segmentColor = i === 0 ? "#82ff9f" : (((movementTick - i) & 1) !== 0 ? "#2bdc68" : "#22b455");
            paintCell(segment.cellX, segment.cellY, segmentColor);
        }

        let eyeX = headX * cellPixelWidth + (direction === "right" ? 1 : 0);
        let eyeY = headY * cellPixelHeight + (direction === "down" ? 1 : 0);
        gameCanvas.setPixelColor(eyeX, eyeY, "#000000");
    }

    function resetGame() {
        score = 0;
        level = 1;
        gameOver = false;
        won = false;
        isPaused = false;
        direction = "right";
        directionQueue = [];
        movementTick = 0;
        foodPulse = false;
        snakeModel.clear();
        headX = 0;
        headY = 0;

        if (!playable) {
            redrawCanvas();
            return;
        }

        let centerX = Math.floor(gridWidth / 2);
        let centerY = Math.floor(gridHeight / 2);
        headX = centerX;
        headY = centerY;
        snakeModel.append({
            "cellX": centerX,
            "cellY": centerY
        });
        snakeModel.append({
            "cellX": centerX - 1,
            "cellY": centerY
        });
        snakeModel.append({
            "cellX": centerX - 2,
            "cellY": centerY
        });
        spawnFood();
        redrawCanvas();
    }

    function spawnFood() {
        if (!playable)
            return;

        let cellCount = gridWidth * gridHeight;
        if (snakeModel.count >= cellCount) {
            finishGame(true);
            return;
        }

        let firstCell = Math.floor(Math.random() * cellCount);
        for (let offset = 0; offset < cellCount; ++offset) {
            let candidate = (firstCell + offset) % cellCount;
            let candidateX = candidate % gridWidth;
            let candidateY = Math.floor(candidate / gridWidth);
            if (!isSnakeAt(candidateX, candidateY, snakeModel.count)) {
                foodX = candidateX;
                foodY = candidateY;
                return;
            }
        }

        finishGame(true);
    }

    function finishGame(didWin: bool) {
        won = didWin;
        gameOver = true;
        highScore = Math.max(highScore, score);
        redrawCanvas();
    }

    function queueDirection(candidate: string) {
        let queueLen = directionQueue.length;
        let previousDirection = queueLen > 0 ? directionQueue[queueLen - 1] : direction;
        if (candidate === previousDirection || queueLen >= 2)
            return;

        let isReverse = (previousDirection === "left" && candidate === "right") || (previousDirection === "right" && candidate === "left") || (previousDirection === "up" && candidate === "down") || (previousDirection === "down" && candidate === "up");
        if (isReverse)
            return;

        if (queueLen === 0)
            directionQueue = [candidate];
        else
            directionQueue = [directionQueue[0], candidate];
    }

    function step() {
        if (!playable || gameOver || isPaused || resizing || snakeModel.count === 0)
            return;

        if (directionQueue.length > 0) {
            let nextDir = directionQueue[0];
            if (directionQueue.length > 1)
                directionQueue = [directionQueue[1]];
            else
                directionQueue = [];
            direction = nextDir;
        }

        let nextX = headX;
        let nextY = headY;

        if (direction === "right")
            ++nextX;
        else if (direction === "left")
            --nextX;
        else if (direction === "up")
            --nextY;
        else
            ++nextY;

        nextX = (nextX + gridWidth) % gridWidth;
        nextY = (nextY + gridHeight) % gridHeight;

        let ateFood = nextX === foodX && nextY === foodY;
        // On a normal move the tail leaves its cell, so entering that cell is legal.
        let collisionCount = snakeModel.count - (ateFood ? 0 : 1);
        if (isSnakeAt(nextX, nextY, collisionCount)) {
            finishGame(false);
            return;
        }

        if (!ateFood) {
            let oldTail = snakeModel.get(snakeModel.count - 1);
            eraseCell(oldTail.cellX, oldTail.cellY);
            snakeModel.remove(snakeModel.count - 1);
        }

        // Old head becomes body segment; (movementTick - i) parity stays invariant as the snake moves.
        let neckColor = (movementTick & 1) !== 0 ? "#2bdc68" : "#22b455";
        paintCell(headX, headY, neckColor);

        snakeModel.insert(0, {
            "cellX": nextX,
            "cellY": nextY
        });
        headX = nextX;
        headY = nextY;

        // Paint new head
        paintCell(nextX, nextY, "#82ff9f");
        let eyeX = nextX * cellPixelWidth + (direction === "right" ? 1 : 0);
        let eyeY = nextY * cellPixelHeight + (direction === "down" ? 1 : 0);
        gameCanvas.setPixelColor(eyeX, eyeY, "#000000");

        ++movementTick;
        if (movementTick % 3 === 0)
            foodPulse = !foodPulse;

        if (ateFood) {
            score += 10;
            highScore = Math.max(highScore, score);
            level = Math.floor(score / 50) + 1;
            if (snakeModel.count === gridWidth * gridHeight)
                finishGame(true);
            else
                spawnFood();
        }

        if (!gameOver) {
            paintCell(foodX, foodY, foodPulse ? "#ffb000" : "#ff4365");
            gameCanvas.setPixelColor(foodX * cellPixelWidth + 1, foodY * cellPixelHeight, "#b7f34a");
        }
    }

    function scheduleResize() {
        if (!initialized)
            return;
        resizing = true;
        resizeTimer.restart();
    }

    function finishResize() {
        resizing = false;
        if (!playable) {
            redrawCanvas();
            return;
        }

        for (let i = 0; i < snakeModel.count; ++i) {
            let segment = snakeModel.get(i);
            if (segment.cellX >= gridWidth || segment.cellY >= gridHeight) {
                resetGame();
                return;
            }
        }

        if (snakeModel.count === 0)
            resetGame();
        else if (foodX >= gridWidth || foodY >= gridHeight || isSnakeAt(foodX, foodY, snakeModel.count))
            spawnFood();
        redrawCanvas();
    }

    color: "#000000"

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Q || event.key === Qt.Key_Escape) {
            Qt.quit();
            event.accepted = true;
            return;
        }

        if (event.key === Qt.Key_R || (gameOver && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter))) {
            resetGame();
            event.accepted = true;
            return;
        }

        if ((event.key === Qt.Key_Space || event.key === Qt.Key_P) && !gameOver && playable) {
            isPaused = !isPaused;
            event.accepted = true;
            return;
        }

        if (gameOver || isPaused || !playable)
            return;

        if (event.key === Qt.Key_Left || event.key === Qt.Key_A || event.key === Qt.Key_H) {
            queueDirection("left");
            event.accepted = true;
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_D || event.key === Qt.Key_L) {
            queueDirection("right");
            event.accepted = true;
        } else if (event.key === Qt.Key_Up || event.key === Qt.Key_W || event.key === Qt.Key_K) {
            queueDirection("up");
            event.accepted = true;
        } else if (event.key === Qt.Key_Down || event.key === Qt.Key_S || event.key === Qt.Key_J) {
            queueDirection("down");
            event.accepted = true;
        }
    }

    Component.onCompleted: {
        initialized = true;
        resetGame();
    }

    onGridWidthChanged: scheduleResize()
    onGridHeightChanged: scheduleResize()

    ListModel {
        id: snakeModel
    }

    Timer {
        interval: rootWindow.speedInterval
        running: rootWindow.initialized && rootWindow.playable && !rootWindow.gameOver && !rootWindow.isPaused && !rootWindow.resizing
        repeat: true
        onTriggered: rootWindow.step()
    }

    Timer {
        id: resizeTimer
        interval: 100
        repeat: false
        onTriggered: rootWindow.finishResize()
    }

    Column {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: header
            width: parent.width
            height: 3
            color: "#111111"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.top: parent.top
                text: parent.width < 64 ? qsTr("SNAKE") : qsTr("SNAKE // HALF-BLOCK EDITION")
                color: "#7dff9b"
                bold: true
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 1
                anchors.top: parent.top
                text: parent.width < 54 ? qsTr("%1 pts").arg(rootWindow.score) : qsTr("SCORE %1  BEST %2  LVL %3").arg(rootWindow.score).arg(rootWindow.highScore).arg(rootWindow.level)
                color: "#ffd166"
                bold: true
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.bottom: parent.bottom
                text: parent.width < 70 ? qsTr("WASD/ARROWS · SPACE · R · Q") : qsTr("MOVE WASD/ARROWS  ·  PAUSE SPACE  ·  RESTART R  ·  QUIT Q")
                color: "#789488"
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 1
                anchors.bottom: parent.bottom
                visible: parent.width >= 70
                text: rootWindow.gameOver ? (rootWindow.won ? qsTr("BOARD CLEARED") : qsTr("GAME OVER")) : (rootWindow.isPaused ? qsTr("PAUSED") : qsTr("RUNNING · %1ms").arg(rootWindow.speedInterval))
                color: rootWindow.gameOver ? "#ff5d73" : (rootWindow.isPaused ? "#ffd166" : "#57e389")
                bold: true
            }
        }

        Rectangle {
            id: gameField
            width: parent.width
            height: parent.height - header.height
            color: "#000000"
            border.style: Border.Single
            border.color: rootWindow.gameOver ? "#8f3045" : "#225c38"

            Canvas {
                id: gameCanvas
                x: 1
                y: 1
                width: Math.max(0, gameField.width - 2)
                height: Math.max(0, gameField.height - 2)
                renderMode: Canvas.HalfBlock
                autoClear: false
                fillColor: "transparent"
            }

            Rectangle {
                id: sizeWarning
                readonly property bool compact: width < 30 || gameField.height < 9

                anchors.centerIn: parent
                width: Math.max(3, Math.min(parent.width - 2, 38))
                height: compact ? Math.max(1, Math.min(parent.height - 2, 3)) : 7
                visible: !rootWindow.playable
                color: "#17120b"
                border.style: Border.Single
                border.color: "#ffd166"

                Column {
                    anchors.centerIn: parent
                    width: Math.max(0, parent.width - 2)
                    spacing: 1

                    Text {
                        width: parent.width
                        text: sizeWarning.compact ? qsTr("TOO SMALL") : qsTr("TERMINAL TOO SMALL")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#ffd166"
                        bold: true
                    }

                    Text {
                        width: parent.width
                        visible: !sizeWarning.compact
                        text: qsTr("Resize to at least 14 × 10")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#d7c7a4"
                    }

                    Text {
                        width: parent.width
                        visible: !sizeWarning.compact
                        text: qsTr("Current grid: %1 × %2").arg(rootWindow.gridWidth).arg(rootWindow.gridHeight)
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#789488"
                    }
                }
            }

            Rectangle {
                id: gameOverOverlay
                anchors.centerIn: parent
                width: Math.min(parent.width - 4, 38)
                height: 9
                visible: rootWindow.gameOver && rootWindow.playable
                color: rootWindow.won ? "#092417" : "#240c13"
                border.style: Border.Double
                border.color: rootWindow.won ? "#57e389" : "#ff5d73"

                Column {
                    anchors.centerIn: parent
                    width: Math.max(0, parent.width - 2)
                    spacing: 1

                    Text {
                        width: parent.width
                        text: rootWindow.won ? qsTr("BOARD CLEARED!") : qsTr("GAME OVER")
                        horizontalAlignment: Qt.AlignHCenter
                        color: rootWindow.won ? "#57e389" : "#ff5d73"
                        bold: true
                    }

                    Text {
                        width: parent.width
                        text: qsTr("Score %1  ·  Best %2").arg(rootWindow.score).arg(rootWindow.highScore)
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#ffd166"
                    }

                    Text {
                        width: parent.width
                        text: qsTr("R / ENTER to play again")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#e4eee8"
                    }

                    Text {
                        width: parent.width
                        text: qsTr("Q / ESC to quit")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#789488"
                    }
                }
            }

            Rectangle {
                id: pauseOverlay
                anchors.centerIn: parent
                width: Math.min(parent.width - 4, 28)
                height: 5
                visible: rootWindow.isPaused && !rootWindow.gameOver && rootWindow.playable
                color: "#17120b"
                border.style: Border.Single
                border.color: "#ffd166"

                Column {
                    anchors.centerIn: parent
                    width: Math.max(0, parent.width - 2)
                    spacing: 1

                    Text {
                        width: parent.width
                        text: qsTr("PAUSED")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#ffd166"
                        bold: true
                    }

                    Text {
                        width: parent.width
                        text: qsTr("SPACE to continue")
                        horizontalAlignment: Qt.AlignHCenter
                        color: "#e4eee8"
                    }
                }
            }
        }
    }
}
