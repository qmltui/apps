pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Headers & Printable Strings MDI window (`F8`). Clicking or pressing Enter on
// any parsed ELF/PNG/ZIP/WASM header field or ASCII string jumps and highlights
// its exact byte span in the active HexWindow.
TvWindow {
    id: structWin

    property BinaryDocument document: null
    property string filterKind: "all" // "all", "header", "string"
    property int currentIndex: 0

    readonly property var filteredItems: {
        const all = structWin.document ? structWin.document.structureItems : [];
        if (structWin.filterKind === "all")
            return all;
        let out = [];
        for (let i = 0; i < all.length; ++i) {
            if (String(all[i].kind || "") === structWin.filterKind)
                out.push(all[i]);
        }
        return out;
    }

    signal rangeActivated(int offset, int length)

    function jumpToSelected() {
        const list = structWin.filteredItems;
        if (!list || structWin.currentIndex < 0 || structWin.currentIndex >= list.length)
            return;
        const entry = list[structWin.currentIndex];
        structWin.rangeActivated(Number(entry.offset || 0), Number(entry.length || 1));
    }

    objectName: "structureWindow"
    title: "Headers & Strings (" + structWin.filteredItems.length + ")"
    footerText: "Enter/Click: Highlight Span"
    paletteRole: "dialog"
    width: 36
    height: 15
    minWidth: 28
    minHeight: 8
    preferredFocusItem: listFocus

    onDocumentChanged: {
        structWin.currentIndex = 0;
    }

    FocusScope {
        id: listFocus

        anchors.fill: parent
        focus: true
        focusPolicy: Qt.StrongFocus

        Keys.onPressed: event => {
            const count = structWin.filteredItems.length;
            if (event.key === Qt.Key_Down && count > 0) {
                structWin.currentIndex = Math.min(count - 1, structWin.currentIndex + 1);
                structWin.jumpToSelected();
                event.accepted = true;
            } else if (event.key === Qt.Key_Up && count > 0) {
                structWin.currentIndex = Math.max(0, structWin.currentIndex - 1);
                structWin.jumpToSelected();
                event.accepted = true;
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                structWin.jumpToSelected();
                event.accepted = true;
            }
        }

        Column {
            id: structCol

            anchors.fill: parent
            spacing: 0

            // Filter tab bar: [All] [Headers] [Strings]
            Rectangle {
                width: structCol.width
                height: 1
                color: TvTheme.barBg

                Row {
                    x: 1
                    spacing: 2

                    Text {
                        text: structWin.filterKind === "all" ? "[● All]" : "[All]"
                        color: structWin.filterKind === "all" ? TvTheme.barHotkeyFg : TvTheme.barFg
                        backgroundColor: TvTheme.barBg
                        bold: structWin.filterKind === "all"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: _event => {
                                structWin.filterKind = "all";
                                structWin.currentIndex = 0;
                            }
                        }
                    }

                    Text {
                        text: structWin.filterKind === "header" ? "[● Headers]" : "[Headers]"
                        color: structWin.filterKind === "header" ? TvTheme.barHotkeyFg : TvTheme.barFg
                        backgroundColor: TvTheme.barBg
                        bold: structWin.filterKind === "header"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: _event => {
                                structWin.filterKind = "header";
                                structWin.currentIndex = 0;
                            }
                        }
                    }

                    Text {
                        text: structWin.filterKind === "string" ? "[● Strings]" : "[Strings]"
                        color: structWin.filterKind === "string" ? TvTheme.barHotkeyFg : TvTheme.barFg
                        backgroundColor: TvTheme.barBg
                        bold: structWin.filterKind === "string"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: _event => {
                                structWin.filterKind = "string";
                                structWin.currentIndex = 0;
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: listViewport

                width: structCol.width
                height: Math.max(1, structCol.height - 1)
                color: TvTheme.listBg
                clip: true

                Text {
                    visible: structWin.filteredItems.length === 0
                    x: 1
                    y: 0
                    text: "No matching structure entries."
                    color: TvTheme.listFg
                    backgroundColor: TvTheme.listBg
                }

                Column {
                    id: itemsColumn

                    width: listViewport.width
                    y: {
                        const avail = Math.max(1, listViewport.height);
                        if (structWin.currentIndex < avail)
                            return 0;
                        return -(structWin.currentIndex - avail + 1);
                    }

                    Repeater {
                        model: structWin.filteredItems

                        delegate: Item {
                            id: itemRow

                            required property var modelData
                            required property int index

                            readonly property bool selected: structWin.currentIndex === itemRow.index
                            readonly property bool isHdr: String(itemRow.modelData.kind || "") === "header"
                            readonly property color bg: itemRow.selected ? TvTheme.listSelectedBg : TvTheme.listBg
                            readonly property color fg: itemRow.selected ? TvTheme.listSelectedFg : (itemRow.isHdr ? "#ffff55" : TvTheme.listFg)

                            width: itemsColumn.width
                            height: 1

                            Rectangle {
                                anchors.fill: parent
                                color: itemRow.bg

                                Text {
                                    x: 1
                                    width: Math.max(0, itemRow.width - 2)
                                    clip: true
                                    text: String(itemRow.modelData.offsetHex || "0x0000") + " " + String(itemRow.modelData.label || "") + " · " + String(itemRow.modelData.detail || "")
                                    color: itemRow.fg
                                    backgroundColor: itemRow.bg
                                    bold: itemRow.selected || itemRow.isHdr
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: _event => {
                                    listFocus.forceActiveFocus();
                                    structWin.currentIndex = itemRow.index;
                                    structWin.jumpToSelected();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
