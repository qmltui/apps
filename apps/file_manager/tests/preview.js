// test_preview.js - Tests preview column for directory, text, and binary items

function main(terminal) {
    terminal.waitForText("NORMAL", 2000);

    // Initial focus on assets folder -> Directory Preview
    terminal.expectText("assets");
    terminal.expectText("logo.png");

    // Move to docs folder
    terminal.press("j");
    terminal.expectText("docs");
    terminal.expectText("architecture.md");

    // Move down to README.md text file (last item in list)
    terminal.press("End");
    terminal.waitForText("README.md", 2000);

    // Verify text preview has line numbers and markdown content
    terminal.expectText("1 │");
    terminal.expectText("Terminal File Manager");
    terminal.captureStyled("readme_preview_styled.txt");

    // Move up to package.json to verify JSON key/string preview
    terminal.press("k");
    terminal.waitForText("\"file-manager\"", 2000);
    terminal.expectText("\"name\":");

    // Navigate to assets to test binary and image preview
    terminal.press("Home"); // Top: assets
    terminal.press("l"); // Enter assets
    terminal.waitForText("archive.tar.gz", 2000);

    // 1. Binary Archive Preview
    terminal.expectText("Binary / Archive");
    terminal.expectText("application/gzip");

    // 2. Image Asset Preview (move down to banner.jpg)
    terminal.press("j");
    terminal.waitForText("banner.jpg", 2000);
    terminal.expectText("Image Asset");
    terminal.expectText("image/jpeg");
    terminal.expectText("[m] Mode: Auto");
    // Wait for async worker thread to commit half-block pixels into the preview pane
    terminal.waitForText("▀", 2000);

    // Cycle render modes with 'm': Auto -> HalfBlock -> Braille -> ASCII -> Auto
    terminal.press("m");
    terminal.waitForText("[m] Mode: HalfBlock", 2000);
    terminal.press("m");
    terminal.waitForText("[m] Mode: Braille", 2000);
    terminal.press("m");
    terminal.waitForText("[m] Mode: ASCII", 2000);
    terminal.press("m");
    terminal.waitForText("[m] Mode: Auto", 2000);

    // 3. Vector SVG Asset Preview (move down to icon.svg)
    terminal.press("j");
    terminal.waitForText("Image Asset · SVG", 2000);
    terminal.expectText("image/svg+xml");
    terminal.expectText("Vector SVG");
    terminal.waitForText("▀", 2000);

    // 4. PNG Image Preview (move down to logo.png)
    terminal.press("j");
    terminal.waitForText("512x512 px", 2000);
    terminal.expectText("image/png");
    terminal.waitForText("▀", 2000);

    // 5. Multi-page PDF Document Preview (move down to spec.pdf)
    terminal.press("j");
    terminal.waitForText("Image Asset · PDF", 2000);
    terminal.expectText("application/pdf");
    terminal.expectText("Page 1/3");
    terminal.waitForText("▀", 2000);

    // Step PDF pages with ']' and '['
    terminal.press("]");
    terminal.waitForText("Page 2/3", 2000);
    terminal.press("]");
    terminal.waitForText("Page 3/3", 2000);
    terminal.press("[");
    terminal.waitForText("Page 2/3", 2000);

    terminal.captureText("preview.txt");
}
