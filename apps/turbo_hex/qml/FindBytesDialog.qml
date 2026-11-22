pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Modal `Ctrl+F` Find Hex Bytes or ASCII String dialog.
TvDialog {
    id: findDialog

    property BinaryDocument document: null
    property bool hexMode: true
    property string statusMessage: "Enter hex bytes (e.g. 7F 45 4C 46) or ASCII text"

    function runFind(fromCursor: bool) {
        if (!findDialog.document)
            return;
        const q = queryInput.text.trim();
        if (q.length === 0)
            return;
        const hit = findDialog.document.findPattern(q, findDialog.hexMode, fromCursor);
        if (hit >= 0) {
            findDialog.statusMessage = "Found at offset 0x" + hit.toString(16).toUpperCase();
            findDialog.accept();
        } else {
            findDialog.statusMessage = findDialog.hexMode ? "No match (check even hex digit pairs)" : "Text pattern not found";
        }
    }

    objectName: "findBytesDialog"
    title: "Find Hex Bytes / ASCII Text"
    width: findDialog.parent ? Math.min(56, Math.max(38, findDialog.parent.width - 4)) : 52
    height: 12
    initialFocusItem: queryInput

    onOpened: {
        findDialog.statusMessage = findDialog.hexMode ? "Hex example: 7F 45 4C 46 or 48 31 C0 C3" : "ASCII example: QmlTui or IHDR";
        queryInput.selectAll();
    }

    Column {
        id: findCol

        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Row {
            width: findCol.width
            height: 1
            spacing: 3

            Text {
                text: "Mode:"
                color: TvTheme.dialogFg
                backgroundColor: TvTheme.dialogBg
                bold: true
            }

            Text {
                text: (findDialog.hexMode ? "(●) " : "( ) ") + "Hex Bytes"
                color: findDialog.hexMode ? TvTheme.dialogControlFg : TvTheme.dialogFg
                backgroundColor: TvTheme.dialogBg
                bold: findDialog.hexMode

                MouseArea {
                    anchors.fill: parent
                    onClicked: _event => {
                        findDialog.hexMode = true;
                        findDialog.statusMessage = "Hex example: 7F 45 4C 46 or 48 31 C0 C3";
                    }
                }
            }

            Text {
                text: (!findDialog.hexMode ? "(●) " : "( ) ") + "ASCII Text"
                color: !findDialog.hexMode ? TvTheme.dialogControlFg : TvTheme.dialogFg
                backgroundColor: TvTheme.dialogBg
                bold: !findDialog.hexMode

                MouseArea {
                    anchors.fill: parent
                    onClicked: _event => {
                        findDialog.hexMode = false;
                        findDialog.statusMessage = "ASCII example: QmlTui or STATUS=READY";
                    }
                }
            }
        }

        Rectangle {
            width: findCol.width
            height: 1
            color: TvTheme.inputBg

            TextInput {
                id: queryInput

                objectName: "findQueryInput"
                anchors.fill: parent
                color: TvTheme.inputFg
                selectionColor: TvTheme.listSelectedBg
                selectedTextColor: TvTheme.listSelectedFg
                cursorBlinking: false
                onAccepted: findDialog.runFind(false)
            }
        }

        Text {
            width: findCol.width
            clip: true
            text: findDialog.statusMessage
            color: TvTheme.dialogMutedFg
            backgroundColor: TvTheme.dialogBg
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            TvButton {
                width: 14
                text: "~F~ind"
                isDefault: true
                surfaceColor: TvTheme.dialogBg
                onClicked: findDialog.runFind(false)
            }

            TvButton {
                width: 14
                text: "Find ~N~ext"
                surfaceColor: TvTheme.dialogBg
                onClicked: findDialog.runFind(true)
            }

            TvButton {
                width: 12
                text: "~C~lose"
                surfaceColor: TvTheme.dialogBg
                onClicked: findDialog.reject()
            }
        }
    }
}
