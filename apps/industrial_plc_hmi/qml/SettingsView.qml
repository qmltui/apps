pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property int targetSpeed: 1500
    property double maxTempLimit: 85.0

    readonly property bool wide: width >= 94
    readonly property bool tall: height >= 21
    readonly property int innerWidth: Math.max(20, width - 4)
    readonly property int innerHeight: Math.max(14, height - 2)
    readonly property int leftColWidth: Math.floor((innerWidth - 1) * 0.52)
    readonly property bool hasInputFocus: setRpmInput.activeFocus || setTempInput.activeFocus

    signal targetSpeedRequested(string input)
    signal tempLimitRequested(string input)
    signal skinCycleRequested
    signal defaultsRequested

    onTargetSpeedChanged: {
        if (!setRpmInput.activeFocus)
            setRpmInput.text = root.targetSpeed.toString();
    }

    onMaxTempLimitChanged: {
        if (!setTempInput.activeFocus)
            setTempInput.text = root.maxTempLimit.toString();
    }

    anchors.fill: parent
    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: HmiTheme.borderNormal

    title: Text {
        text: "◆ SYSTEM PARAMETERS & SAFETY THRESHOLDS"
        color: HmiTheme.textHighlight
        bold: true
    }

    titleTrailing: Text {
        visible: root.wide
        text: "CALIBRATION CONSOLE"
        color: HmiTheme.textSecondary
    }

    footer: Text {
        text: "Tab: Switch Field · ↑↓: Step · Enter: Apply"
        color: HmiTheme.textWarning
    }

    footerTrailing: Text {
        visible: root.wide
        text: "[D] Factory Defaults"
        color: HmiTheme.textDim
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // LEFT COLUMN: Editable Setpoint Modules
        // Note: setRpmInput and setTempInput remain the sole StrongFocus controls
        // so Tab -> Tab predictably navigates field 1 -> field 2.
        Column {
            width: root.leftColWidth
            height: root.innerHeight
            spacing: 1

            Panel {
                width: parent.width
                height: root.tall ? 8 : 7
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: setRpmInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.borderSubtle

                title: Text {
                    text: root.wide ? "1. TARGET SPEED SETPOINT (RPM)" : "1. SPEED SETPOINT"
                    color: setRpmInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.textPrimary
                    bold: true
                }

                titleTrailing: Text {
                    text: root.targetSpeed + " RPM"
                    color: HmiTheme.traceSpeed
                    bold: true
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: 1

                    Row {
                        width: parent.width
                        height: 1
                        spacing: 1

                        Rectangle {
                            width: 5
                            height: 1
                            color: HmiTheme.panelBg

                            Text {
                                anchors.centerIn: parent
                                text: "[-]"
                                color: HmiTheme.textSecondary
                                bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    const nextVal = Math.max(0, root.targetSpeed - 100);
                                    root.targetSpeedRequested(String(nextVal));
                                }
                            }
                        }

                        Rectangle {
                            width: Math.max(8, parent.width - 12)
                            height: 1
                            color: HmiTheme.recessedBg

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 1
                                text: setRpmInput.activeFocus ? "▸" : " "
                                color: HmiTheme.borderFocus
                                bold: true
                            }

                            TextInput {
                                id: setRpmInput
                                anchors.left: parent.left
                                anchors.leftMargin: 3
                                anchors.right: rpmUnitLabel.left
                                anchors.rightMargin: 1
                                height: 1
                                text: root.targetSpeed.toString()
                                color: HmiTheme.textPrimary
                                Keys.onUpPressed: {
                                    const v = Math.min(3000, root.targetSpeed + 100);
                                    root.targetSpeedRequested(String(v));
                                    setRpmInput.text = root.targetSpeed.toString();
                                }
                                Keys.onDownPressed: {
                                    const v = Math.max(0, root.targetSpeed - 100);
                                    root.targetSpeedRequested(String(v));
                                    setRpmInput.text = root.targetSpeed.toString();
                                }
                                onAccepted: {
                                    root.targetSpeedRequested(setRpmInput.text);
                                    setRpmInput.text = root.targetSpeed.toString();
                                }
                            }

                            Text {
                                id: rpmUnitLabel
                                anchors.right: parent.right
                                anchors.rightMargin: 1
                                text: "RPM"
                                color: HmiTheme.textDim
                            }
                        }

                        Rectangle {
                            width: 5
                            height: 1
                            color: HmiTheme.panelBg

                            Text {
                                anchors.centerIn: parent
                                text: "[+]"
                                color: HmiTheme.textSecondary
                                bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    const nextVal = Math.min(3000, root.targetSpeed + 100);
                                    root.targetSpeedRequested(String(nextVal));
                                }
                            }
                        }
                    }

                    ProgressBar {
                        width: parent.width
                        height: 1
                        from: 0
                        to: 3000
                        value: root.targetSpeed
                        style: "segmented"
                        trackColor: HmiTheme.recessedBg
                        thresholds: [
                            {
                                value: 2200,
                                color: HmiTheme.traceSpeed
                            },
                            {
                                value: 2700,
                                color: HmiTheme.textWarning
                            },
                            {
                                value: 3000,
                                color: HmiTheme.statusError
                            }
                        ]
                    }

                    Row {
                        visible: root.tall
                        spacing: 1

                        Text {
                            text: "PRESETS:"
                            color: HmiTheme.textDim
                        }

                        Repeater {
                            model: [750, 1500, 2400]

                            delegate: Rectangle {
                                id: speedPreset
                                required property int modelData

                                width: 6
                                height: 1
                                color: root.targetSpeed === modelData ? HmiTheme.selectionBg : HmiTheme.recessedBg

                                Text {
                                    anchors.centerIn: parent
                                    text: String(speedPreset.modelData)
                                    color: root.targetSpeed === speedPreset.modelData ? HmiTheme.textHighlight : HmiTheme.textSecondary
                                    bold: root.targetSpeed === speedPreset.modelData
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.targetSpeedRequested(String(speedPreset.modelData))
                                }
                            }
                        }
                    }
                }
            }

            Panel {
                width: parent.width
                height: root.tall ? 8 : 7
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: setTempInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.borderSubtle

                title: Text {
                    text: root.wide ? "2. OVER-TEMP SAFETY LIMIT (°C)" : "2. OVER-TEMP LIMIT"
                    color: setTempInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.textPrimary
                    bold: true
                }

                titleTrailing: Text {
                    text: root.maxTempLimit.toFixed(1) + " °C"
                    color: HmiTheme.traceTemp
                    bold: true
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: 1

                    Row {
                        width: parent.width
                        height: 1
                        spacing: 1

                        Rectangle {
                            width: 5
                            height: 1
                            color: HmiTheme.panelBg

                            Text {
                                anchors.centerIn: parent
                                text: "[-]"
                                color: HmiTheme.textSecondary
                                bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    const nextVal = Math.max(30.0, root.maxTempLimit - 5.0);
                                    root.tempLimitRequested(nextVal.toFixed(1));
                                }
                            }
                        }

                        Rectangle {
                            width: Math.max(8, parent.width - 12)
                            height: 1
                            color: HmiTheme.recessedBg

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 1
                                text: setTempInput.activeFocus ? "▸" : " "
                                color: HmiTheme.borderFocus
                                bold: true
                            }

                            TextInput {
                                id: setTempInput
                                anchors.left: parent.left
                                anchors.leftMargin: 3
                                anchors.right: tempUnitLabel.left
                                anchors.rightMargin: 1
                                height: 1
                                text: root.maxTempLimit.toString()
                                color: HmiTheme.textPrimary
                                Keys.onUpPressed: {
                                    const v = Math.min(110.0, root.maxTempLimit + 2.5);
                                    root.tempLimitRequested(v.toFixed(1));
                                    setTempInput.text = root.maxTempLimit.toString();
                                }
                                Keys.onDownPressed: {
                                    const v = Math.max(30.0, root.maxTempLimit - 2.5);
                                    root.tempLimitRequested(v.toFixed(1));
                                    setTempInput.text = root.maxTempLimit.toString();
                                }
                                onAccepted: {
                                    root.tempLimitRequested(setTempInput.text);
                                    setTempInput.text = root.maxTempLimit.toString();
                                }
                            }

                            Text {
                                id: tempUnitLabel
                                anchors.right: parent.right
                                anchors.rightMargin: 1
                                text: "°C"
                                color: HmiTheme.textDim
                            }
                        }

                        Rectangle {
                            width: 5
                            height: 1
                            color: HmiTheme.panelBg

                            Text {
                                anchors.centerIn: parent
                                text: "[+]"
                                color: HmiTheme.textSecondary
                                bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    const nextVal = Math.min(110.0, root.maxTempLimit + 5.0);
                                    root.tempLimitRequested(nextVal.toFixed(1));
                                }
                            }
                        }
                    }

                    ProgressBar {
                        width: parent.width
                        height: 1
                        from: 30
                        to: 110
                        value: root.maxTempLimit
                        style: "segmented"
                        trackColor: HmiTheme.recessedBg
                        thresholds: [
                            {
                                value: 75,
                                color: HmiTheme.statusProcessing
                            },
                            {
                                value: 95,
                                color: HmiTheme.textWarning
                            },
                            {
                                value: 110,
                                color: HmiTheme.statusError
                            }
                        ]
                    }

                    Row {
                        visible: root.tall
                        spacing: 1

                        Text {
                            text: "PRESETS:"
                            color: HmiTheme.textDim
                        }

                        Repeater {
                            model: [65, 85, 100]

                            delegate: Rectangle {
                                id: tempPreset
                                required property int modelData

                                width: 6
                                height: 1
                                color: Math.abs(root.maxTempLimit - modelData) < 0.1 ? HmiTheme.selectionBg : HmiTheme.recessedBg

                                Text {
                                    anchors.centerIn: parent
                                    text: tempPreset.modelData + "°C"
                                    color: Math.abs(root.maxTempLimit - tempPreset.modelData) < 0.1 ? HmiTheme.textHighlight : HmiTheme.textSecondary
                                    bold: Math.abs(root.maxTempLimit - tempPreset.modelData) < 0.1
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.tempLimitRequested(String(tempPreset.modelData))
                                }
                            }
                        }
                    }
                }
            }

            Panel {
                visible: root.innerHeight >= 25
                width: parent.width
                height: Math.max(6, root.innerHeight - 18)
                color: HmiTheme.panelAltBg
                border.style: Border.Rounded
                border.color: HmiTheme.borderSubtle

                title: Text {
                    text: "3. DISPLAY SKIN & CONSOLE PROFILE"
                    color: HmiTheme.textSecondary
                    bold: true
                }

                titleTrailing: Text {
                    text: "[K]"
                    color: HmiTheme.textHighlight
                    bold: true
                }

                Column {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    spacing: 1

                    Text {
                        width: parent.width
                        clip: true
                        text: "Select operator palette or retro CRT skin:"
                        color: HmiTheme.textSecondary
                    }

                    Row {
                        spacing: 1

                        Repeater {
                            model: ["ISA-SLATE", "AMBER CRT", "PHOSPHOR"]

                            delegate: Rectangle {
                                id: skinChip
                                required property int index
                                required property string modelData
                                width: Math.max(12, Math.floor((root.leftColWidth - 6) / 3))
                                height: 3
                                color: HmiTheme.skinIndex === skinChip.index ? HmiTheme.selectionBg : HmiTheme.recessedBg
                                border.style: Border.Rounded
                                border.color: HmiTheme.skinIndex === skinChip.index ? HmiTheme.borderFocus : HmiTheme.borderSubtle

                                Text {
                                    anchors.centerIn: parent
                                    text: skinChip.modelData
                                    color: HmiTheme.skinIndex === skinChip.index ? HmiTheme.textHighlight : HmiTheme.textSecondary
                                    bold: HmiTheme.skinIndex === skinChip.index
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: HmiTheme.skinIndex = skinChip.index
                                }
                            }
                        }
                    }

                    Text {
                        visible: root.innerHeight >= 27
                        width: parent.width
                        clip: true
                        text: "Updates gauges, mimic pipes & traces live."
                        color: HmiTheme.textDim
                    }
                }
            }
        }

        // RIGHT COLUMN: Hardware Calibration & Appearance Spec
        Panel {
            width: Math.max(16, root.innerWidth - root.leftColWidth - 1)
            height: root.innerHeight
            color: HmiTheme.panelAltBg
            border.style: Border.Rounded
            border.color: HmiTheme.borderSubtle

            title: Text {
                text: "◆ CALIBRATION & SAFETY SPEC"
                color: HmiTheme.textSecondary
                bold: true
            }

            Column {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.rightMargin: 1
                spacing: 1

                Column {
                    width: parent.width
                    spacing: 0

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Max Speed Ceiling"
                            color: HmiTheme.textSecondary
                        }

                        Text {
                            anchors.right: parent.right
                            text: "3000 RPM"
                            color: HmiTheme.textPrimary
                            bold: true
                        }
                    }

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Thermal Trip Range"
                            color: HmiTheme.textSecondary
                        }

                        Text {
                            anchors.right: parent.right
                            text: "30–110 °C"
                            color: HmiTheme.textPrimary
                            bold: true
                        }
                    }

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Telemetry Scan Rate"
                            color: HmiTheme.textSecondary
                        }

                        Text {
                            anchors.right: parent.right
                            text: "1000 ms"
                            color: HmiTheme.textPrimary
                        }
                    }

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Interlock Policy"
                            color: HmiTheme.textSecondary
                        }

                        Text {
                            anchors.right: parent.right
                            text: "Latch [R]"
                            color: HmiTheme.textWarning
                        }
                    }

                    Item {
                        width: parent.width
                        height: 1

                        Text {
                            anchors.left: parent.left
                            text: "Analog Loop Scale"
                            color: HmiTheme.textSecondary
                        }

                        Text {
                            anchors.right: parent.right
                            text: "4–20 mA"
                            color: HmiTheme.textPrimary
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: HmiTheme.recessedBg

                    Text {
                        anchors.centerIn: parent
                        text: "[K] HMI Skin: " + HmiTheme.skinName
                        color: HmiTheme.textHighlight
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.skinCycleRequested()
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: HmiTheme.recessedBg

                    Text {
                        anchors.centerIn: parent
                        text: "[D] Restore Factory Calibration"
                        color: HmiTheme.textWarning
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.defaultsRequested()
                    }
                }

                Text {
                    visible: root.tall
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: "Safety trips immediately cut main drive contactor DO-03 and close process valves V-A / V-B until operator reset."
                    color: HmiTheme.textDim
                }
            }
        }
    }
}
