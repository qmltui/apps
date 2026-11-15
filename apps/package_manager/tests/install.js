/** @param {Runner} app */
function main(app) {
    // 1. Initial interactive Transaction Plan prompt & startup banner in scrollback
    app.expectText("Transaction Plan");
    app.expectText("qpm-studio v0.5.0");
    app.expectText("8 pkgs");
    app.expectText("qmltui-core");
    app.expectScrollbackText("qpm install qpm-studio --release");
    app.captureText("01_prompt_table.txt");

    // 2. Toggle optional wasm-sdk feature (3) and inspect dependency TreeView (d)
    app.typeText("3");
    app.expectText("10 pkgs");
    app.expectText("qmltui-wasm-shell");

    app.typeText("d");
    app.expectText("Table View");
    app.expectText("[+wasm-sdk]");
    app.captureText("02_prompt_tree.txt");
    app.typeText("d");
    app.expectText("Dep Tree");

    // 3. Confirm install -> locks plan into scrollback and starts 4-worker Fetching
    app.press("Return");
    app.expectText("Fetching Packages");
    app.expectScrollbackText("Locked Transaction Plan");
    app.expectScrollbackText("Resolved");
    app.expectScrollbackText("Cached");
    app.captureText("03_fetching.txt");

    // 4. Advance virtual clock into parallel Building phase
    app.advanceTime(2900);
    app.expectText("Building Release Graph");
    app.expectScrollbackText("Verified");
    app.expectScrollbackText("Ed25519 package signatures");

    // 5. Expand and collapse the live compiler stream inside Terminal.Inline
    app.typeText("v");
    app.expectText("LIVE STREAM");
    app.advanceTime(400);
    app.captureText("04_building_verbose.txt");
    app.typeText("v");
    app.expectNoText("LIVE STREAM");

    // 6. Step to completion and verify AOT diagnostic + final install receipt in scrollback
    app.advanceTime(4500);
    app.expectText("Summary  Successfully installed 10 packages");
    app.expectScrollbackText("qmlcachegen · AOT Binding Report");
    app.expectScrollbackText("Installed qpm-studio v0.5.0");
    app.expectScrollbackText("~/.qpm/bin/qpm-studio");
    app.captureText("05_finished_inline.txt");
    app.captureScrollback("05_finished_scrollback.txt");
}
