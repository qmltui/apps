pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Examples.MarkdownViewer

Rectangle {
    id: editor

    required property var theme
    required property MdDocStyle docStyle
    property int selectedRow: 0

    readonly property var hrPresets: ["───────── ❖ ─────────", "────────────────────", "--------------------", "════════════════════", "· · · · · · · · · · ", "* * * * * * * * * * ", "- - - - - - - - - - ", "~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ", "- - - - - ◈ - - - - -"]
    property int hrPresetIndex: 0

    readonly property var bullet1Presets: ["✦ ", "• ", "▸ ", "◆ ", "■ ", "- ", "❯ ", "→ ", "* "]
    property int bullet1Index: 0

    readonly property var bullet2Presets: ["▸ ", "▪ ", "▫ ", "* ", "› ", "– ", "- "]
    property int bullet2Index: 0

    readonly property var taskCheckedPresets: ["[✔] ", "[✓] ", "[x] ", "[X] ", "[+] ", "☑ ", "[ok] "]
    property int taskCheckedIndex: 0

    readonly property var taskUncheckedPresets: ["[ ] ", "( ) ", "☐ ", "[·] ", "[?] "]
    property int taskUncheckedIndex: 0

    readonly property var quotePresets: ["▍ ", "│ ", "┃ ", "▌ ", "▎ ", "> ", "» ", "  "]
    property int quoteIndex: 0

    readonly property int totalRows: 14

    function selectNext() {
        editor.selectedRow = (editor.selectedRow + 1) % editor.totalRows;
    }

    function selectPrev() {
        editor.selectedRow = (editor.selectedRow - 1 + editor.totalRows) % editor.totalRows;
    }

    function applyBasePreset(presetName: string) {
        if (presetName === "Modern") {
            editor.docStyle.h1Format = MdDocStyle.Badge;
            editor.docStyle.h1BadgePadding = 1;
            editor.docStyle.headingFormat = MdDocStyle.Bullets;
            editor.docStyle.headingBanners = false;
            editor.docStyle.codeBlockFormat = MdDocStyle.Indented;
            editor.docStyle.codeBlockIndent = 2;
            editor.docStyle.tableFormat = MdDocStyle.Open;
            editor.docStyle.horizontalRuleText = "--------";
            editor.docStyle.listBulletLevel1 = "• ";
            editor.docStyle.listBulletLevel2 = "▪ ";
            editor.docStyle.taskListChecked = "[✓] ";
            editor.docStyle.taskListUnchecked = "[ ] ";
            editor.docStyle.blockquotePrefix = "│ ";
        } else if (presetName === "Classic") {
            editor.docStyle.h1Format = MdDocStyle.Banner;
            editor.docStyle.h1BadgePadding = 0;
            editor.docStyle.headingFormat = MdDocStyle.Hashes;
            editor.docStyle.headingBanners = true;
            editor.docStyle.codeBlockFormat = MdDocStyle.Boxed;
            editor.docStyle.codeBlockIndent = 0;
            editor.docStyle.tableFormat = MdDocStyle.Boxed;
            editor.docStyle.horizontalRuleText = "════════════════════";
            editor.docStyle.listBulletLevel1 = "- ";
            editor.docStyle.listBulletLevel2 = "  * ";
            editor.docStyle.taskListChecked = "[x] ";
            editor.docStyle.taskListUnchecked = "[ ] ";
            editor.docStyle.blockquotePrefix = "> ";
        } else if (presetName === "Minimal") {
            editor.docStyle.h1Format = MdDocStyle.Plain;
            editor.docStyle.h1BadgePadding = 0;
            editor.docStyle.headingFormat = MdDocStyle.Plain;
            editor.docStyle.headingBanners = false;
            editor.docStyle.codeBlockFormat = MdDocStyle.Borderless;
            editor.docStyle.codeBlockIndent = 0;
            editor.docStyle.tableFormat = MdDocStyle.Minimal;
            editor.docStyle.horizontalRuleText = "---";
            editor.docStyle.listBulletLevel1 = "* ";
            editor.docStyle.listBulletLevel2 = "* ";
            editor.docStyle.taskListChecked = "[x] ";
            editor.docStyle.taskListUnchecked = "[ ] ";
            editor.docStyle.blockquotePrefix = "  ";
        } else {
            editor.docStyle.h1Format = MdDocStyle.Badge;
            editor.docStyle.h1BadgePadding = 3;
            editor.docStyle.headingFormat = MdDocStyle.Bullets;
            editor.docStyle.headingBanners = true;
            editor.docStyle.codeBlockFormat = MdDocStyle.Indented;
            editor.docStyle.codeBlockIndent = 4;
            editor.docStyle.tableFormat = MdDocStyle.Open;
            editor.docStyle.horizontalRuleText = "───────── ❖ ─────────";
            editor.docStyle.listBulletLevel1 = "✦ ";
            editor.docStyle.listBulletLevel2 = "▸ ";
            editor.docStyle.taskListChecked = "[✔] ";
            editor.docStyle.taskListUnchecked = "[ ] ";
            editor.docStyle.blockquotePrefix = "▍ ";
        }
    }

    function adjustSelected(delta: int) {
        switch (editor.selectedRow) {
        case 0:
            if (delta > 0)
                editor.applyBasePreset("Fancy");
            else
                editor.applyBasePreset("Modern");
            break;
        case 1:
            if (editor.docStyle.h1Format === MdDocStyle.Badge)
                editor.docStyle.h1Format = delta > 0 ? MdDocStyle.Banner : MdDocStyle.Plain;
            else if (editor.docStyle.h1Format === MdDocStyle.Banner)
                editor.docStyle.h1Format = delta > 0 ? MdDocStyle.Plain : MdDocStyle.Badge;
            else
                editor.docStyle.h1Format = delta > 0 ? MdDocStyle.Badge : MdDocStyle.Banner;
            break;
        case 2:
            editor.docStyle.h1BadgePadding = Math.max(0, Math.min(8, editor.docStyle.h1BadgePadding + delta));
            break;
        case 3:
            if (editor.docStyle.headingFormat === MdDocStyle.Bullets)
                editor.docStyle.headingFormat = delta > 0 ? MdDocStyle.Hashes : MdDocStyle.Plain;
            else if (editor.docStyle.headingFormat === MdDocStyle.Hashes)
                editor.docStyle.headingFormat = delta > 0 ? MdDocStyle.Plain : MdDocStyle.Bullets;
            else
                editor.docStyle.headingFormat = delta > 0 ? MdDocStyle.Bullets : MdDocStyle.Hashes;
            break;
        case 4:
            editor.docStyle.headingBanners = !editor.docStyle.headingBanners;
            break;
        case 5:
            if (editor.docStyle.codeBlockFormat === MdDocStyle.Indented)
                editor.docStyle.codeBlockFormat = delta > 0 ? MdDocStyle.Boxed : MdDocStyle.Borderless;
            else if (editor.docStyle.codeBlockFormat === MdDocStyle.Boxed)
                editor.docStyle.codeBlockFormat = delta > 0 ? MdDocStyle.Borderless : MdDocStyle.Indented;
            else
                editor.docStyle.codeBlockFormat = delta > 0 ? MdDocStyle.Indented : MdDocStyle.Boxed;
            break;
        case 6:
            editor.docStyle.codeBlockIndent = Math.max(0, Math.min(8, editor.docStyle.codeBlockIndent + delta));
            break;
        case 7:
            if (editor.docStyle.tableFormat === MdDocStyle.Open)
                editor.docStyle.tableFormat = delta > 0 ? MdDocStyle.Boxed : MdDocStyle.Minimal;
            else if (editor.docStyle.tableFormat === MdDocStyle.Boxed)
                editor.docStyle.tableFormat = delta > 0 ? MdDocStyle.Minimal : MdDocStyle.Open;
            else
                editor.docStyle.tableFormat = delta > 0 ? MdDocStyle.Open : MdDocStyle.Boxed;
            break;
        case 8:
            editor.hrPresetIndex = (editor.hrPresetIndex + delta + editor.hrPresets.length) % editor.hrPresets.length;
            editor.docStyle.horizontalRuleText = editor.hrPresets[editor.hrPresetIndex];
            break;
        case 9:
            editor.bullet1Index = (editor.bullet1Index + delta + editor.bullet1Presets.length) % editor.bullet1Presets.length;
            editor.docStyle.listBulletLevel1 = editor.bullet1Presets[editor.bullet1Index];
            break;
        case 10:
            editor.bullet2Index = (editor.bullet2Index + delta + editor.bullet2Presets.length) % editor.bullet2Presets.length;
            editor.docStyle.listBulletLevel2 = editor.bullet2Presets[editor.bullet2Index];
            break;
        case 11:
            editor.taskCheckedIndex = (editor.taskCheckedIndex + delta + editor.taskCheckedPresets.length) % editor.taskCheckedPresets.length;
            editor.docStyle.taskListChecked = editor.taskCheckedPresets[editor.taskCheckedIndex];
            break;
        case 12:
            editor.taskUncheckedIndex = (editor.taskUncheckedIndex + delta + editor.taskUncheckedPresets.length) % editor.taskUncheckedPresets.length;
            editor.docStyle.taskListUnchecked = editor.taskUncheckedPresets[editor.taskUncheckedIndex];
            break;
        case 13:
            editor.quoteIndex = (editor.quoteIndex + delta + editor.quotePresets.length) % editor.quotePresets.length;
            editor.docStyle.blockquotePrefix = editor.quotePresets[editor.quoteIndex];
            break;
        }
    }

    color: editor.theme.surface
    border {
        style: Border.Single
        color: editor.theme.accent
    }

    Column {
        id: mainColumn

        anchors.fill: parent
        spacing: 0

        Rectangle {
            width: mainColumn.width
            height: 2
            color: editor.theme.bg

            Text {
                anchors.fill: parent
                text: " ⚙ LIVE STYLE EDITOR "
                color: editor.theme.accent
                bold: true
                verticalAlignment: Qt.AlignVCenter
            }
        }

        Text {
            width: mainColumn.width
            text: " [↑/↓] Select  [←/→/Enter] Tweak"
            color: editor.theme.textMuted
        }

        Rectangle {
            width: mainColumn.width
            height: 1
            color: editor.theme.border
        }

        Column {
            id: optionsColumn

            width: mainColumn.width
            spacing: 0

            Repeater {
                model: [
                    {
                        label: "1. Quick Base",
                        value: "Modern / Cyber"
                    },
                    {
                        label: "2. H1 Format",
                        value: (editor.docStyle.h1Format === MdDocStyle.Badge ? "Badge" : (editor.docStyle.h1Format === MdDocStyle.Banner ? "Banner" : "Plain"))
                    },
                    {
                        label: "3. H1 Padding",
                        value: editor.docStyle.h1BadgePadding + " sp"
                    },
                    {
                        label: "4. Headings",
                        value: (editor.docStyle.headingFormat === MdDocStyle.Bullets ? "Bullets" : (editor.docStyle.headingFormat === MdDocStyle.Hashes ? "Hashes" : "Plain"))
                    },
                    {
                        label: "5. H-Banners",
                        value: editor.docStyle.headingBanners ? "ON" : "OFF"
                    },
                    {
                        label: "6. Code Format",
                        value: (editor.docStyle.codeBlockFormat === MdDocStyle.Indented ? "Indented" : (editor.docStyle.codeBlockFormat === MdDocStyle.Boxed ? "Boxed" : "Borderless"))
                    },
                    {
                        label: "7. Code Indent",
                        value: editor.docStyle.codeBlockIndent + " sp"
                    },
                    {
                        label: "8. Table Format",
                        value: (editor.docStyle.tableFormat === MdDocStyle.Open ? "Open" : (editor.docStyle.tableFormat === MdDocStyle.Boxed ? "Boxed" : "Minimal"))
                    },
                    {
                        label: "9. Divider HR",
                        value: editor.docStyle.horizontalRuleText.substring(0, Math.min(8, editor.docStyle.horizontalRuleText.length)) + "…"
                    },
                    {
                        label: "10. Bullet L1",
                        value: "'" + editor.docStyle.listBulletLevel1 + "'"
                    },
                    {
                        label: "11. Bullet L2",
                        value: "'" + editor.docStyle.listBulletLevel2 + "'"
                    },
                    {
                        label: "12. Task [x]",
                        value: "'" + editor.docStyle.taskListChecked.trim() + "'"
                    },
                    {
                        label: "13. Task [ ]",
                        value: "'" + editor.docStyle.taskListUnchecked.trim() + "'"
                    },
                    {
                        label: "14. Quote Pre",
                        value: "'" + editor.docStyle.blockquotePrefix + "'"
                    }
                ]

                Rectangle {
                    id: rowRect

                    required property int index
                    required property var modelData

                    width: optionsColumn.width
                    height: 1
                    color: (rowRect.index === editor.selectedRow) ? editor.theme.accentSecondary : "transparent"

                    Row {
                        anchors {
                            fill: parent
                            leftMargin: 1
                            rightMargin: 1
                        }

                        Text {
                            width: 15
                            text: (rowRect.index === editor.selectedRow ? "▶ " : "  ") + rowRect.modelData.label
                            color: (rowRect.index === editor.selectedRow) ? editor.theme.bg : editor.theme.textPrimary
                            bold: (rowRect.index === editor.selectedRow)
                        }

                        Text {
                            text: " ‹ " + rowRect.modelData.value + " ›"
                            color: (rowRect.index === editor.selectedRow) ? editor.theme.bg : editor.theme.warning
                            bold: (rowRect.index === editor.selectedRow)
                        }
                    }
                }
            }
        }
    }
}
