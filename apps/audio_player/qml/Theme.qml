pragma Singleton
pragma ComponentBehavior: Bound
import QtQml

// Audio Player palette and formatting helpers using strictly the terminal
// default color ("default") and 16-color ANSI palette entries ("ansi:*").
QtObject {
    id: theme

    // Surfaces (terminal default + ANSI brightblack highlight)
    readonly property string background: "default"
    readonly property string barBackground: "default"
    readonly property string panel: "default"
    readonly property string playlistBg: "default"
    readonly property string playlistSelectedBg: "ansi:brightblack"

    // Hairline Dividers & Modal Border (paired with dim: true on Text dividers)
    readonly property string border: "ansi:brightblack"
    readonly property string header: "ansi:brightyellow"

    // Text
    readonly property string text: "default"
    readonly property string subtle: "default"
    readonly property string muted: "ansi:brightblack"

    // Playlist
    readonly property string playlistText: "default"
    readonly property string playlistActive: "ansi:brightgreen"

    // Semantic & Visualizer ANSI Colors
    readonly property string accent: "ansi:brightgreen"
    readonly property string secondary: "ansi:brightred"
    readonly property string mid: "ansi:brightyellow"
    readonly property string error: "ansi:brightred"
    readonly property string warning: "ansi:brightyellow"
    readonly property string hot: "ansi:brightred"
    readonly property string playing: "ansi:brightgreen"
    readonly property string fileSource: "ansi:brightgreen"
    readonly property string scopeAccent: "ansi:brightgreen"
    readonly property string scopeRight: "ansi:brightcyan"
    readonly property string vuLow: "ansi:brightgreen"

    // 1/8-cell vertical resolution for smooth bar tops.
    readonly property var blockGlyphs: [" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"]

    // Truncates text to maxCells, ending with an ellipsis when cut.
    function fitText(text: string, maxCells: int): string {
        if (maxCells <= 0)
            return "";
        if (text.length <= maxCells)
            return text;
        return text.substring(0, Math.max(0, maxCells - 1)) + "…";
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
