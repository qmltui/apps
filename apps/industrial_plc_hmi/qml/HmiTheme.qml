pragma Singleton
pragma ComponentBehavior: Bound

import QtQml
import QmlTui

QtObject {
    id: theme

    // 3 switchable industrial SCADA skins:
    //   0: ISA-101 SLATE   (calm gunmetal slate, sky-cyan normal, emerald active, amber maint, crimson fault)
    //   1: AMBER CRT       (warm Honeywell/DEC gas-plasma amber phosphor)
    //   2: PHOSPHOR GREEN  (high-contrast classic oscilloscope green)
    property int skinIndex: 0
    readonly property int skinCount: 3

    readonly property string skinName: {
        switch (skinIndex) {
        case 1:
            return "AMBER CRT";
        case 2:
            return "PHOSPHOR";
        default:
            return "ISA-SLATE";
        }
    }

    // Core Surfaces
    readonly property color canvasBg: skinIndex === 1 ? "#0c0803" : skinIndex === 2 ? "#060b07" : "#080c10"
    readonly property color panelBg: canvasBg
    readonly property color panelAltBg: panelBg
    readonly property color recessedBg: skinIndex === 1 ? "#070401" : skinIndex === 2 ? "#040805" : "#05080b"
    readonly property color selectionBg: skinIndex === 1 ? "#2b1e0b" : skinIndex === 2 ? "#16301b" : "#1b2836"

    readonly property color headerBg: skinIndex === 1 ? "#1a1106" : skinIndex === 2 ? "#0e1a10" : "#111923"
    readonly property color headerBgError: "#36090c"
    readonly property color headerBgErrorPulse: "#520d12"
    readonly property color footerBg: skinIndex === 1 ? "#120c04" : skinIndex === 2 ? "#09110b" : "#0b1016"
    readonly property color footerBgError: "#2e080b"

    // Legacy aliases kept for compatibility
    readonly property color bgIdle: canvasBg
    readonly property color bgProcessing: canvasBg
    readonly property color bgMaintenance: canvasBg
    readonly property color bgError: "#160607"
    readonly property color bgErrorAlt: "#0d0405"
    readonly property color cardBg: panelBg
    readonly property color cardBgAlt: panelAltBg

    // Structural Borders
    readonly property color borderNormal: skinIndex === 1 ? "#4a3312" : skinIndex === 2 ? "#234229" : "#273545"
    readonly property color borderSubtle: skinIndex === 1 ? "#2e1f0b" : skinIndex === 2 ? "#162b1a" : "#1a2430"
    readonly property color borderFocus: skinIndex === 1 ? "#ffcc33" : skinIndex === 2 ? "#00ff66" : "#38bdf8"
    readonly property color borderActive: skinIndex === 1 ? "#ffb000" : skinIndex === 2 ? "#00e65c" : "#38bdf8"
    readonly property color borderWarning: "#ffb020"
    readonly property color borderError: "#ff453a"

    // Semantic Machine State Colors & High-Contrast Pill Ink
    readonly property color statusIdle: skinIndex === 1 ? "#d98a2b" : skinIndex === 2 ? "#4cd987" : "#38bdf8"
    readonly property color statusProcessing: skinIndex === 1 ? "#ffcc33" : skinIndex === 2 ? "#00ff66" : "#2ee66b"
    readonly property color statusMaintenance: skinIndex === 1 ? "#ff9900" : "#ffb020"
    readonly property color statusError: "#ff453a"
    readonly property color badgeInk: skinIndex === 1 ? "#0d0802" : skinIndex === 2 ? "#040b05" : "#060a0e"

    // Typography Colors
    readonly property color textPrimary: skinIndex === 1 ? "#ffe4ad" : skinIndex === 2 ? "#dff5e3" : "#e6edf3"
    readonly property color textSecondary: skinIndex === 1 ? "#d49b4b" : skinIndex === 2 ? "#8ec298" : "#9db0c2"
    readonly property color textDim: skinIndex === 1 ? "#7d5926" : skinIndex === 2 ? "#4d7555" : "#5c7082"
    readonly property color textHighlight: skinIndex === 1 ? "#ffcc33" : skinIndex === 2 ? "#00ff66" : "#38bdf8"
    readonly property color textSuccess: skinIndex === 1 ? "#ffd15c" : skinIndex === 2 ? "#2ee66b" : "#2ee66b"
    readonly property color textWarning: skinIndex === 1 ? "#ff9900" : "#ffb020"
    readonly property color textDanger: "#ff5c52"
    readonly property color textAccent: skinIndex === 1 ? "#ffb000" : skinIndex === 2 ? "#5ce1e6" : "#a78bfa"

    // Oscilloscope / Trend Scope Trace Colors
    readonly property color traceTemp: skinIndex === 1 ? "#ffcc33" : skinIndex === 2 ? "#00ff66" : "#fb923c"
    readonly property color traceSpeed: skinIndex === 1 ? "#ff8800" : skinIndex === 2 ? "#5ce1e6" : "#38bdf8"
    readonly property color tracePressure: skinIndex === 1 ? "#d98a2b" : skinIndex === 2 ? "#ffd166" : "#a78bfa"
    readonly property color traceLimit: "#ff453a"
    readonly property color traceGrid: skinIndex === 1 ? "#33220b" : skinIndex === 2 ? "#18301c" : "#1c2834"

    // Tactile Pushbutton Tints
    readonly property color btnStartBg: skinIndex === 1 ? "#241906" : skinIndex === 2 ? "#0d2613" : "#0f2419"
    readonly property color btnStopBg: "#261012"
    readonly property color btnResetBg: "#261b0b"
    readonly property color btnMaintBg: skinIndex === 1 ? "#1f1507" : skinIndex === 2 ? "#0e201d" : "#121e2b"

    function nextSkin() {
        skinIndex = (skinIndex + 1) % skinCount;
    }

    function stateColor(state: int, valid: bool): color {
        if (!valid || state === PlcTypes.Error)
            return statusError;
        if (state === PlcTypes.Maintenance)
            return statusMaintenance;
        if (state === PlcTypes.Processing)
            return statusProcessing;
        return statusIdle;
    }

    function stateBadgeText(state: int, valid: bool): string {
        if (!valid)
            return "[? INVALID]";
        if (state === PlcTypes.Error)
            return "[▲ FAULT!]";
        if (state === PlcTypes.Maintenance)
            return "[⚙ MAINT]";
        if (state === PlcTypes.Processing)
            return "[● RUNNING]";
        return "[■ STANDBY]";
    }

    function makeProgressBar(value: real, from: real, to: real, width: int): string {
        if (width <= 0 || to <= from)
            return "";
        const pct = Math.max(0.0, Math.min(1.0, (value - from) / (to - from)));
        const filled = Math.round(pct * width);
        return "█".repeat(filled) + "░".repeat(Math.max(0, width - filled));
    }

    function fitText(text: string, maxCells: int): string {
        if (maxCells <= 0)
            return "";
        if (text.length <= maxCells)
            return text;
        return text.substring(0, Math.max(0, maxCells - 1)) + "…";
    }
}
