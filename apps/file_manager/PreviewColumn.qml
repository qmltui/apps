pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: root

    required property var previewData
    property var backend: null

    onWidthChanged: {
        if (root.backend && root.width > 0) {
            root.backend.previewWidth = Math.max(10, root.width - 4);
        }
    }

    color: Theme.bgColumnDim
    border.style: Border.None

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
                textFormat: Text.PlainText
                color: Theme.textPrimary
                text: root.previewData ? (root.previewData.formattedText || root.previewData.rawText || (root.previewData.textLines ? root.previewData.textLines.join("\n") : "")) : ""
            }
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
                    Text { text: "📁 " + (root.previewData ? root.previewData.name : ""); color: Theme.accentBlue; bold: true }
                    Text { text: "(" + (root.previewData ? root.previewData.itemCount : 0) + " items)"; color: Theme.textSecondary }
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
                width: parent.width
                spacing: 0

                Text {
                    text: root.previewData && root.previewData.type === "image" ? "🖼️ Image Asset" : "📦 Binary / Archive"
                    color: Theme.accentPink
                    bold: true
                }

                Text { text: "File: " + (root.previewData ? root.previewData.name : ""); color: Theme.textPrimary; bold: true }
                Text { text: "MIME: " + (root.previewData ? root.previewData.mimeType : ""); color: Theme.accentCyan }
                Text { text: "Size: " + (root.previewData ? root.previewData.sizeFormatted : ""); color: Theme.textSecondary }
                Text { text: "Perms: " + (root.previewData ? root.previewData.permissions : "") + "  Owner: " + (root.previewData ? root.previewData.owner : ""); color: Theme.textDim }
            }

            Text {
                text: root.previewData && root.previewData.type === "image" ? "[ Image Content Preview ]" : "[ Binary Data Block ]"
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
