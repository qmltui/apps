pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Panel {
    id: root

    property int machineState: PlcTypes.Idle
    property int targetSpeed: 1500
    property double maxTempLimit: 85.0
    property bool tallLayout: false

    readonly property bool hasInputFocus: rpmInput.activeFocus || tempLimitInput.activeFocus

    signal targetSpeedRequested(string input)
    signal tempLimitRequested(string input)
    signal startRequested
    signal stopRequested
    signal resetRequested
    signal maintRequested

    onTargetSpeedChanged: {
        rpmInput.text = root.targetSpeed.toString();
    }

    onMaxTempLimitChanged: {
        tempLimitInput.text = root.maxTempLimit.toString();
    }

    color: HmiTheme.panelBg
    border.style: Border.Rounded
    border.color: root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal

    title: Text {
        text: "◆ CONTROLS"
        color: HmiTheme.textSecondary
        bold: true
    }

    titleTrailing: Text {
        text: root.machineState === PlcTypes.Error ? "LOCKED" : "LOCAL"
        color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
        bold: true
    }

    footer: Text {
        text: "TAB:Edit"
        color: HmiTheme.textDim
    }

    footerTrailing: Text {
        visible: root.width >= 25
        text: "ENT:Apply"
        color: HmiTheme.textDim
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        // Setpoint 1: RPM
        Column {
            width: parent.width
            spacing: 0

            Item {
                width: parent.width
                height: 1

                Text {
                    anchors.left: parent.left
                    text: root.width >= 27 ? "RPM Setpoint" : "Speed Set"
                    color: rpmInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.textSecondary
                    bold: true
                }

                Text {
                    anchors.right: parent.right
                    text: root.targetSpeed + " RPM"
                    color: HmiTheme.traceSpeed
                }
            }

            Row {
                width: parent.width
                height: 1
                spacing: 1

                Rectangle {
                    width: 3
                    height: 1
                    color: HmiTheme.panelAltBg

                    Text {
                        anchors.centerIn: parent
                        text: "-"
                        color: HmiTheme.textSecondary
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.targetSpeedRequested(String(Math.max(0, root.targetSpeed - 100)));
                            rpmInput.text = root.targetSpeed.toString();
                        }
                    }
                }

                Rectangle {
                    width: Math.max(6, root.width - 12)
                    height: 1
                    color: HmiTheme.recessedBg

                    Text {
                        anchors.left: parent.left
                        text: rpmInput.activeFocus ? "▸" : " "
                        color: HmiTheme.borderFocus
                        bold: true
                    }

                    TextInput {
                        id: rpmInput
                        anchors.left: parent.left
                        anchors.leftMargin: 1
                        anchors.right: parent.right
                        anchors.rightMargin: 1
                        height: 1
                        text: root.targetSpeed.toString()
                        color: HmiTheme.textPrimary
                        Keys.onUpPressed: {
                            root.targetSpeedRequested(String(Math.min(3000, root.targetSpeed + 100)));
                            rpmInput.text = root.targetSpeed.toString();
                        }
                        Keys.onDownPressed: {
                            root.targetSpeedRequested(String(Math.max(0, root.targetSpeed - 100)));
                            rpmInput.text = root.targetSpeed.toString();
                        }
                        onAccepted: {
                            root.targetSpeedRequested(rpmInput.text);
                            rpmInput.text = root.targetSpeed.toString();
                        }
                    }
                }

                Rectangle {
                    width: 3
                    height: 1
                    color: HmiTheme.panelAltBg

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: HmiTheme.textSecondary
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.targetSpeedRequested(String(Math.min(3000, root.targetSpeed + 100)));
                            rpmInput.text = root.targetSpeed.toString();
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
                style: "line"
                color: HmiTheme.traceSpeed
                trackColor: HmiTheme.borderSubtle
            }
        }

        // Setpoint 2: Safety Temp Limit
        Column {
            width: parent.width
            spacing: 0

            Item {
                width: parent.width
                height: 1

                Text {
                    anchors.left: parent.left
                    text: "Temp Limit"
                    color: tempLimitInput.activeFocus ? HmiTheme.borderFocus : HmiTheme.textSecondary
                    bold: true
                }

                Text {
                    anchors.right: parent.right
                    text: root.maxTempLimit.toFixed(1) + " °C"
                    color: HmiTheme.traceTemp
                }
            }

            Row {
                width: parent.width
                height: 1
                spacing: 1

                Rectangle {
                    width: 3
                    height: 1
                    color: HmiTheme.panelAltBg

                    Text {
                        anchors.centerIn: parent
                        text: "-"
                        color: HmiTheme.textSecondary
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.tempLimitRequested(Math.max(30.0, root.maxTempLimit - 5.0).toFixed(1));
                            tempLimitInput.text = root.maxTempLimit.toString();
                        }
                    }
                }

                Rectangle {
                    width: Math.max(6, root.width - 12)
                    height: 1
                    color: HmiTheme.recessedBg

                    Text {
                        anchors.left: parent.left
                        text: tempLimitInput.activeFocus ? "▸" : " "
                        color: HmiTheme.borderFocus
                        bold: true
                    }

                    TextInput {
                        id: tempLimitInput
                        anchors.left: parent.left
                        anchors.leftMargin: 1
                        anchors.right: parent.right
                        anchors.rightMargin: 1
                        height: 1
                        text: root.maxTempLimit.toString()
                        color: HmiTheme.textPrimary
                        Keys.onUpPressed: {
                            root.tempLimitRequested(Math.min(110.0, root.maxTempLimit + 2.5).toFixed(1));
                            tempLimitInput.text = root.maxTempLimit.toString();
                        }
                        Keys.onDownPressed: {
                            root.tempLimitRequested(Math.max(30.0, root.maxTempLimit - 2.5).toFixed(1));
                            tempLimitInput.text = root.maxTempLimit.toString();
                        }
                        onAccepted: {
                            root.tempLimitRequested(tempLimitInput.text);
                            tempLimitInput.text = root.maxTempLimit.toString();
                        }
                    }
                }

                Rectangle {
                    width: 3
                    height: 1
                    color: HmiTheme.panelAltBg

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: HmiTheme.textSecondary
                        bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.tempLimitRequested(Math.min(110.0, root.maxTempLimit + 5.0).toFixed(1));
                            tempLimitInput.text = root.maxTempLimit.toString();
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
                style: "line"
                color: HmiTheme.traceTemp
                trackColor: HmiTheme.borderSubtle
            }
        }

        // Tactile Pushbutton Command Matrix
        Column {
            width: parent.width
            spacing: 1

            Text {
                text: "▸ STATION COMMANDS"
                color: HmiTheme.textSecondary
                bold: true
            }

            Row {
                width: parent.width
                height: 3
                spacing: 1

                readonly property int btnHalf: Math.floor((Math.max(10, root.width - 4) - 1) / 2)
                readonly property int btnRem: Math.max(10, root.width - 4) - btnHalf - 1

                Button {
                    id: startBtn
                    width: parent.btnHalf
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: startBtn.width >= 13 ? "[S] START" : "S START"
                    textColor: root.machineState === PlcTypes.Error ? HmiTheme.textDim : HmiTheme.textSuccess
                    background: Rectangle {
                        border.style: startBtn.activeFocus ? Border.Double : Border.Rounded
                        border.color: startBtn.activeFocus ? HmiTheme.borderFocus : (root.machineState === PlcTypes.Processing ? HmiTheme.statusProcessing : HmiTheme.borderNormal)
                    }
                    onClicked: root.startRequested()
                }

                Button {
                    id: stopBtn
                    width: parent.btnRem
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: stopBtn.width >= 12 ? "[X] STOP" : "X STOP"
                    textColor: HmiTheme.textDanger
                    background: Rectangle {
                        border.style: stopBtn.activeFocus ? Border.Double : Border.Rounded
                        border.color: stopBtn.activeFocus ? HmiTheme.borderFocus : HmiTheme.borderNormal
                    }
                    onClicked: root.stopRequested()
                }
            }

            Row {
                width: parent.width
                height: 3
                spacing: 1

                readonly property int btnHalf: Math.floor((Math.max(10, root.width - 4) - 1) / 2)
                readonly property int btnRem: Math.max(10, root.width - 4) - btnHalf - 1

                Button {
                    id: resetBtn
                    width: parent.btnHalf
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: resetBtn.width >= 13 ? "[R] RESET" : "R RESET"
                    textColor: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textWarning
                    background: Rectangle {
                        border.style: resetBtn.activeFocus || root.machineState === PlcTypes.Error ? Border.Double : Border.Rounded
                        border.color: resetBtn.activeFocus ? HmiTheme.borderFocus : (root.machineState === PlcTypes.Error ? HmiTheme.borderError : HmiTheme.borderNormal)
                    }
                    onClicked: root.resetRequested()
                }

                Button {
                    id: maintBtn
                    width: parent.btnRem
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: maintBtn.width >= 13 ? "[M] MAINT" : "M MAINT"
                    textColor: root.machineState === PlcTypes.Maintenance ? HmiTheme.textWarning : HmiTheme.textHighlight
                    background: Rectangle {
                        border.style: maintBtn.activeFocus ? Border.Double : Border.Rounded
                        border.color: maintBtn.activeFocus ? HmiTheme.borderFocus : (root.machineState === PlcTypes.Maintenance ? HmiTheme.statusMaintenance : HmiTheme.borderNormal)
                    }
                    onClicked: root.maintRequested()
                }
            }
        }

        // Interlock Guard Checklist on tall viewports
        Panel {
            visible: root.tallLayout
            width: parent.width
            height: Math.max(3, parent.height - 18)
            color: HmiTheme.panelAltBg
            border.style: Border.Rounded
            border.color: HmiTheme.borderSubtle

            title: Text {
                text: "INTERLOCKS"
                color: HmiTheme.textDim
            }

            Column {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.rightMargin: 1
                spacing: 0

                Item {
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Thermal"
                        color: HmiTheme.textSecondary
                    }

                    Text {
                        anchors.right: parent.right
                        text: root.machineState === PlcTypes.Error ? "▲ TRIP" : "● OK"
                        color: root.machineState === PlcTypes.Error ? HmiTheme.textDanger : HmiTheme.textSuccess
                        bold: true
                    }
                }

                Item {
                    visible: parent.height >= 2
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Speed Gov"
                        color: HmiTheme.textSecondary
                    }

                    Text {
                        anchors.right: parent.right
                        text: "● OK"
                        color: HmiTheme.textSuccess
                    }
                }

                Item {
                    visible: parent.height >= 3
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Watchdog"
                        color: HmiTheme.textSecondary
                    }

                    Text {
                        anchors.right: parent.right
                        text: "● ARMED"
                        color: HmiTheme.textSuccess
                    }
                }

                Item {
                    visible: parent.height >= 4
                    width: parent.width
                    height: 1

                    Text {
                        anchors.left: parent.left
                        text: "Skin [K]"
                        color: HmiTheme.textDim
                    }

                    Text {
                        anchors.right: parent.right
                        text: HmiTheme.skinName
                        color: HmiTheme.textHighlight
                        bold: true
                    }
                }
            }
        }
    }
}
