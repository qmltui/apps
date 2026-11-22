/** @param {Runner} app */
function main(app) {
    // 1. Initial 3-window synchronized workspace: HexWindow + Data Inspector + Headers & Strings
    app.waitForText("Turbo Hex");
    app.expectText("firmware.elf [ELF64]");
    app.item("inspectorWindow").expectText("0x000000 (0)");
    app.item("structureWindow").expectText("ELF Magic");

    // 2. Click `e_entry` in StructureWindow -> highlights 8-byte span & decodes 0x00401000 in Inspector!
    app.item("structureWindow").clickText("e_entry");
    app.expectText("Sel: 8 B");
    app.item("inspectorWindow").expectText("0x000018 (24)");
    app.item("inspectorWindow").expectText("0x00401000");

    // 3. Go to Offset (`Ctrl+G`) -> jump to `0x78` (.rodata string)
    app.press("G", ["Ctrl"]);
    app.waitForItem("goToOffsetDialog");
    app.typeText("0x78");
    app.press("Enter");
    app.item("inspectorWindow").expectText("0x000078 (120)");
    app.item("inspectorWindow").expectText("\"QmlTui::TVis\"");

    // 4. Find Hex Bytes (`Ctrl+F`) -> locate x86-64 `48 31 C0 C3` epilogue at 0xB8
    app.press("F", ["Ctrl"]);
    app.waitForItem("findBytesDialog");
    app.typeText("48 31 C0 C3");
    app.press("Enter");
    app.item("inspectorWindow").expectText("0x0000B8 (184)");
    app.expectText("Sel: 4 B");

    // 5. Overwrite Edit Mode (`F4`): patch 1 byte (`90` NOP), test Unsaved Changes guard, then `F2` Save
    app.press("F4");
    app.expectText("[F4: EDIT]");
    app.typeText("90");
    app.expectText("firmware.elf [ELF64] *");

    app.press("F3", ["Alt"]);
    app.waitForItem("unsavedChangesDialog");
    app.press("Escape");
    app.expectText("firmware.elf [ELF64] *");
    app.press("F2");
    app.expectNoText("firmware.elf [ELF64] *");
    app.press("F4");
    app.expectText("[F4: VIEW]");

    // 6. Plain single-key mnemonics work directly in default VIEW mode!
    //    (a) Plain 'H' -> 'A' -> plain 'O' opens & closes About Turbo Hex
    app.clickText("firmware.elf [ELF64]");
    app.press("H");
    app.waitForText("About Turbo Hex...");
    app.press("A");
    app.waitForItem("aboutDialog");
    app.press("O");

    //    (b) Open second binary (`badge.png`) via plain 'F' -> 'P' and Tile windows side-by-side (`W` -> `T`)
    app.press("F");
    app.waitForText("Sample PNG Image");
    app.press("P");
    app.waitForItem("hexSecondary");
    app.expectText("badge.png [PNG]");
    app.item("structureWindow").expectText("Chunk IHDR · 32x16 px");

    app.press("W");
    app.waitForText("Tile");
    app.press("T");

    // 7. Cycle BIOS / DOS color themes (`F9`)
    app.press("F9");
    app.expectText("Turbo Cyan");
    app.press("F9");
    app.expectText("Dark DOS");
    app.press("F9");
    app.expectText("Monochrome");
    app.press("F9");
    app.expectText("Borland Blue");

    // 8. Multi-viewport resize check (`60x18`, `80x24`, `110x32`)
    app.resize(60, 18);
    app.expectText("File");
    app.expectText("Help");
    app.resize(80, 24);
    app.expectText("Turbo Hex");
    app.resize(110, 32);
    app.expectText("Turbo Hex");

    // 9. Paint hygiene & idle budget check (`dirtyDuringPaintTotal === 0`, zero idle frames)
    const stats = app.frameStats();
    if (stats.dirtyDuringPaintTotal !== 0) {
        app.fail("Expected dirtyDuringPaintTotal === 0, got " + stats.dirtyDuringPaintTotal);
    }
    const frameBefore = app.frameCount();
    app.advanceTime(500);
    const frameAfter = app.frameCount();
    if (frameAfter !== frameBefore) {
        app.fail("Unexpected idle repaints: " + frameBefore + " -> " + frameAfter);
    }

    app.captureText("turbo_hex/final_check.txt");
    app.press("X", ["Alt"]);
}
