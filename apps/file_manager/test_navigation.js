// test_navigation.js - Tests Miller column navigation (j/k/h/l, Enter)

function main(terminal) {
    terminal.waitForText("NORMAL", 2000);
    terminal.expectText("workspace");

    // Check that top level folders and files are listed
    terminal.expectText("assets");
    terminal.expectText("docs");
    terminal.expectText("src");
    terminal.expectText("README.md");

    // Move cursor down
    terminal.press("j");
    terminal.press("j");

    // Navigate into 'src' folder (or directory under cursor)
    // Press 'l' or 'Enter' to enter directory
    terminal.press("l");
    terminal.waitForText("main.cpp", 2000);
    terminal.expectText("App.cpp");
    terminal.expectText("utils.rs");

    // Navigate back up to parent directory
    terminal.press("h");
    terminal.waitForText("README.md", 2000);
    terminal.expectText("docs");
    terminal.expectText("src");

    // Click an item in the middle column and verify Up/Down arrow keys still navigate
    terminal.clickText("docs");
    terminal.waitForText("architecture.md", 2000);
    terminal.press("Down");
    terminal.waitForText("main.cpp", 2000);
    terminal.press("Up");
    terminal.waitForText("architecture.md", 2000);

    // Verify Escape in NORMAL mode does not quit the application
    terminal.press("Escape");
    terminal.expectText("NORMAL");
    terminal.expectText("README.md");

    // 1. Open split shell with '!' and verify Escape returns to navigation while keeping shell open
    terminal.press("!");
    terminal.waitForText("SHELL ●", 2000);
    terminal.press("Escape");
    terminal.waitForText("NORMAL", 2000);
    terminal.expectText("SHELL ○ · /workspace");

    // 2. Navigate files while shell drawer remains open
    terminal.press("Down");
    terminal.waitForText("main.cpp", 2000);

    // 3. Double-press '!!' quickly to open Fullscreen Shell
    terminal.press("!");
    terminal.press("!");
    terminal.waitForText("SHELL (FULL) ●", 2000);
    terminal.expectText("SHELL (FULL)");

    // 4. Escape from fullscreen shell steps down to split unfocused and restores file navigation
    terminal.press("Escape");
    terminal.waitForText("NORMAL", 2000);
    terminal.expectText("SHELL ○ · /workspace");

    // 5. Enter 'src' directory and press '!' -> auto-syncs shell working directory to /workspace/src
    terminal.press("l");
    terminal.waitForText("App.cpp", 2000);
    terminal.press("!");
    terminal.waitForText("SHELL ● · /workspace/src", 2000);
    terminal.expectText("Shell cd → /workspace/src");

    // 6. Click an item in the file list while shell is focused -> unfocuses shell without closing it
    terminal.clickText("utils.rs");
    terminal.waitForText("SHELL ○ · /workspace/src", 2000);
    terminal.expectText("NORMAL");

    // 7. Refocus shell with '!', then step back with Escape (unfocus shell) and second Escape (hide shell drawer)
    terminal.press("!");
    terminal.waitForText("SHELL ● · /workspace/src", 2000);
    terminal.press("Escape");
    terminal.waitForText("SHELL ○ · /workspace/src", 2000);
    terminal.press("Escape");
    terminal.sleep(250);
    terminal.expectNoText("SHELL ○");

    // Navigate back to /workspace
    terminal.press("h");
    terminal.waitForText("README.md", 2000);

    // Capture text artifact
    terminal.captureText("navigation.txt");
}
