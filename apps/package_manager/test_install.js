/** @param {Runner} app */
function main(app) {
    // 1. Initial frame shows the live Cargo-style Fetching progress bar with byte totals
    app.expectText("Fetching [");
    app.expectText("1/8 (0 KiB/42.8 MiB): qmltui-ptyview, md4qt");
    app.expectScrollbackText("Installing qpm-cli v0.5.0 (~/.qpm/bin)");
    app.expectScrollbackText("Updating qt-registry.io index");
    app.captureText("01_fetching.txt");

    // 2. Step into the Compiling / Building phase (weighted by package sizes)
    app.advanceTime(1600);
    app.expectText("Building [");
    app.expectScrollbackText("Downloaded 8 packages (42.8 MiB) in");
    app.captureText("02_building.txt");

    // 3. Step to completion and verify dynamically computed scrollback log and summary line
    app.advanceTime(2000);
    app.waitForText("Summary Successfully installed 8 packages (42.8 MiB)", 3000);
    app.expectScrollbackText("Compiling kdab-kuesa v2.4.1");
    app.expectScrollbackText("Compiling qpm-cli v0.5.0");
    app.expectScrollbackText("Finished `release` profile [optimized] target(s)");
    app.expectScrollbackText("Installed ~/.qpm/bin/qpm-cli (8 packages, 42.8 MiB)");
    app.captureText("03_finished_inline.txt");
    app.captureScrollback("03_finished_scrollback.txt");
}
