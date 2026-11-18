pragma ComponentBehavior: Bound

import QtQml
import QmlTui.Examples.MarkdownViewer

MdDocStyle {
    id: root

    h1Format: MdDocStyle.Banner
    h1BadgePadding: 0
    headingFormat: MdDocStyle.Hashes
    headingBanners: true
    codeBlockFormat: MdDocStyle.Boxed
    codeBlockIndent: 0
    tableFormat: MdDocStyle.Boxed
    horizontalRuleFormat: MdDocStyle.FullWidth
    listBulletLevel1: "- "
    listBulletLevel2: "  * "
    taskListChecked: "[x] "
    taskListUnchecked: "[ ] "
    blockquotePrefix: "> "
}
