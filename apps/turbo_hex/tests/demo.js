/** @param {Runner} app */
function main(app) {
    // Deterministic CLOCK VIRTUAL Asciinema showcase: Turbo Hex Binary Inspector on QmlTui.TVision
    app.waitForText("Turbo Hex");
    app.marker("3-Window Binary Inspector");
    app.sleep(850);

    // 1. Synchronized ELF64 header inspection: click fields in StructureWindow
    app.marker("ELF64 Header & Span Highlighting");
    app.item("structureWindow").clickText("e_entry");
    app.sleep(650);
    app.item("structureWindow").clickText("Program Header #0");
    app.sleep(650);
    app.item("structureWindow").clickText("\"QmlTui::TVision Firmware");
    app.sleep(750);

    // 2. Search Hex Bytes (`Ctrl+F`) -> jump to machine code bytes `48 31 C0 C3`
    app.marker("Hex Pattern Search (Ctrl+F)");
    app.press("F", ["Ctrl"]);
    app.waitForItem("findBytesDialog");
    app.sleep(350);
    app.typeText("48 31 C0 C3");
    app.sleep(450);
    app.press("Enter");
    app.sleep(700);

    // 3. Patch byte in Overwrite Mode (`F4`) and save (`F2`)
    app.marker("In-Place Byte Patching (F4)");
    app.press("F4");
    app.sleep(350);
    app.typeText("9090");
    app.sleep(550);
    app.press("F2");
    app.press("F4");
    app.sleep(450);

    // 4. Open second binary (`badge.png`) & Tile MDI windows side-by-side
    app.marker("Multi-File PNG Inspection & MDI Tile");
    app.press("F");
    app.sleep(300);
    app.press("P");
    app.waitForItem("hexSecondary");
    app.sleep(650);

    app.press("W");
    app.sleep(300);
    app.press("T");
    app.sleep(900);

    // 5. Switchable BIOS / DOS palettes (`F9`)
    app.marker("Switchable Palettes (F9)");
    app.press("F9");
    app.sleep(600);
    app.press("F9");
    app.sleep(600);
    app.press("F9");
    app.sleep(550);
    app.press("F9");
    app.sleep(850);

    app.stopRecording();
}
