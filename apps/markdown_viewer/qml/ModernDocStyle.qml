pragma ComponentBehavior: Bound

import QtQml
import QmlTui.Examples.MarkdownViewer

MdDocStyle {
    id: root

    h1Format: MdDocStyle.Badge
    h1BadgePadding: 1
    headingFormat: MdDocStyle.Bullets
    headingBanners: false
    codeBlockFormat: MdDocStyle.Indented
    codeBlockIndent: 2
    tableFormat: MdDocStyle.Open
    horizontalRuleFormat: MdDocStyle.Dashed
    horizontalRuleText: "--------"
    listBulletLevel1: "• "
    listBulletLevel2: "▪ "
    taskListChecked: "[✓] "
    taskListUnchecked: "[ ] "
    blockquotePrefix: "│ "
}
