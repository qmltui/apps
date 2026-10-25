// Audio Player TUI demo-mode smoke scenario (120x36).
// Verifies the 3 docked Audio Player decks (Main Player, Playlist Editor, and
// Visualizer), transport controls, playlist track selection, modal Load dialog,
// skins, and visualizer modes (Spectrum + Scope, 24-Band Spectrum, Phosphor Scope).
function main(terminal) {
    terminal.waitForText("AUDIO PLAYER");
    terminal.sleep(300); // wall time: C++ synth timer builds up levels
    terminal.expectText("DEMO");
    terminal.expectText("SYNTHWAVE RUNNER");
    terminal.expectText("PLAYLIST");
    terminal.expectText("1. Neon Pulse - Synthwave Runner");
    terminal.expectText("2. SubGrid - Acid Overdrive");
    terminal.expectText("VISUALIZER // SPECTRUM + SCOPE · LP off");
    terminal.expectText("PLAYING");
    terminal.expectText("PL");
    terminal.expectText("VIS");
    terminal.expectText("Quit");
    terminal.captureText("audio_player/initial.txt");

    // 1 hides the Main Player deck: Playlist expands to full left column height
    terminal.typeText("1");
    terminal.expectNoText("⚡ AUDIO PLAYER");
    terminal.expectText("PLAYLIST");
    terminal.captureText("audio_player/main_deck_hidden.txt");
    terminal.typeText("1");
    terminal.expectText("⚡ AUDIO PLAYER");

    // 2 hides the Playlist deck: Main Player expands vertically
    terminal.typeText("2");
    terminal.expectNoText("PLAYLIST");
    terminal.expectText("VISUALIZER");
    terminal.typeText("2");
    terminal.expectText("PLAYLIST");

    // 3 hides the Visualizer deck: Left column takes full width
    terminal.typeText("3");
    terminal.expectNoText("VISUALIZER");
    terminal.expectText("PLAYLIST");
    terminal.captureText("audio_player/visualizer_hidden.txt");
    terminal.typeText("3");
    terminal.expectText("VISUALIZER");

    // Hiding all 3 decks shows the restore hint, then restoring brings them back
    terminal.typeText("1");
    terminal.typeText("2");
    terminal.typeText("3");
    terminal.expectText("All decks hidden");
    terminal.typeText("1");
    terminal.typeText("2");
    terminal.typeText("3");
    terminal.expectNoText("All decks hidden");
    terminal.expectText("⚡ AUDIO PLAYER");
    terminal.expectText("PLAYLIST");

    // Playlist navigation: Down + Enter selects and plays Track 2
    terminal.press("Down");
    terminal.press("Return");
    terminal.expectText("ACID OVERDRIVE");
    terminal.typeText("z"); // Previous track back to Track 1
    terminal.expectText("SYNTHWAVE RUNNER");

    // Transport: Space pauses, V stops, X plays
    terminal.press("Space");
    terminal.expectText("PAUSED");
    terminal.typeText("v");
    terminal.expectText("STOPPED");
    terminal.typeText("x");
    terminal.expectText("PLAYING");

    // Load Media modal (L opens, Escape closes without quitting)
    terminal.typeText("l");
    terminal.expectText("LOAD / ADD MEDIA");
    terminal.press("Escape");
    terminal.expectNoText("LOAD / ADD MEDIA");
    terminal.expectText("PLAYLIST");

    // Skin cycling (K): CLASSIC -> AMBER AMP -> CYBER NEON -> CLASSIC
    terminal.typeText("k");
    terminal.expectText("[AMBER AMP]");
    terminal.typeText("k");
    terminal.expectText("[CYBER NEON]");
    terminal.typeText("k");
    terminal.expectText("[CLASSIC]");

    // Scope low-pass presets: [ lowers the cutoff, ] raises it back to off
    terminal.typeText("[");
    terminal.expectText("LP 8 kHz");
    terminal.typeText("[");
    terminal.expectText("LP 4 kHz");
    terminal.typeText("]");
    terminal.typeText("]");
    terminal.expectText("LP off");

    // W cycles the visualizer mode: Combo -> 24-Band Spectrum -> Phosphor Scope -> Combo
    terminal.typeText("w");
    terminal.expectText("VISUALIZER // 24-BAND SPECTRUM");
    terminal.typeText("w");
    terminal.sleep(200); // wall time: C++ synth timer fills the phosphor scope
    terminal.expectText("VISUALIZER // PHOSPHOR SCOPE");
    terminal.expectRegex("[⠁-⣿]");
    terminal.captureText("audio_player/phosphor_scope.txt");
    terminal.typeText("w");
    terminal.expectText("VISUALIZER // SPECTRUM + SCOPE");

    // Blend toggle for shared oscilloscope cells
    terminal.typeText("a");
    terminal.expectText("Blend");
    terminal.typeText("a");

    terminal.captureText("audio_player/final.txt");
}
