pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Examples.MarkdownViewer 1.0

TextArea {
    id: root

    property MdDoc document: null
    property MdViewStyle style: null
    property bool showLineNumbers: root.style ? root.style.showLineNumbers : false
    property string searchQuery: ""
    property int activeMatchIndex: 0
    property int requestedScrollY: 0
    readonly property int scrollY: root.contentY
    readonly property int contentHeight: root.document ? root.document.lineCount : 0

    signal linkActivated(string url)

    function clampScroll(y: int): int {
        const maxScroll = Math.max(0, root.contentHeight - Math.max(0, root.height));
        return Math.max(0, Math.min(y, maxScroll));
    }

    function scrollToLine(lineIndex: int): void {
        const clamped = root.clampScroll(lineIndex);
        root.requestedScrollY = clamped;
        root.contentY = clamped;
    }

    function scrollBy(delta: int): void {
        root.scrollToLine(root.contentY + delta);
    }

    function pageUp(): void {
        root.scrollBy(-Math.max(1, root.height - 1));
    }

    function pageDown(): void {
        root.scrollBy(Math.max(1, root.height - 1));
    }

    function refreshDocumentHighlights(): void {
        if (!root.document || !root.style) {
            root.clearHighlightLayer("document");
            return;
        }
        root.setHighlightLayer("document", root.document.documentHighlights(root.style.palette), root.textRevision, 0);
    }

    function refreshSearchHighlights(): void {
        if (!root.document || !root.style || root.searchQuery.trim().length === 0) {
            root.clearHighlightLayer("search");
            root.clearHighlightLayer("active_search");
            return;
        }
        root.setHighlightLayer("search", root.document.searchHighlights(root.searchQuery, root.activeMatchIndex, root.style.palette, false), root.textRevision, 10);
        root.setHighlightLayer("active_search", root.document.searchHighlights(root.searchQuery, root.activeMatchIndex, root.style.palette, true), root.textRevision, 20);
    }

    function syncRenderedTextAndHighlights(): void {
        const savedScroll = root.requestedScrollY;
        root.text = root.document ? root.document.renderedText : "";
        const clamped = root.clampScroll(savedScroll);
        root.requestedScrollY = clamped;
        root.contentY = clamped;
        root.refreshDocumentHighlights();
        root.refreshSearchHighlights();
    }

    function syncDocument(): void {
        if (root.document && root.width > 0) {
            root.document.updateViewport(root.width, root.showLineNumbers);
        }
        root.syncRenderedTextAndHighlights();
    }

    implicitWidth: 80
    implicitHeight: 24
    focusPolicy: Qt.NoFocus
    readOnly: true
    cursorVisible: false

    Component.onCompleted: {
        root.syncDocument();
    }

    onDocumentChanged: {
        root.syncDocument();
    }

    onStyleChanged: {
        root.syncDocument();
    }

    onShowLineNumbersChanged: {
        root.syncDocument();
    }

    onWidthChanged: {
        root.syncDocument();
    }

    onHeightChanged: {
        const clamped = root.clampScroll(root.requestedScrollY);
        root.requestedScrollY = clamped;
        root.contentY = clamped;
    }

    onSearchQueryChanged: {
        root.refreshSearchHighlights();
    }

    onActiveMatchIndexChanged: {
        root.refreshSearchHighlights();
    }

    Connections {
        target: root.document
        function onParsedChanged(): void {
            root.syncRenderedTextAndHighlights();
        }
    }

    Connections {
        target: root.style
        function onStyleChanged(): void {
            root.syncDocument();
        }
        function onPaletteChanged(): void {
            root.refreshDocumentHighlights();
            root.refreshSearchHighlights();
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: event => {
            if (!root.document)
                return;
            const url = root.document.linkAt(event.x, event.y + root.contentY);
            if (url.length > 0) {
                root.linkActivated(url);
            }
        }

        onWheel: event => {
            if (event.wheelDeltaY > 0) {
                root.scrollBy(-3);
            } else if (event.wheelDeltaY < 0) {
                root.scrollBy(3);
            }
        }
    }
}
