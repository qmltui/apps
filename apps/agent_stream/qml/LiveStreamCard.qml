pragma ComponentBehavior: Bound

import QmlTui
import QmlTui.Markdown

// Live in-flight token stream rendered inside Terminal.Inline before the block
// completes and flushes permanently into Scrollback.
Column {
    id: liveCard

    required property int streamKind
    required property string title
    required property string content

    readonly property bool isThought: liveCard.streamKind === AgentSession.LiveThoughtStream

    width: parent ? parent.width : 80
    spacing: 0

    ModernDocStyle {
        id: liveDocStyle

        h1Format: MdDocStyle.Plain
        headingFormat: MdDocStyle.Hashes
        codeBlockFormat: MdDocStyle.Indented
        codeBlockIndent: 2
        wrapCodeBlocks: true
        tableFormat: MdDocStyle.Open
        wrapTableCells: true
    }

    Item {
        width: parent.width
        height: 1
    }

    Row {
        spacing: 1

        Text {
            text: liveCard.isThought ? "▼ Thought" : "✦ Agent"
            color: liveCard.isThought ? Theme.violet : Theme.cyan
            bold: true
        }
        Text {
            text: "(" + liveCard.title + ")"
            color: Theme.muted
        }
    }

    Text {
        visible: liveCard.isThought
        x: 2
        width: Math.max(1, liveCard.width - 2)
        text: liveCard.isThought ? (liveCard.content + "▍") : ""
        color: Theme.slate
        italic: true
    }

    MdBlock {
        visible: !liveCard.isThought
        x: 2
        width: Math.max(1, liveCard.width - 2)
        markdown: (!liveCard.isThought && liveCard.visible) ? (liveCard.content + "▍") : ""
        docStyle: liveDocStyle
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
