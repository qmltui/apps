/** @param {Runner} terminal */
// Audio Player TUI demo-mode smoke scenario (120x36).
// Verifies the revived 8-row Classic Winamp MainDeck, Hybrid Chrome workspace
// (Left Stack: MainDeck + Playlist | Right Wing: Spectrum + Visualizer),
// ANSI skins, action-hijacked marquee readouts, transport, Load Media modal,
// and W cycling Stereo Scope -> Phosphor Scope -> Radial Scope.
function main(terminal) {
    terminal.waitForText("AUDIO PLAYER");
    terminal.sleep(300); // wall time: C++ synth timer builds up levels
    terminal.expectText("DEMO");
    terminal.expectText("[CLASSIC]");
    terminal.expectText("PLAYLIST");
    terminal.expectText("1. Neon Pulse - Synthwave Runner");
    terminal.expectText("2. SubGrid - Acid Overdrive");
    terminal.expectText("SPECTRUM // 24-BAND FFT");
    terminal.expectText("VISUALIZER // STEREO SCOPE");
    terminal.expectText("PLAYING");
    terminal.expectText("├");
    terminal.expectText("Quit");
    terminal.captureText("audio_player/initial.txt");

    // Winamp Action Marquee: Volume, Balance, Skin cycling, and Llama Easter egg
    terminal.typeText("+");
    terminal.expectText("VOLUME: 85%");
    terminal.typeText("-");
    terminal.expectText("VOLUME: 80%");

    terminal.typeText(".");
    terminal.expectText("BALANCE: 10% RIGHT");
    terminal.typeText(",");
    terminal.expectText("BALANCE: CENTER");

    terminal.typeText("k");
    terminal.expectText("[AMBER AMP]");
    terminal.expectText("SKIN: AMBER AMP");
    terminal.typeText("k");
    terminal.expectText("[CYBER NEON]");
    terminal.typeText("k");
    terminal.expectText("[CLASSIC]");

    terminal.clickText("⚡ AUDIO PLAYER");
    terminal.expectText("WHIPS THE LLAMA'S ASS!");

    // Clicking LCD mini-visualizer cycles to Braille Oscilloscope inside the recessed LCD
    terminal.click(8, 3);
    terminal.expectText("LCD ANALYZER: OSCILLOSCOPE");
    terminal.captureText("audio_player/lcd_scope.txt");
    terminal.click(8, 3);
    terminal.expectText("LCD ANALYZER: OFF");
    terminal.click(8, 3);
    terminal.expectText("LCD ANALYZER: SPECTRUM");

    // 1 hides the Main Player deck: Playlist expands to full left column height
    terminal.typeText("1");
    terminal.expectNoText("⚡ AUDIO PLAYER");
    terminal.expectText("PLAYLIST");
    terminal.captureText("audio_player/main_deck_hidden.txt");
    terminal.typeText("1");
    terminal.expectText("⚡ AUDIO PLAYER");

    // 2 hides the Playlist deck: MainDeck centers vertically in left column
    terminal.typeText("2");
    terminal.expectNoText("+add");
    terminal.expectText("⚡ AUDIO PLAYER");
    terminal.expectText("SPECTRUM // 24-BAND FFT");
    terminal.typeText("2");
    terminal.expectText("+add");

    // 3 and 4 hide the right visualizer wing: Winamp stack centers at hardware width
    terminal.typeText("3");
    terminal.expectNoText("SPECTRUM // 24-BAND FFT");
    terminal.expectText("VISUALIZER // STEREO SCOPE");
    terminal.typeText("4");
    terminal.expectNoText("VISUALIZER //");
    terminal.expectText("⚡ AUDIO PLAYER");
    terminal.expectText("PLAYLIST");
    terminal.captureText("audio_player/visualizer_hidden.txt");
    terminal.typeText("3");
    terminal.typeText("4");
    terminal.expectText("SPECTRUM // 24-BAND FFT");
    terminal.expectText("VISUALIZER // STEREO SCOPE");

    // Hiding all 4 decks shows the restore hint, then restoring brings them back
    terminal.typeText("1");
    terminal.typeText("2");
    terminal.typeText("3");
    terminal.typeText("4");
    terminal.expectText("All decks hidden");
    terminal.typeText("1");
    terminal.typeText("2");
    terminal.typeText("3");
    terminal.typeText("4");
    terminal.expectNoText("All decks hidden");
    terminal.expectText("⚡ AUDIO PLAYER");
    terminal.expectText("PLAYLIST");

    // Playlist keyboard navigation: Down + Enter selects and plays Track 2
    terminal.press("Down");
    terminal.press("Return");
    terminal.expectText("ACID OVERDRIVE");

    // Playlist mouse interaction: clicking Track 4 focuses the playlist and plays it
    terminal.clickText("4. Demoscene");
    terminal.expectText("▸ PLAYLIST");
    terminal.expectText("PHOSPHOR HORIZON");
    terminal.clickText("1. Neon Pulse");
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
    terminal.captureText("audio_player/load_dialog.txt");
    terminal.press("Escape");
    terminal.expectNoText("LOAD / ADD MEDIA");
    terminal.expectText("PLAYLIST");

    // W cycles the visualizer mode: Stereo Scope -> Phosphor Scope -> Radial Scope -> Stereo Scope
    terminal.typeText("w");
    terminal.sleep(200); // wall time: C++ synth timer fills the phosphor scope
    terminal.expectText("VISUALIZER // PHOSPHOR SCOPE");
    terminal.expectRegex("[⠁-⣿]");
    terminal.captureText("audio_player/phosphor_scope.txt");

    terminal.typeText("w");
    terminal.sleep(200); // wall time: C++ synth timer fills the radial scope
    terminal.expectText("VISUALIZER // RADIAL SCOPE");
    terminal.expectRegex("[⠁-⣿]");
    terminal.captureText("audio_player/radial_scope.txt");

    terminal.typeText("w");
    terminal.expectText("VISUALIZER // STEREO SCOPE");

    // Blend toggle for shared oscilloscope cells
    terminal.typeText("a");
    terminal.expectText("Blend");
    terminal.typeText("a");

    terminal.captureText("audio_player/final.txt");
}
