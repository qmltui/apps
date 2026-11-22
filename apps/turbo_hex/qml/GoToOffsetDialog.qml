pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Modal `Ctrl+G` Go to Byte Offset dialog accepting hex (`0x78`, `78h`) or decimal (`120`).
TvDialog {
    id: gotoDialog

    property BinaryDocument document: null

    signal offsetSubmitted(int offset)

    function submit() {
        if (!gotoDialog.document)
            return;
        const parsed = gotoDialog.document.parseOffsetString(offsetInput.text);
        if (parsed < 0)
            return;
        gotoDialog.accept();
        gotoDialog.offsetSubmitted(parsed);
    }

    objectName: "goToOffsetDialog"
    title: "Go to Byte Offset"
    width: gotoDialog.parent ? Math.min(48, Math.max(34, gotoDialog.parent.width - 4)) : 44
    height: 10
    initialFocusItem: offsetInput

    onOpened: {
        offsetInput.text = gotoDialog.document ? gotoDialog.document.cursorHex : "0x00";
        offsetInput.selectAll();
    }

    Column {
        id: gotoCol

        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Text {
            width: gotoCol.width
            clip: true
            text: "Enter hex (0x78) or decimal (120) offset:"
            color: TvTheme.dialogMutedFg
            backgroundColor: TvTheme.dialogBg
        }

        Rectangle {
            width: gotoCol.width
            height: 1
            color: TvTheme.inputBg

            TextInput {
                id: offsetInput

                objectName: "offsetInput"
                anchors.fill: parent
                color: TvTheme.inputFg
                selectionColor: TvTheme.listSelectedBg
                selectedTextColor: TvTheme.listSelectedFg
                cursorBlinking: false
                onAccepted: gotoDialog.submit()
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            TvButton {
                width: 13
                text: "~J~ump"
                isDefault: true
                surfaceColor: TvTheme.dialogBg
                onClicked: gotoDialog.submit()
            }

            TvButton {
                width: 13
                text: "~C~ancel"
                surfaceColor: TvTheme.dialogBg
                onClicked: gotoDialog.reject()
            }
        }
    }
}
