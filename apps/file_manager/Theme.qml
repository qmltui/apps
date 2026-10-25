pragma Singleton
import QtQml
import QmlTui

QtObject {
    // Active Terminal Palette (SGR default + 16-color ANSI palette indices)
    readonly property color bgMain:       "default"
    readonly property color bgHeader:     "default"
    readonly property color bgPanel:      "default"
    readonly property color bgColumn:     "default"
    readonly property color bgColumnDim:  "default"
    readonly property color bgCursor:     "ansi:brightblack"
    readonly property color bgSelected:   "ansi:brightblack"
    readonly property color bgMarked:     "ansi:black"
    readonly property color bgModal:      "default"

    // Borders & Dividers
    readonly property color borderNormal: "ansi:brightblack"
    readonly property color borderActive: "ansi:brightblue"
    readonly property color borderMuted:  "ansi:brightblack"
    readonly property color borderYellow: "ansi:brightyellow"
    readonly property color borderGreen:  "ansi:brightgreen"
    readonly property color borderRed:    "ansi:brightred"

    // Foreground / Text
    readonly property color textPrimary:   "default"
    readonly property color textSecondary: "ansi:white"
    readonly property color textMuted:     "ansi:brightblack"
    readonly property color textDim:       "ansi:brightblack"
    readonly property color textHighlight: "ansi:brightwhite"
    readonly property color textBadge:     "ansi:black"

    // Accent Colors
    readonly property color accentBlue:   "ansi:brightblue"
    readonly property color accentCyan:   "ansi:brightcyan"
    readonly property color accentGreen:  "ansi:brightgreen"
    readonly property color accentYellow: "ansi:brightyellow"
    readonly property color accentPeach:  "ansi:yellow"
    readonly property color accentMauve:  "ansi:magenta"
    readonly property color accentRed:    "ansi:brightred"
    readonly property color accentPink:   "ansi:brightmagenta"

    // Status Badges
    readonly property color badgeNormal:  "ansi:brightblue"
    readonly property color badgeSelect:  "ansi:brightyellow"
    readonly property color badgeFilter:  "ansi:brightmagenta"
    readonly property color badgeCommand: "ansi:yellow"

    // Glyphs
    readonly property string iconFolder:   "📁"
    readonly property string iconFile:     "📄"
    readonly property string iconCpp:      "⚡"
    readonly property string iconRust:     "🦀"
    readonly property string iconPython:   "🐍"
    readonly property string iconConfig:   "⚙️"
    readonly property string iconArchive:  "📦"
    readonly property string iconImage:    "🖼️"
    readonly property string iconCheck:    "✓"
    readonly property string iconCursor:   "▸"
    readonly property string iconDisk:     "💾"
    readonly property string iconUser:     "👤"
}
