pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Multi-instance Hex + ASCII MDI window wrapping BinaryDocument and HexGridView.
TvWindow {
    id: hexWin

    property bool editMode: false

    readonly property bool isHexWindow: true
    readonly property BinaryDocument document: doc
    readonly property string filePath: doc.filePath
    readonly property string displayName: doc.displayName
    readonly property bool isModified: doc.isModified

    function openSource(path: string): bool {
        hexWin.editMode = false;
        return doc.loadFile(path);
    }

    function saveBuffer(): bool {
        return doc.save();
    }

    function saveBufferAs(targetPath: string): bool {
        return doc.saveAs(targetPath);
    }

    function toggleEditMode() {
        hexWin.editMode = !hexWin.editMode;
        hexGrid.forceActiveFocus();
    }

    title: doc.displayName + " [" + doc.formatBadge + "]" + (doc.isModified ? " *" : "")
    footerText: doc.cursorHex + " (" + doc.cursorOffset + "/" + doc.byteCount + " B) │ " + (hexGrid.asciiActive ? "ASCII" : "HEX") + (hexWin.editMode ? " [EDIT]" : " [VIEW]")
    paletteRole: "window"
    interceptClose: doc.isModified
    minWidth: 46
    minHeight: 9
    preferredFocusItem: hexGrid

    BinaryDocument {
        id: doc
    }

    Column {
        id: winCol

        anchors.fill: parent
        spacing: 0

        // 1-row Hex Window status/mode bar
        Rectangle {
            width: winCol.width
            height: 1
            color: TvTheme.barBg

            Row {
                x: 1
                spacing: 2

                Text {
                    text: hexWin.editMode ? "[F4: EDIT]" : "[F4: VIEW]"
                    color: hexWin.editMode ? TvTheme.barHotkeyFg : TvTheme.barFg
                    backgroundColor: TvTheme.barBg
                    bold: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _event => hexWin.toggleEditMode()
                    }
                }

                Text {
                    text: doc.littleEndian ? "[Endian: LE]" : "[Endian: BE]"
                    color: TvTheme.barFg
                    backgroundColor: TvTheme.barBg

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _event => {
                            doc.littleEndian = !doc.littleEndian;
                        }
                    }
                }

                Text {
                    text: hexGrid.asciiActive ? "[Tab: ASCII]" : "[Tab: HEX]"
                    color: TvTheme.barFg
                    backgroundColor: TvTheme.barBg

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _event => {
                            hexGrid.asciiActive = !hexGrid.asciiActive;
                            hexGrid.forceActiveFocus();
                        }
                    }
                }

                Text {
                    visible: doc.selectionLength > 0
                    text: "Sel: " + doc.selectionLength + " B"
                    color: TvTheme.barHotkeyFg
                    backgroundColor: TvTheme.barBg
                    bold: true
                }
            }
        }

        HexGridView {
            id: hexGrid

            objectName: "hexGrid_" + hexWin.windowId
            width: winCol.width
            height: Math.max(1, winCol.height - 1)
            document: doc
            editMode: hexWin.editMode
            backgroundColor: TvTheme.winBg
            headerColor: TvTheme.winActiveBorder
            offsetColor: TvTheme.winMutedFg
            byteColor: TvTheme.winFg
            modifiedColor: "#ffff55"
            selectionBg: TvTheme.listSelectedBg
            selectionFg: TvTheme.listSelectedFg
            cursorBg: hexWin.editMode ? "#ffaa00" : TvTheme.buttonFocusedBg
            cursorFg: "#000000"
        }
    }
}
