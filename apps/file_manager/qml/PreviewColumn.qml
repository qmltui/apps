pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Image
import QmlTui.Markdown

Rectangle {
    id: root

    required property var previewData
    property var backend: null
    property int renderModeIndex: 0

    readonly property int currentRenderMode: {
        if (root.renderModeIndex === 1)
            return Image.HalfBlock;
        if (root.renderModeIndex === 2)
            return Image.Braille;
        if (root.renderModeIndex === 3)
            return Image.ASCII;
        return Image.Auto;
    }

    readonly property string currentRenderModeName: {
        if (root.renderModeIndex === 1)
            return "HalfBlock";
        if (root.renderModeIndex === 2)
            return "Braille";
        if (root.renderModeIndex === 3)
            return "ASCII";
        return "Auto";
    }

    function cycleRenderMode(): string {
        root.renderModeIndex = (root.renderModeIndex + 1) % 4;
        return root.currentRenderModeName;
    }

    onWidthChanged: {
        if (root.backend && root.width > 0) {
            root.backend.previewWidth = Math.max(10, root.width - 4);
        }
    }

    onPreviewDataChanged: {
        if (root.previewData && root.previewData.type === "markdown") {
            mdPreview.scrollToLine(0);
        }
    }

    color: Theme.bgColumnDim
    border.style: Border.None

    ModernDocStyle {
        id: mdDocStyle

        wrapCodeBlocks: true
        wrapTableCells: true
    }

    MdViewStyle {
        id: mdViewStyle

        showLineNumbers: true
        palette {
            text: Theme.textPrimary
            textMuted: Theme.textMuted
            heading: Theme.textHighlight
            heading1Bg: "ansi:blue"
            heading2: Theme.accentBlue
            heading3: Theme.accentCyan
            heading4: Theme.accentGreen
            heading5: Theme.accentYellow
            code: Theme.accentPeach
            inlineCode: Theme.accentCyan
            inlineCodeBg: "default"
            quote: Theme.accentCyan
            border: Theme.borderNormal
            link: Theme.accentCyan
            success: Theme.accentGreen
            warning: Theme.accentYellow
            syntaxKeyword: Theme.accentMauve
            syntaxType: Theme.accentCyan
            syntaxFunction: Theme.accentBlue
            syntaxString: Theme.accentGreen
            syntaxComment: Theme.textMuted
            syntaxNumber: Theme.accentYellow
            syntaxPreprocessor: Theme.accentMauve
            syntaxSymbol: Theme.accentPink
        }
    }

    MdDoc {
        id: mdPreviewDoc

        text: (root.previewData && root.previewData.type === "markdown") ? (root.previewData.rawText || "") : ""
        style: mdDocStyle
    }

    // 1. Text File Preview
    Item {
        id: textPreviewPane
        visible: Boolean(root.previewData && root.previewData.type === "text")
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.bottom: parent.bottom
        clip: true

        ScrollView {
            id: textScrollView
            anchors.fill: parent
            clip: true
            focusPolicy: Qt.NoFocus

            Text {
                id: textParagraph
                width: textScrollView.width
                wrapMode: Text.NoWrap
                textFormat: Text.AnsiText
                color: Theme.textPrimary
                text: root.previewData ? (root.previewData.highlightedText || root.previewData.formattedText || root.previewData.rawText || (root.previewData.textLines ? root.previewData.textLines.join("\n") : "")) : ""
            }
        }
    }

    // 1b. Markdown Document Preview
    Item {
        id: markdownPreviewPane
        visible: Boolean(root.previewData && root.previewData.type === "markdown")
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.bottom: parent.bottom
        clip: true

        MdView {
            id: mdPreview

            anchors.fill: parent
            document: mdPreviewDoc
            style: mdViewStyle
        }
    }

    // 2. Directory Child Items Preview
    Item {
        id: dirPreviewPane
        visible: Boolean(root.previewData && root.previewData.type === "directory")
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.bottom: parent.bottom
        clip: true

        Column {
            anchors.fill: parent
            spacing: 1

            // Directory Info Header (borderless)
            Column {
                width: parent.width
                spacing: 0

                Row {
                    spacing: 1
                    Text {
                        text: "📁 " + (root.previewData ? root.previewData.name : "")
                        color: Theme.accentBlue
                        bold: true
                    }
                    Text {
                        text: "(" + (root.previewData ? root.previewData.itemCount : 0) + " items)"
                        color: Theme.textSecondary
                    }
                }

                Text {
                    text: "Modified: " + (root.previewData ? root.previewData.modifiedFormatted : "")
                    color: Theme.textDim
                }
            }

            // Child Entries List
            ListView {
                id: dirChildList
                width: parent.width
                height: Math.max(1, parent.height - 3)
                clip: true
                focusPolicy: Qt.NoFocus
                model: root.previewData ? root.previewData.dirChildren : []

                onFocusChanged: {
                    if (focus)
                        focus = false;
                }

                delegate: Rectangle {
                    id: dirChildDelegate

                    required property int index
                    required property var modelData

                    width: dirChildList.width
                    height: 1
                    color: QmlTui.transparent

                    Row {
                        x: 0
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Text {
                            text: dirChildDelegate.modelData.icon || "📄"
                            color: dirChildDelegate.modelData.iconColor || Theme.textPrimary
                        }

                        Text {
                            text: dirChildDelegate.modelData.name || ""
                            color: dirChildDelegate.modelData.isDir ? Theme.accentBlue : Theme.textPrimary
                        }
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: dirChildDelegate.modelData.sizeFormatted || ""
                        color: Theme.textDim
                    }
                }
            }
        }
    }

    // 3. Binary / Image / Archive Metadata (borderless)
    Item {
        id: binaryPreviewPane
        visible: Boolean(root.previewData && (root.previewData.type === "binary" || root.previewData.type === "image"))
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.bottom: parent.bottom

        Column {
            anchors.fill: parent
            spacing: 1

            Column {
                id: metadataHeader
                width: parent.width
                spacing: 0

                Text {
                    text: {
                        if (!root.previewData || root.previewData.type !== "image")
                            return "📦 Binary / Archive";
                        if (root.previewData.mediaKind === "pdf")
                            return "📕 Image Asset · PDF";
                        if (root.previewData.mediaKind === "svg")
                            return "📐 Image Asset · SVG";
                        return "🖼️ Image Asset";
                    }
                    color: Theme.accentPink
                    bold: true
                }

                Text {
                    text: "File: " + (root.previewData ? root.previewData.name : "")
                    color: Theme.textPrimary
                    bold: true
                }
                Text {
                    text: "MIME: " + (root.previewData ? root.previewData.mimeType : "")
                    color: Theme.accentCyan
                }
                Text {
                    text: {
                        const sz = root.previewData ? (root.previewData.sizeFormatted || "") : "";
                        const dims = root.previewData && root.previewData.dimensions ? (" · " + root.previewData.dimensions) : "";
                        return "Size: " + sz + dims;
                    }
                    color: Theme.textSecondary
                }
                Text {
                    text: "Perms: " + (root.previewData ? root.previewData.permissions : "") + "  Owner: " + (root.previewData ? root.previewData.owner : "")
                    color: Theme.textDim
                }
                Text {
                    visible: Boolean(root.previewData && root.previewData.type === "image")
                    text: {
                        const modeStr = "[m] Mode: " + root.currentRenderModeName;
                        const pageCount = (root.previewData && root.previewData.pageCount) ? root.previewData.pageCount : 1;
                        if (pageCount > 1) {
                            const curPage = (root.backend ? root.backend.previewPage : 0) + 1;
                            return modeStr + "  [[/]] Page " + curPage + "/" + pageCount;
                        }
                        return modeStr;
                    }
                    color: Theme.accentYellow
                }
            }

            Image {
                id: previewImage
                width: parent.width
                height: Math.max(4, parent.height - metadataHeader.height - 1)
                visible: Boolean(root.previewData && root.previewData.type === "image")
                retainWhileLoading: true
                fillMode: Image.PreserveAspectFit
                renderMode: root.currentRenderMode
                smooth: false
                source: {
                    if (!root.previewData || root.previewData.type !== "image" || !root.previewData.path)
                        return "";
                    const w = Math.max(16, previewImage.renderSize.width);
                    const h = Math.max(16, previewImage.renderSize.height);
                    const mockFlag = (root.backend && root.backend.isMock) ? "1" : "0";
                    const page = root.backend ? root.backend.previewPage : 0;
                    return "image://preview/render?path=" + encodeURIComponent(root.previewData.path) + "&mime=" + encodeURIComponent(root.previewData.mimeType || "") + "&page=" + page + "&mock=" + mockFlag + "&w=" + w + "&h=" + h;
                }
            }

            Text {
                visible: Boolean(root.previewData && root.previewData.type === "binary")
                text: "[ Binary Data Block ]"
                color: Theme.textMuted
            }
        }
    }

    // 4. Empty state
    Text {
        anchors.centerIn: parent
        visible: Boolean(!root.previewData || root.previewData.type === "empty" || (!root.previewData.type))
        text: "(No file selected)"
        color: Theme.textDim
    }
}
