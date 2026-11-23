pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: commandPalette

    property string filterText: ""
    property alias selectedIndex: commandModel.selectedIndex
    // Keyboard navigation moves the window; mouse selection keeps the
    // command under the pointer stationary until it is clicked.
    property alias windowStart: commandModel.windowStart
    readonly property alias currentCommand: commandModel.currentCommand

    signal commandSelected(string cmd)
    signal commandExecuted(string cmd)

    function open(): void {
        commandPalette.visible = true;
    }

    function close(): void {
        commandPalette.visible = false;
    }

    function selectNext(): void {
        if (commandModel.matchCount > 0) {
            commandModel.selectedIndex = (commandModel.selectedIndex + 1) % commandModel.matchCount;
            commandPalette.revealCurrent();
        }
    }

    function selectPrev(): void {
        if (commandModel.matchCount > 0) {
            commandModel.selectedIndex = (commandModel.selectedIndex - 1 + commandModel.matchCount) % commandModel.matchCount;
            commandPalette.revealCurrent();
        }
    }

    function revealCurrent(): void {
        commandModel.windowStart = Math.max(0, Math.min(commandModel.selectedIndex - 2, commandModel.matchCount - 5));
    }

    function chooseCurrent(): void {
        const chosen = commandModel.currentCommand;
        if (chosen.length > 0) {
            commandPalette.close();
            commandPalette.commandSelected(chosen);
        }
    }

    function executeCurrent(): void {
        const chosen = commandModel.currentCommand;
        if (chosen.length > 0) {
            commandPalette.close();
            Qt.callLater(() => {
                commandPalette.commandExecuted(chosen);
            });
        }
    }

    width: parent ? parent.width : 80
    spacing: 0
    visible: false

    CommandCatalogModel {
        id: commandModel
        filterText: commandPalette.filterText
    }

    // Header Info Bar
    Row {
        width: parent.width
        height: 1
        spacing: 1

        StatusBadge {
            text: "Commands " + (commandModel.matchCount > 0 ? "[" + (commandModel.selectedIndex + 1) + "/" + commandModel.matchCount + "]" : "")
            status: "info"
            variant: "subtle"
            showIcon: false
        }

        Text {
            text: "· [Tab/→] fill · [Enter] run · [↑/↓] select · [Esc] close"
            color: Theme.muted
        }
    }

    // Borderless Command Suggestions
    Repeater {
        // C++ CommandCatalogModel updates roles via dataChanged when windowStart
        // shifts, keeping active MouseAreas alive while handlers execute.
        model: commandModel
        delegate: Rectangle {
            id: commandEntry

            required property int actualIndex
            required property string cmd
            required property string desc

            readonly property bool isCurrent: commandEntry.actualIndex === commandModel.selectedIndex
            readonly property bool isHovered: cmdMouse.containsMouse

            width: commandPalette.width
            height: 1
            color: commandEntry.isHovered ? (cmdMouse.pressed ? Theme.pressedBg : Theme.hoverBg) : (commandEntry.isCurrent ? Theme.hoverBg : "transparent")

            Row {
                anchors.fill: parent
                spacing: 1

                Text {
                    text: (commandEntry.isCurrent || commandEntry.isHovered) ? "  ❯ " : "    "
                    color: commandEntry.isHovered ? "white" : Theme.cyan
                    bold: commandEntry.isCurrent || commandEntry.isHovered
                }

                Text {
                    text: commandEntry.cmd
                    color: (commandEntry.isCurrent || commandEntry.isHovered) ? "white" : Theme.cyan
                    bold: commandEntry.isCurrent || commandEntry.isHovered
                    underline: commandEntry.isHovered
                }

                Text {
                    text: "· " + commandEntry.desc
                    color: commandEntry.isHovered ? Theme.white : (commandEntry.isCurrent ? Theme.slate : Theme.muted)
                }
            }

            MouseArea {
                id: cmdMouse
                anchors.fill: parent
                hoverEnabled: true
                onEntered: commandModel.selectedIndex = commandEntry.actualIndex
                onMouseXChanged: {
                    if (cmdMouse.containsMouse)
                        commandModel.selectedIndex = commandEntry.actualIndex;
                }
                onMouseYChanged: {
                    if (cmdMouse.containsMouse)
                        commandPalette.selectedIndex = commandEntry.actualIndex;
                }
                onClicked: {
                    commandModel.selectedIndex = commandEntry.actualIndex;
                    commandPalette.executeCurrent();
                }
            }
        }
    }
}
