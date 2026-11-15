/** @param {Runner} app */
function main(app) {
    app.waitForText("Transaction Plan");
    app.marker("Transaction Plan & Features");
    // Wall time: showcase hold so viewer can read initial resolved table
    app.sleep(850);

    // Enable optional wasm-sdk feature (+2 packages, live table & size update)
    app.typeText("3");
    app.expectText("10 pkgs");
    app.sleep(650);

    // Inspect hierarchical dependency TreeView before confirming
    app.marker("Dependency Graph");
    app.typeText("d");
    app.expectText("[+wasm-sdk]");
    app.sleep(850);
    app.typeText("d");
    app.sleep(350);

    // Confirm plan -> commits Locked Transaction Plan card & starts 4-worker fetch
    app.marker("Parallel Fetch & Sparkline");
    app.press("Return");
    app.expectText("Fetching Packages");

    // Wait until Building Release Graph begins
    app.waitForText("Building Release Graph", 6000);
    app.marker("Parallel Build & Live Stream");
    app.sleep(650);

    // Expand inline compiler/linker stream dynamically, then collapse
    app.typeText("v");
    app.expectText("LIVE STREAM");
    app.sleep(1500);
    app.typeText("v");
    app.expectNoText("LIVE STREAM");

    // Wait for final Summary Receipt card committed into scrollback
    app.waitForText("Summary  Successfully installed 10 packages", 6000);
    app.marker("Install Summary Receipt");
    app.sleep(1400);
    app.stopRecording();
}
