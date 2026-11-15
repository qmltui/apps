pragma Singleton
pragma ComponentBehavior: Bound
import QtQml

// Audio Player palette and formatting helpers using strictly the terminal
// default color ("default") and 16-color ANSI palette entries ("ansi:*").
// Supports 3 switchable ANSI skins:
//   0: CLASSIC     (gold chrome, recessed black LCD, brightgreen LEDs, green/yellow/red bars)
//   1: AMBER AMP   (warm phosphor yellow/amber/red palette)
//   2: CYBER NEON  (brightcyan/magenta/green palette)
QtObject {
    id: theme

    property int skinIndex: 0
    readonly property int skinCount: 3

    readonly property string skinName: {
        switch (skinIndex) {
        case 1: return "AMBER AMP";
        case 2: return "CYBER NEON";
        default: return "CLASSIC";
        }
    }

    // Surfaces (terminal default + ANSI black LCD & brightblack highlight)
    readonly property string background: "default"
    readonly property string barBackground: "default"
    readonly property string panel: "default"
    readonly property string lcdBackground: "ansi:black"
    readonly property string playlistBg: "default"
    readonly property string playlistSelectedBg: "ansi:brightblack"

    // Deck Chrome, Hairline Dividers & Modal Border
    readonly property string border: "ansi:brightblack"
    readonly property string frame: "ansi:brightblack"
    readonly property string activeFrame: skinIndex === 2 ? "ansi:brightcyan" : "ansi:yellow"
    readonly property string titleBarGold: skinIndex === 2 ? "ansi:brightcyan" : "ansi:brightyellow"
    readonly property string header: titleBarGold

    // Text & LED Readouts
    readonly property string text: "default"
    readonly property string subtle: "ansi:white"
    readonly property string muted: "ansi:brightblack"
    readonly property string dim: "ansi:brightblack"
    readonly property string ledGreen: skinIndex === 1 ? "ansi:brightyellow"
                                     : skinIndex === 2 ? "ansi:brightcyan" : "ansi:brightgreen"

    // Playlist
    readonly property string playlistText: "default"
    readonly property string playlistActive: ledGreen

    // Semantic & Visualizer ANSI Colors
    readonly property string accent: ledGreen
    readonly property string secondary: skinIndex === 2 ? "ansi:brightmagenta" : "ansi:brightred"
    readonly property string mid: skinIndex === 1 ? "ansi:yellow"
                                : skinIndex === 2 ? "ansi:magenta" : "ansi:brightyellow"
    readonly property string error: "ansi:brightred"
    readonly property string warning: "ansi:brightyellow"
    readonly property string hot: "ansi:brightred"
    readonly property string playing: skinIndex === 1 ? "ansi:brightyellow" : "ansi:brightgreen"
    readonly property string fileSource: "ansi:brightgreen"
    readonly property string scopeAccent: ledGreen
    readonly property string scopeRight: skinIndex === 1 ? "ansi:brightred"
                                       : skinIndex === 2 ? "ansi:brightmagenta" : "ansi:brightcyan"
    readonly property string vuLow: skinIndex === 1 ? "ansi:brightyellow" : "ansi:brightgreen"

    // 1/8-cell vertical resolution for smooth bar tops.
    readonly property var blockGlyphs: [" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"]

    function nextSkin() {
        skinIndex = (skinIndex + 1) % skinCount;
    }

    // Truncates text to maxCells, ending with an ellipsis when cut.
    function fitText(text: string, maxCells: int): string {
        if (maxCells <= 0)
            return "";
        if (text.length <= maxCells)
            return text;
        return text.substring(0, Math.max(0, maxCells - 1)) + "…";
    }

    // Produces a looping horizontal marquee slice of `width` cells.
    function marqueeSlice(text: string, offset: int, width: int): string {
        if (width <= 0)
            return "";
        if (!text || text.length === 0)
            return " ".repeat(width);
        if (text.length <= width)
            return text.padEnd(width, " ");
        const loop = text + "   ";
        const len = loop.length;
        const start = ((offset % len) + len) % len;
        let out = "";
        for (let i = 0; i < width; ++i)
            out += loop[(start + i) % len];
        return out;
    }

    // Level 0..1 to a dB string with a full-scale reference of 1.0.
    function formatDecibels(level: real): string {
        if (level <= 0.01)
            return "-∞ dB";
        const db = 20 * Math.log10(level);
        return (db >= 0 ? "+" : "") + db.toFixed(1) + " dB";
    }

    function formatBalance(bal: real): string {
        const pct = Math.round(bal * 100);
        if (Math.abs(pct) <= 4)
            return "CTR";
        return pct < 0 ? (Math.abs(pct) + "L") : (pct + "R");
    }
}
