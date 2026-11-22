pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

// Live Data Inspector MDI window decoding integers, floats, bits, and ASCII
// at the active BinaryDocument's cursorOffset.
TvWindow {
    id: inspWin

    property BinaryDocument document: null

    readonly property var rows: inspWin.document ? inspWin.document.inspectorRows : []
    readonly property bool isLittleEndian: inspWin.document ? inspWin.document.littleEndian : true

    objectName: "inspectorWindow"
    title: "Data Inspector"
    footerText: inspWin.isLittleEndian ? "Little-Endian (x86/ARM)" : "Big-Endian (Network)"
    paletteRole: "dialog"
    width: 36
    height: 13
    minWidth: 28
    minHeight: 8

    Column {
        id: inspCol

        anchors.fill: parent
        spacing: 0

        Rectangle {
            width: inspCol.width
            height: 1
            color: TvTheme.barBg

            Row {
                x: 1
                spacing: 2

                Text {
                    text: inspWin.document ? (inspWin.document.displayName + " @ " + inspWin.document.cursorHex) : "No binary"
                    color: TvTheme.barFg
                    backgroundColor: TvTheme.barBg
                    bold: true
                }

                Text {
                    visible: inspWin.document !== null
                    text: inspWin.isLittleEndian ? "[LE ⇄ BE]" : "[BE ⇄ LE]"
                    color: TvTheme.barHotkeyFg
                    backgroundColor: TvTheme.barBg
                    bold: true

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _event => {
                            if (inspWin.document)
                                inspWin.document.littleEndian = !inspWin.document.littleEndian;
                        }
                    }
                }
            }
        }

        Rectangle {
            id: tableBg

            width: inspCol.width
            height: Math.max(1, inspCol.height - 1)
            color: TvTheme.dialogBg
            clip: true

            Column {
                id: rowsColumn

                width: tableBg.width
                spacing: 0

                Repeater {
                    model: inspWin.rows

                    delegate: Item {
                        id: rowItem

                        required property var modelData
                        required property int index

                        readonly property color rowBg: (rowItem.index % 2 === 0) ? TvTheme.dialogBg : TvTheme.listBg
                        readonly property color keyFg: (rowItem.index % 2 === 0) ? TvTheme.dialogMutedFg : TvTheme.listFg
                        readonly property color valFg: (rowItem.index % 2 === 0) ? TvTheme.dialogFg : TvTheme.listSelectedFg

                        width: rowsColumn.width
                        height: 1

                        Rectangle {
                            anchors.fill: parent
                            color: rowItem.rowBg

                            Text {
                                x: 1
                                width: 14
                                clip: true
                                text: String(rowItem.modelData.label || "")
                                color: rowItem.keyFg
                                backgroundColor: rowItem.rowBg
                            }

                            Text {
                                x: 16
                                width: Math.max(0, rowItem.width - 17)
                                clip: true
                                text: String(rowItem.modelData.value || "")
                                color: rowItem.valFg
                                backgroundColor: rowItem.rowBg
                                bold: true
                            }
                        }
                    }
                }
            }
        }
    }
}
