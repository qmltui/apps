pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// "Save Binary As" modal dialog (`Shift+F2`).
TvDialog {
    id: saveAsDialog

    property string suggestedPath: ""

    signal savePathConfirmed(string targetPath)

    function submit() {
        const raw = pathInput.text.trim();
        if (raw.length === 0)
            return;
        const resolved = HexWorkspace.resolvePath(HexWorkspace.rootPath, raw);
        saveAsDialog.accept();
        saveAsDialog.savePathConfirmed(resolved);
    }

    objectName: "saveAsDialog"
    title: "Save Binary As"
    width: saveAsDialog.parent ? Math.min(54, Math.max(36, saveAsDialog.parent.width - 4)) : 50
    height: 10
    initialFocusItem: pathInput

    onOpened: {
        pathInput.text = saveAsDialog.suggestedPath.length > 0 ? saveAsDialog.suggestedPath : "patched.bin";
        pathInput.selectAll();
    }

    Column {
        id: saveCol

        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Text {
            width: saveCol.width
            clip: true
            text: "Base: " + HexWorkspace.rootPath
            color: TvTheme.dialogMutedFg
            backgroundColor: TvTheme.dialogBg
        }

        Row {
            width: saveCol.width
            height: 1
            spacing: 1

            Text {
                text: "Save as:"
                color: TvTheme.dialogFg
                backgroundColor: TvTheme.dialogBg
                bold: true
            }

            Rectangle {
                width: Math.max(12, saveCol.width - 9)
                height: 1
                color: TvTheme.inputBg

                TextInput {
                    id: pathInput

                    objectName: "saveAsInput"
                    anchors.fill: parent
                    color: TvTheme.inputFg
                    selectionColor: TvTheme.listSelectedBg
                    selectedTextColor: TvTheme.listSelectedFg
                    cursorBlinking: false
                    onAccepted: saveAsDialog.submit()
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            TvButton {
                width: 13
                text: "~S~ave"
                isDefault: true
                surfaceColor: TvTheme.dialogBg
                onClicked: saveAsDialog.submit()
            }

            TvButton {
                width: 13
                text: "~C~ancel"
                surfaceColor: TvTheme.dialogBg
                onClicked: saveAsDialog.reject()
            }
        }
    }
}
