/** @param {Runner} app */
function main(app) {
    app.waitForText("NORMAL", 2000);
    app.expectText("workspace");
    app.marker("Miller Columns");
    app.sleep(800);

    // Navigate into assets and preview image/archive metadata
    app.press("l");
    app.waitForText("archive.tar.gz", 2000);
    app.sleep(600);
    app.press("j");
    app.waitForText("banner.jpg", 2000);
    app.sleep(700);
    app.press("h");
    app.waitForText("README.md", 2000);
    app.sleep(500);

    // Navigate into src and preview source code
    app.marker("Source Preview");
    app.press("j");
    app.press("j");
    app.sleep(500);
    app.press("l");
    app.waitForText("main.cpp", 2000);
    app.sleep(700);
    app.press("j");
    app.sleep(600);
    app.press("h");
    app.waitForText("README.md", 2000);
    app.sleep(500);

    // Preview README.md
    app.press("End");
    app.waitForText("Terminal File Manager", 2000);
    app.sleep(800);

    // Mark file and toggle dotfiles
    app.marker("Selection & Dotfiles");
    app.press("Space");
    app.waitForText("[1 marked]", 2000);
    app.sleep(600);
    app.press("Space");
    app.expectNoText("[1 marked]");
    app.sleep(400);
    app.press(".");
    app.waitForText(".git", 2000);
    app.sleep(700);

    // Live filter search
    app.marker("Live Filter");
    app.press("/");
    app.waitForText("FILTER", 2000);
    app.sleep(400);
    app.typeText("CMake");
    app.waitForText("CMakeLists.txt", 2000);
    app.sleep(700);
    app.press("Escape");
    app.waitForText("README.md", 2000);
    app.sleep(500);

    // Open animated embedded shell drawer
    app.marker("Embedded Shell Drawer");
    app.press("!");
    app.sleep(300);
    app.expectText("SHELL ●");
    app.sleep(700);
    app.press("Escape");
    app.sleep(300);
    app.waitForText("NORMAL", 2000);
    app.sleep(500);

    // Open Keybindings Help modal
    app.marker("Help Modal");
    app.press("?");
    app.waitForText("KEYBINDINGS", 2000);
    app.sleep(900);
    app.press("Escape");
    app.waitForText("NORMAL", 2000);
    app.sleep(800);
    app.stopRecording();
}
