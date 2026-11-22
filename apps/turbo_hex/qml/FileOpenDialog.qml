pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Borland F3 "Open Binary File" modal TvDialog showing magic format badges.
TvDialog {
    id: openDialog

    property string currentDir: ""
    property var entries: []
    property int currentIndex: 0

    readonly property var selectedEntry: (openDialog.entries && openDialog.currentIndex >= 0 && openDialog.currentIndex < openDialog.entries.length) ? openDialog.entries[openDialog.currentIndex] : null
    readonly property string selectedMeta: {
        if (!openDialog.selectedEntry)
            return "No matching entries";
        const item = openDialog.selectedEntry;
        if (item.isDir)
            return "Directory — Press Enter to browse " + String(item.path || "");
        return String(item.name || "") + " · " + String(item.format || "BIN") + " · " + String(item.bytes || 0) + " bytes";
    }

    signal fileChosen(string filePath)

    function refreshEntries() {
        const list = HexWorkspace.listDirectory(openDialog.currentDir, maskInput.text);
        openDialog.entries = list;
        if (list.length === 0)
            openDialog.currentIndex = -1;
        else if (openDialog.currentIndex < 0 || openDialog.currentIndex >= list.length)
            openDialog.currentIndex = 0;
    }

    function activateSelection() {
        const entry = openDialog.selectedEntry;
        if (!entry)
            return;
        if (entry.isDir) {
            openDialog.currentDir = String(entry.path || "");
            openDialog.currentIndex = 0;
            openDialog.refreshEntries();
            return;
        }
        const chosen = String(entry.path || "");
        if (chosen.length === 0)
            return;
        openDialog.accept();
        openDialog.fileChosen(chosen);
    }

    objectName: "fileOpenDialog"
    title: "Open Binary File"
    width: openDialog.parent ? Math.min(60, Math.max(38, openDialog.parent.width - 4)) : 56
    height: openDialog.parent ? Math.min(17, Math.max(12, openDialog.parent.height - 2)) : 16
    initialFocusItem: fileListScope

    onOpened: {
        openDialog.currentDir = HexWorkspace.rootPath;
        maskInput.text = "*";
        openDialog.currentIndex = 0;
        openDialog.refreshEntries();
    }

    FocusScope {
        id: fileListScope

        anchors.fill: parent
        focus: true
        focusPolicy: Qt.StrongFocus

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Down && openDialog.entries.length > 0) {
                openDialog.currentIndex = (openDialog.currentIndex + 1) % openDialog.entries.length;
                event.accepted = true;
            } else if (event.key === Qt.Key_Up && openDialog.entries.length > 0) {
                openDialog.currentIndex = (openDialog.currentIndex - 1 + openDialog.entries.length) % openDialog.entries.length;
                event.accepted = true;
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                openDialog.activateSelection();
                event.accepted = true;
            }
        }

        Column {
            id: bodyCol

            anchors.fill: parent
            anchors.margins: 1
            spacing: 1

            Text {
                width: bodyCol.width
                clip: true
                text: "Dir: " + (openDialog.currentDir.length > 0 ? openDialog.currentDir : HexWorkspace.rootPath)
                color: TvTheme.dialogMutedFg
                backgroundColor: TvTheme.dialogBg
            }

            Row {
                width: bodyCol.width
                height: 1
                spacing: 1

                Text {
                    spans: TvMnemonic.buildSpans("~M~ask:", TvTheme.dialogFg, TvTheme.barHotkeyFg, TvTheme.dialogBg, false, TvTheme.isMonochrome)
                }

                Rectangle {
                    width: Math.max(12, bodyCol.width - 7)
                    height: 1
                    color: TvTheme.inputBg

                    TextInput {
                        id: maskInput

                        objectName: "fileMaskInput"
                        anchors.fill: parent
                        text: "*"
                        color: TvTheme.inputFg
                        selectionColor: TvTheme.listSelectedBg
                        selectedTextColor: TvTheme.listSelectedFg
                        cursorBlinking: false
                        onTextChanged: openDialog.refreshEntries()
                        onAccepted: openDialog.activateSelection()
                    }
                }
            }

            Row {
                width: bodyCol.width
                height: Math.max(4, bodyCol.height - 5)
                spacing: 2

                Rectangle {
                    id: listBox

                    width: Math.max(18, bodyCol.width - 16)
                    height: parent.height
                    color: TvTheme.listBg
                    clip: true

                    Column {
                        id: fileListColumn

                        width: listBox.width
                        y: {
                            const avail = Math.max(1, listBox.height);
                            if (openDialog.currentIndex < avail)
                                return 0;
                            return -(openDialog.currentIndex - avail + 1);
                        }

                        Repeater {
                            model: openDialog.entries

                            delegate: Item {
                                id: fileRow

                                required property var modelData
                                required property int index

                                readonly property bool selected: openDialog.currentIndex === fileRow.index
                                readonly property bool isFolder: Boolean(fileRow.modelData.isDir)
                                readonly property color rowBg: fileRow.selected ? TvTheme.listSelectedBg : TvTheme.listBg
                                readonly property color rowFg: fileRow.selected ? TvTheme.listSelectedFg : (fileRow.isFolder ? "#ffff55" : TvTheme.listFg)

                                width: fileListColumn.width
                                height: 1

                                Rectangle {
                                    anchors.fill: parent
                                    color: fileRow.rowBg

                                    Text {
                                        x: 1
                                        width: Math.max(0, fileRow.width - 2)
                                        clip: true
                                        text: (fileRow.isFolder ? "▸ " : "  ") + String(fileRow.modelData.name || "") + (fileRow.isFolder ? "" : ("  [" + String(fileRow.modelData.format || "BIN") + " · " + String(fileRow.modelData.bytes || 0) + " B]"))
                                        color: fileRow.rowFg
                                        backgroundColor: fileRow.rowBg
                                        bold: fileRow.selected || fileRow.isFolder
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: _event => {
                                        if (openDialog.currentIndex === fileRow.index)
                                            openDialog.activateSelection();
                                        else
                                            openDialog.currentIndex = fileRow.index;
                                    }
                                }
                            }
                        }
                    }
                }

                Column {
                    width: 14
                    spacing: 1

                    TvButton {
                        width: 13
                        text: "~O~pen"
                        isDefault: true
                        surfaceColor: TvTheme.dialogBg
                        enabled: openDialog.selectedEntry !== null
                        onClicked: openDialog.activateSelection()
                    }

                    TvButton {
                        width: 13
                        text: "~C~ancel"
                        surfaceColor: TvTheme.dialogBg
                        onClicked: openDialog.reject()
                    }
                }
            }

            Text {
                width: bodyCol.width
                clip: true
                text: openDialog.selectedMeta
                color: TvTheme.dialogMutedFg
                backgroundColor: TvTheme.dialogBg
            }
        }
    }
}
