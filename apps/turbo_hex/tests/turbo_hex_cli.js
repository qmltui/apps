/** @param {Runner} app */
function main(app) {
    // CLI binary file inspection test (`tests/fixtures/sample.bin`)
    app.waitForText("Turbo Hex");
    app.expectText("sample.bin [WASM]");
    app.item("structureWindow").expectText("WASM Magic");
    app.item("structureWindow").expectText("\"TURBO_HEX_CLI_OK\"");

    // Click extracted string in StructureWindow -> jumps HexWindow & InspectorWindow to string offset
    app.item("structureWindow").clickText("\"TURBO_HEX_CLI_OK\"");
    app.item("inspectorWindow").expectText("\"TURBO_HEX_CL\"");

    app.press("Q", ["Ctrl"]);
}
