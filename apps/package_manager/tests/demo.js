/** @param {Runner} app */
function main(app) {
    app.expectText("Fetching [");
    app.expectScrollbackText("Installing qpm-cli v0.5.0 (~/.qpm/bin)");
    app.marker("Fetching Packages");

    app.waitForText("Building [", 5000);
    app.marker("Compiling Crates");

    app.waitForText("Summary Successfully installed 8 packages (42.8 MiB)", 6000);
    app.marker("Install Complete");
    app.sleep(1200);
    app.stopRecording();
}
