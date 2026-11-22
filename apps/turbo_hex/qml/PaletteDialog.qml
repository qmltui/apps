pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import QmlTui.TVision

// Modal Turbo Vision palette & desktop stipple configurator.
TvDialog {
    id: paletteDialog

    readonly property var patternLabels: ["Light Shade (░)", "Medium Shade (▒)", "Center Dot (·)", "Solid Fill ( )"]

    objectName: "paletteDialog"
    title: "Colors & Desktop"
    width: paletteDialog.parent ? Math.min(52, Math.max(36, paletteDialog.parent.width - 4)) : 50
    height: paletteDialog.parent ? Math.min(14, Math.max(12, paletteDialog.parent.height - 2)) : 14

    ButtonGroup {
        id: schemeGroup
    }

    Column {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Row {
            width: parent.width
            spacing: 2

            Column {
                width: Math.max(20, Math.floor(parent.width * 0.58))
                spacing: 0

                Text {
                    text: "Color Palette:"
                    color: TvTheme.dialogFg
                    backgroundColor: TvTheme.dialogBg
                    bold: true
                }

                Repeater {
                    model: TvTheme.schemeDescriptions

                    delegate: RadioButton {
                        id: schemeRadio

                        required property var modelData
                        required property int index

                        text: String(schemeRadio.modelData)
                        textColor: TvTheme.dialogFg
                        buttonGroup: schemeGroup
                        checked: !TvTheme.hasCustomPalette && TvTheme.schemeIndex === schemeRadio.index
                        onClicked: TvTheme.setSchemeByIndex(schemeRadio.index)
                    }
                }
            }

            Column {
                id: patternColumn

                width: Math.max(12, parent.width - Math.floor(parent.width * 0.58) - 2)
                spacing: 0

                Text {
                    text: "Desktop Pattern:"
                    color: TvTheme.dialogFg
                    backgroundColor: TvTheme.dialogBg
                    bold: true
                }

                Repeater {
                    model: paletteDialog.patternLabels

                    delegate: Item {
                        id: patRow

                        required property var modelData
                        required property int index

                        readonly property bool activePat: (TvTheme.patternIndex % 4) === patRow.index

                        width: patternColumn.width
                        height: 1

                        Text {
                            width: parent.width
                            clip: true
                            text: (patRow.activePat ? "[x] " : "[ ] ") + String(patRow.modelData)
                            color: patRow.activePat ? TvTheme.listSelectedFg : TvTheme.dialogFg
                            backgroundColor: patRow.activePat ? TvTheme.listSelectedBg : TvTheme.dialogBg
                            bold: patRow.activePat
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: _event => {
                                TvTheme.patternIndex = patRow.index;
                            }
                        }
                    }
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            TvButton {
                width: 14
                text: "~O~K"
                isDefault: true
                surfaceColor: TvTheme.dialogBg
                onClicked: paletteDialog.accept()
            }

            TvButton {
                width: 16
                text: "Cycle ~N~ext"
                surfaceColor: TvTheme.dialogBg
                onClicked: TvTheme.cycleScheme()
            }
        }
    }
}
