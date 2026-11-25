pragma ComponentBehavior: Bound

import QmlTui
import QmlTui.Markdown

Column {
    id: agentCard

    required property string content
    required property string title
    required property string extra

    width: parent ? parent.width : 80
    spacing: 0

    ModernDocStyle {
        id: responseDocStyle

        h1Format: MdDocStyle.Plain
        headingFormat: MdDocStyle.Hashes
        codeBlockFormat: MdDocStyle.Indented
        codeBlockIndent: 2
        wrapCodeBlocks: true
        tableFormat: MdDocStyle.Open
        wrapTableCells: true
    }

    Row {
        spacing: 1

        Text {
            text: "✦ " + (agentCard.title || "Agent")
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " (" + (agentCard.extra || "2.1k tokens · 0.8s") + ")"
            color: Theme.muted
        }
    }

    MdBlock {
        x: 2
        width: Math.max(1, agentCard.width - 2)
        markdown: agentCard.content
        docStyle: responseDocStyle
        color: Theme.white
        palette {
            text: Theme.white
            textMuted: Theme.muted
            heading: Theme.cyan
            heading2: Theme.cyan
            heading3: Theme.violet
            heading4: Theme.green
            heading5: Theme.amber
            code: Theme.amber
            inlineCode: Theme.cyan
            inlineCodeBg: Theme.hoverBg
            quote: Theme.violet
            border: Theme.muted
            link: Theme.cyan
            success: Theme.green
            warning: Theme.amber
            syntaxKeyword: Theme.violet
            syntaxType: Theme.cyan
            syntaxFunction: Theme.green
            syntaxString: Theme.amber
            syntaxComment: Theme.muted
            syntaxNumber: Theme.rose
            syntaxPreprocessor: Theme.violet
            syntaxSymbol: Theme.cyan
        }
    }
}
