/** @param {Runner} app */
function main(app) {
    app.waitForText("NORMAL", 2000);
    app.expectText("workspace");
    app.marker("Miller Columns");
    app.sleep(700);

    // 1. Navigate into assets and showcase rich image, SVG, and PDF previews
    app.marker("Image, SVG & PDF Previews");
    app.press("l");
    app.waitForText("archive.tar.gz", 2000);
    app.sleep(450);

    // JPEG image preview + render mode cycling (Auto -> HalfBlock -> Braille -> Auto)
    app.press("j");
    app.waitForText("banner.jpg", 2000);
    app.waitForText("▀", 2000);
    app.sleep(650);
    app.press("m");
    app.press("m");
    app.waitForText("[m] Mode: Braille", 2000);
    app.sleep(650);
    app.press("m");
    app.press("m");
    app.waitForText("[m] Mode: Auto", 2000);
    app.sleep(450);

    // Vector SVG preview
    app.press("j");
    app.waitForText("Image Asset · SVG", 2000);
    app.waitForText("▀", 2000);
    app.sleep(650);

    // PNG preview
    app.press("j");
    app.waitForText("512x512 px", 2000);
    app.waitForText("▀", 2000);
    app.sleep(650);

    // Multi-page PDF preview + page navigation
    app.press("j");
    app.waitForText("Page 1/3", 2000);
    app.waitForText("▀", 2000);
    app.sleep(600);
    app.press("]");
    app.waitForText("Page 2/3", 2000);
    app.sleep(600);

    // Return to /workspace
    app.press("h");
    app.waitForText("README.md", 2000);
    app.sleep(400);

    // 2. Navigate into src and preview syntax-highlighted source code
    app.marker("Syntax-Highlighted Source");
    app.press("j");
    app.press("j");
    app.press("l");
    app.waitForText("main.cpp", 2000);
    app.sleep(650);
    app.press("j");
    app.sleep(550);
    app.press("h");
    app.waitForText("README.md", 2000);
    app.sleep(400);

    // 3. Preview README.md, mark file, and toggle dotfiles
    app.press("End");
    app.waitForText("Terminal File Manager", 2000);
    app.sleep(650);

    app.marker("Selection & Live Filter");
    app.press("Space");
    app.waitForText("[1 marked]", 2000);
    app.sleep(500);
    app.press("Space");
    app.expectNoText("[1 marked]");
    app.press(".");
    app.waitForText(".git", 2000);
    app.sleep(550);

    // 4. Live filter search
    app.press("/");
    app.waitForText("FILTER", 2000);
    app.sleep(300);
    app.typeText("CMake");
    app.waitForText("CMakeLists.txt", 2000);
    app.sleep(650);
    app.press("Escape");
    app.waitForText("README.md", 2000);
    app.sleep(400);

    // 5. Open split & fullscreen embedded shell drawer, then close cleanly
    app.marker("Embedded Shell Drawer");
    app.press("!");
    app.waitForText("SHELL ●", 2000);
    app.sleep(650);
    app.press("Escape");
    app.waitForText("SHELL ○", 2000);
    app.sleep(400);
    app.press("Escape");
    app.sleep(300);
    app.expectNoText("SHELL ○");

    // 6. Open Keybindings Help modal
    app.marker("Help Modal");
    app.press("?");
    app.waitForText("KEYBINDINGS", 2000);
    app.sleep(850);
    app.press("Escape");
    app.waitForText("NORMAL", 2000);
    app.sleep(700);
    app.stopRecording();
}
