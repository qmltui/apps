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

    // Focus embedded shell with '!' and verify Escape restores arrow key navigation
    terminal.press("!");
    terminal.waitForText("SHELL ●", 2000);
    terminal.press("Escape");
    terminal.waitForText("NORMAL", 2000);
    terminal.press("Down");
    terminal.waitForText("main.cpp", 2000);

    // Capture text artifact
    terminal.captureText("navigation.txt");
}
