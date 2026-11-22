pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Unsaved byte-patch confirmation modal (`Save` / `Discard` / `Cancel`).
TvDialog {
    id: unsavedDialog

    property string targetDescription: "binary"

    signal saveChosen
    signal discardChosen

    objectName: "unsavedChangesDialog"
    title: "Unsaved Patched Bytes"
    width: unsavedDialog.parent ? Math.min(50, Math.max(36, unsavedDialog.parent.width - 4)) : 46
    height: 9
    initialFocusItem: saveBtn

    Column {
        id: promptCol

        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Text {
            width: promptCol.width
            horizontalAlignment: Qt.AlignHCenter
            clip: true
            text: "Save byte edits to " + unsavedDialog.targetDescription + "?"
            color: TvTheme.dialogFg
            backgroundColor: TvTheme.dialogBg
            bold: true
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 1

            TvButton {
                id: saveBtn

                width: 12
                text: "~S~ave"
                isDefault: true
                surfaceColor: TvTheme.dialogBg
                onClicked: {
                    unsavedDialog.accept();
                    unsavedDialog.saveChosen();
                }
            }

            TvButton {
                width: 13
                text: "~D~iscard"
                surfaceColor: TvTheme.dialogBg
                onClicked: {
                    unsavedDialog.accept();
                    unsavedDialog.discardChosen();
                }
            }

            TvButton {
                width: 12
                text: "~C~ancel"
                surfaceColor: TvTheme.dialogBg
                onClicked: unsavedDialog.reject()
            }
        }
    }
}
