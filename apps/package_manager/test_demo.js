/** @param {Runner} app */
function main(app) {
    app.expectText("Fetching [");
    app.expectScrollbackText("Installing qpm-cli v0.5.0 (~/.qpm/bin)");
    app.marker("Fetching Packages");
    for (let i = 0; i < 4; ++i) {
        app.sleep(400);
    }

    app.expectText("Building [");
    app.marker("Compiling Crates");
    for (let i = 0; i < 5; ++i) {
        app.sleep(400);
    }

    app.waitForText("Summary Successfully installed 8 packages (42.8 MiB)", 3000);
    app.marker("Install Complete");
    app.sleep(1200);
    app.stopRecording();
}
