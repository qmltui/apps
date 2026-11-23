pragma ComponentBehavior: Bound

import QmlTui

Popup {
    id: modelPicker

    property string currentModel: "claude-3-7-sonnet"
    property int selectedIndex: 0

    signal modelSelected(string modelId)
    signal switchToDiffRequested

    function selectNext(): void {
        if (modelCatalog.count > 0)
            modelPicker.selectedIndex = (modelPicker.selectedIndex + 1) % modelCatalog.count;
    }

    function selectPrev(): void {
        if (modelCatalog.count > 0)
            modelPicker.selectedIndex = (modelPicker.selectedIndex - 1 + modelCatalog.count) % modelCatalog.count;
    }

    function chooseCurrent(): void {
        const chosenId = modelCatalog.modelIdAt(modelPicker.selectedIndex);
        if (chosenId.length > 0) {
            modelPicker.modelSelected(chosenId);
            modelPicker.close();
        }
    }

    width: parent ? parent.width : 78
    height: 16
    clip: true
    modal: true
    dimBackground: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnOutsideClick

    onVisibleChanged: {
        if (modelPicker.visible) {
            const idx = modelCatalog.indexOfModel(modelPicker.currentModel);
            if (idx >= 0)
                modelPicker.selectedIndex = idx;
        }
    }

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: event => {
        if (event.key === Qt.Key_1) {
            modelPicker.selectedIndex = 0;
            modelPicker.chooseCurrent();
            event.accepted = true;
        } else if (event.key === Qt.Key_2) {
            modelPicker.selectedIndex = 1;
            modelPicker.chooseCurrent();
            event.accepted = true;
        } else if (event.key === Qt.Key_3) {
            modelPicker.selectedIndex = 2;
            modelPicker.chooseCurrent();
            event.accepted = true;
        } else if (event.key === Qt.Key_4) {
            modelPicker.selectedIndex = 3;
            modelPicker.chooseCurrent();
            event.accepted = true;
        } else if (event.key === Qt.Key_Up) {
            modelPicker.selectPrev();
            event.accepted = true;
        } else if (event.key === Qt.Key_Down) {
            modelPicker.selectNext();
            event.accepted = true;
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            modelPicker.chooseCurrent();
            event.accepted = true;
        } else if (event.key === Qt.Key_Escape || event.key === Qt.Key_F2) {
            modelPicker.close();
            event.accepted = true;
        } else if (event.key === Qt.Key_F3) {
            modelPicker.switchToDiffRequested();
            event.accepted = true;
        }
    }

    NeuralModelCatalog {
        id: modelCatalog
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.surface
    }

    Column {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0

        // Header Title
        Row {
            width: parent.width
            height: 1
            spacing: 1

            StatusBadge {
                text: "Models"
                status: "info"
                variant: "subtle"
                showIcon: false
            }

            Text {
                text: "Neural model selector"
                color: "white"
                bold: true
            }

            Text {
                text: " · [1-4/Enter] select · "
                color: Theme.muted
            }

            Rectangle {
                width: closeModelBtn.implicitWidth + 2
                height: 1
                color: closeModelMouse.containsMouse ? Theme.hoverBg : "transparent"

                Text {
                    id: closeModelBtn
                    anchors.centerIn: parent
                    text: "[Esc] close"
                    color: closeModelMouse.containsMouse ? "white" : Theme.muted
                    bold: closeModelMouse.containsMouse
                    underline: closeModelMouse.containsMouse
                }

                MouseArea {
                    id: closeModelMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: modelPicker.close()
                }
            }
        }

        // Divider
        Rectangle {
            width: parent.width
            height: 1
            color: "transparent"
        }

        // Model List Items
        Repeater {
            model: modelCatalog
            delegate: Rectangle {
                id: modelEntry

                required property int index
                required property string modelId
                required property string name
                required property string provider
                required property string context
                required property string rate
                required property string badge
                required property string badgeStatus
                required property string desc

                readonly property bool isSelected: modelEntry.index === modelPicker.selectedIndex
                readonly property bool isActive: modelEntry.modelId === modelPicker.currentModel
                readonly property bool isHovered: modelMouse.containsMouse

                width: parent.width
                height: 3
                color: modelEntry.isHovered ? (modelMouse.pressed ? Theme.pressedBg : Theme.hoverBg) : (modelEntry.isSelected ? Theme.hoverBg : "transparent")

                Column {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0

                    Row {
                        width: parent.width
                        height: 1
                        spacing: 1

                        Text {
                            text: (modelEntry.isSelected || modelEntry.isHovered ? "❯ [" : "  [") + (modelEntry.index + 1) + "] "
                            color: (modelEntry.isSelected || modelEntry.isHovered) ? Theme.cyan : Theme.muted
                            bold: modelEntry.isSelected || modelEntry.isHovered
                        }

                        Text {
                            text: modelEntry.name
                            color: modelEntry.isHovered ? "white" : (modelEntry.isSelected ? "white" : (modelEntry.isActive ? Theme.cyan : Theme.slate))
                            bold: modelEntry.isSelected || modelEntry.isActive || modelEntry.isHovered
                            underline: modelEntry.isHovered
                        }

                        Text {
                            text: "(" + modelEntry.provider + ")"
                            color: modelEntry.isHovered ? Theme.slate : Theme.muted
                        }

                        StatusBadge {
                            text: modelEntry.badge
                            status: modelEntry.badgeStatus
                            variant: "subtle"
                            showIcon: false
                        }

                        Text {
                            text: modelEntry.isActive ? "● active" : ""
                            color: Theme.green
                            bold: true
                        }
                    }

                    Row {
                        width: parent.width
                        height: 1
                        spacing: 1

                        Text {
                            text: "      " + modelEntry.desc
                            color: modelEntry.isHovered ? Theme.white : (modelEntry.isSelected ? Theme.slate : Theme.muted)
                        }
                    }

                    Row {
                        width: parent.width
                        height: 1
                        spacing: 1

                        Text {
                            text: "      Ctx: " + modelEntry.context + " · " + modelEntry.rate
                            color: modelEntry.isHovered ? Theme.slate : Theme.muted
                        }
                    }
                }

                MouseArea {
                    id: modelMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: modelPicker.selectedIndex = modelEntry.index
                    onClicked: {
                        modelPicker.selectedIndex = modelEntry.index;
                        modelPicker.chooseCurrent();
                    }
                }
            }
        }
    }
}
