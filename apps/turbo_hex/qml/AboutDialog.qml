pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Centered Borland "About Turbo Hex" modal TvDialog.
TvDialog {
    id: aboutDialog

    objectName: "aboutDialog"
    title: "About Turbo Hex"
    width: aboutDialog.parent ? Math.min(58, Math.max(38, aboutDialog.parent.width - 4)) : 54
    height: 14
    initialFocusItem: okButton

    Column {
        id: aboutCol

        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Text {
            width: aboutCol.width
            horizontalAlignment: Qt.AlignHCenter
            text: "QmlTui Turbo Hex v0.6"
            color: TvTheme.dialogFg
            backgroundColor: TvTheme.dialogBg
            bold: true
        }

        Text {
            width: aboutCol.width
            horizontalAlignment: Qt.AlignHCenter
            text: "Multi-Window Hex & Binary Inspector on QmlTui.TVision"
            color: TvTheme.dialogMutedFg
            backgroundColor: TvTheme.dialogBg
        }

        Rectangle {
            width: aboutCol.width
            height: 5
            color: TvTheme.listBg

            Column {
                anchors.centerIn: parent
                spacing: 0

                Text {
                    text: " F3 Open Binary      │  F4 Toggle Overwrite Edit    "
                    color: TvTheme.listFg
                    backgroundColor: TvTheme.listBg
                }
                Text {
                    text: " Ctrl+F Find Hex/Str │  Ctrl+G Jump to Offset       "
                    color: TvTheme.listFg
                    backgroundColor: TvTheme.listBg
                }
                Text {
                    text: " Tab Hex ⇄ ASCII     │  F7 Inspector  F8 Structure  "
                    color: TvTheme.listFg
                    backgroundColor: TvTheme.listBg
                }
                Text {
                    text: " F10 / Plain Menus   │  F9 Theme      Alt+X Exit    "
                    color: TvTheme.listFg
                    backgroundColor: TvTheme.listBg
                }
            }
        }

        TvButton {
            id: okButton

            anchors.horizontalCenter: parent.horizontalCenter
            width: 14
            text: "~O~K"
            isDefault: true
            surfaceColor: TvTheme.dialogBg
            onClicked: aboutDialog.accept()
        }
    }
}
