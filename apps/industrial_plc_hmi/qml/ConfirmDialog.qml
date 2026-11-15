pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.Controls
import IndustrialPlcHmi

Popup {
    id: root

    property string title: "CONFIRM ACTION"
    property string message: "Are you sure you want to proceed?"
    property bool faultMode: false

    signal confirmed
    signal cancelled

    width: Math.min(62, parent ? Math.max(38, parent.width - 6) : 62)
    height: 10
    anchors.centerIn: parent
    modal: true
    dimBackground: true
    closePolicy: Popup.CloseOnEscape
    initialFocusItem: confirmButton

    onAccepted: root.confirmed()
    onRejected: root.cancelled()

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Y) {
            root.accept();
            event.accepted = true;
        } else if (event.key === Qt.Key_N || event.key === Qt.Key_Escape) {
            root.reject();
            event.accepted = true;
        }
    }

    function showPrompt(dialogTitle: string, dialogMessage: string, isFault: bool) {
        root.title = dialogTitle;
        root.message = dialogMessage;
        root.faultMode = isFault;
        root.open();
    }

    Panel {
        anchors.fill: parent
        color: HmiTheme.panelBg
        border.style: Border.Double
        border.color: root.faultMode ? HmiTheme.borderError : HmiTheme.borderWarning

        title: Text {
            text: root.faultMode ? "▲ SAFETY INTERLOCK RESET" : "◆ OPERATOR INTERLOCK"
            color: root.faultMode ? HmiTheme.textDanger : HmiTheme.textWarning
            bold: true
        }

        footerTrailing: Text {
            text: "Y / N / ESC"
            color: HmiTheme.textDim
        }

        Column {
            anchors.fill: parent
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            spacing: 1

            Rectangle {
                width: parent.width
                height: 1
                color: root.faultMode ? HmiTheme.statusError : HmiTheme.statusMaintenance

                Text {
                    anchors.centerIn: parent
                    text: "▲ " + root.title
                    color: HmiTheme.badgeInk
                    bold: true
                }
            }

            Text {
                width: parent.width
                height: 2
                wrapMode: Text.WordWrap
                horizontalAlignment: Qt.AlignHCenter
                text: root.message
                color: HmiTheme.textPrimary
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 3

                Button {
                    id: confirmButton
                    width: 17
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: "[Y] CONFIRM"
                    textColor: root.faultMode ? HmiTheme.textDanger : HmiTheme.textSuccess
                    background: Rectangle {
                        color: root.faultMode ? HmiTheme.btnStopBg : HmiTheme.btnStartBg
                        border.style: confirmButton.activeFocus ? Border.Double : Border.Rounded
                        border.color: confirmButton.activeFocus ? HmiTheme.borderFocus : (root.faultMode ? HmiTheme.borderError : HmiTheme.statusProcessing)
                    }
                    onClicked: root.accept()
                }

                Button {
                    id: cancelButton
                    width: 17
                    height: 3
                    leftPadding: 1
                    rightPadding: 1
                    text: "[N] CANCEL"
                    textColor: HmiTheme.textSecondary
                    background: Rectangle {
                        color: HmiTheme.panelAltBg
                        border.style: cancelButton.activeFocus ? Border.Double : Border.Rounded
                        border.color: cancelButton.activeFocus ? HmiTheme.borderFocus : HmiTheme.borderNormal
                    }
                    onClicked: root.reject()
                }
            }
        }
    }
}
