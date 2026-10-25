function main(terminal) {
    terminal.waitForText("RETRO SCREENSAVER");
    terminal.expectText("STARFIELD");
    terminal.captureText("screensaver/active.txt");
    terminal.sleep(200);
    terminal.captureText("screensaver/01_starfield.txt");

    // Switch to Mystify
    terminal.press("2");
    terminal.waitForText("MODE: MYSTIFY");
    terminal.sleep(200);
    terminal.captureText("screensaver/02_mystify.txt");

    // Switch to Doom Fire
    terminal.press("3");
    terminal.waitForText("MODE: DOOM FIRE");
    terminal.sleep(300);
    terminal.captureText("screensaver/03_doom_fire.txt");

    // Switch to 3D Tesseract
    terminal.press("4");
    terminal.waitForText("MODE: 3D TESSERACT");
    terminal.sleep(200);
    terminal.captureText("screensaver/04_3d_tesseract.txt");

    // Switch to Plasma
    terminal.press("5");
    terminal.waitForText("MODE: PLASMA");
    terminal.sleep(200);
    terminal.captureText("screensaver/05_plasma.txt");

    // Switch to Matrix Rain
    terminal.press("6");
    terminal.waitForText("MODE: MATRIX RAIN");
    terminal.sleep(200);
    terminal.captureText("screensaver/06_matrix_rain.txt");

    // Switch to 3D Pipes
    terminal.press("7");
    terminal.waitForText("MODE: 3D PIPES");
    terminal.sleep(200);
    terminal.captureText("screensaver/07_pipes.txt");

    // Switch to ASCII Donut
    terminal.press("8");
    terminal.waitForText("MODE: ASCII DONUT");
    terminal.sleep(200);
    terminal.captureText("screensaver/08_donut.txt");

    // Switch to Game of Life
    terminal.press("9");
    terminal.waitForText("MODE: GAME OF LIFE");
    terminal.sleep(200);
    terminal.captureText("screensaver/09_life.txt");

    // Switch to DVD Logo
    terminal.press("0");
    terminal.waitForText("MODE: DVD LOGO");
    terminal.sleep(200);
    terminal.captureText("screensaver/10_dvd.txt");

    // Switch to Lorenz Attractor
    terminal.press("l");
    terminal.waitForText("MODE: LORENZ ATTRACTOR");
    terminal.sleep(200);
    terminal.captureText("screensaver/11_lorenz.txt");

    // Cycle modes with Tab back to Starfield
    terminal.press("Tab");
    terminal.waitForText("MODE: STARFIELD");

    // Pause
    terminal.press("Space");
    terminal.waitForText("PAUSED");

    // Resume
    terminal.press("Space");
    terminal.waitForText("RUNNING");

    // Cycle theme
    terminal.press("c");
    terminal.waitForText("EMERALD");

    terminal.finish();
}
