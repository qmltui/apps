pragma ComponentBehavior: Bound

import QtQml
import QmlTui
import QmlTui.TVision

Terminal {
    id: root

    property var lastHexWindow: hexWin1
    property var pendingCloseWindow: null
    property bool pendingQuit: false

    readonly property var hexPool: [hexWin1, hexWin2, hexWin3]
    readonly property var currentHexWindow: (desktop.activeWindow && Boolean(desktop.activeWindow["isHexWindow"])) ? desktop.activeWindow : null
    readonly property var targetHexWindow: (root.currentHexWindow && root.currentHexWindow.visible) ? root.currentHexWindow : ((root.lastHexWindow && root.lastHexWindow.visible) ? root.lastHexWindow : null)
    readonly property BinaryDocument activeDocument: root.targetHexWindow ? root.targetHexWindow.document : null
    readonly property string topBarRightText: {
        const avail = root.width - 54;
        const withTheme = "Turbo Hex · " + TvTheme.schemeTitle;
        if (withTheme.length <= avail)
            return withTheme;
        return "Turbo Hex";
    }
    readonly property string rightTelemetry: {
        const doc = root.activeDocument;
        return doc ? (doc.formatBadge + " · " + doc.cursorHex) : "Idle";
    }

    function modifiedHexWindows(): var {
        let dirty = [];
        for (let i = 0; i < root.hexPool.length; ++i) {
            const hw = root.hexPool[i];
            if (hw.visible && hw.isModified)
                dirty.push(hw);
        }
        return dirty;
    }

    function requestQuit() {
        const dirty = root.modifiedHexWindows();
        if (dirty.length === 0) {
            Qt.quit();
            return;
        }
        root.pendingQuit = true;
        root.pendingCloseWindow = null;
        unsavedDialog.targetDescription = dirty.length === 1 ? dirty[0].displayName : (dirty.length + " modified buffers");
        unsavedDialog.open();
    }

    function openBinaryInWorkspace(path: string): var {
        for (let i = 0; i < root.hexPool.length; ++i) {
            const hw = root.hexPool[i];
            if (hw.visible && hw.filePath === path) {
                desktop.activateWindow(hw);
                root.lastHexWindow = hw;
                return hw;
            }
        }

        let slot = null;
        for (let i = 0; i < root.hexPool.length; ++i) {
            if (!root.hexPool[i].visible) {
                slot = root.hexPool[i];
                if (i > 0) {
                    const rightEdge = inspectorWindow.visible ? Math.max(48, inspectorWindow.x - 1) : Math.max(50, desktop.width - 4);
                    slot.setRestoredGeometry(2 * i, 2 * i, Math.min(rightEdge, desktop.width - 2 * i - 2), Math.max(12, desktop.height - 2 * i - 2));
                }
                break;
            }
        }
        if (!slot)
            slot = root.hexPool[root.hexPool.length - 1];

        if (!slot.openSource(path))
            return null;

        slot.visible = true;
        root.lastHexWindow = slot;
        desktop.activateWindow(slot);
        return slot;
    }

    function toggleWindow(win: var) {
        if (win.visible && desktop.activeWindow === win) {
            desktop.closeWindow(win);
        } else {
            win.visible = true;
            win.clampToDesktop(desktop.width, desktop.height);
            desktop.activateWindow(win);
        }
    }

    function setupInitialWorkspace() {
        const availW = Math.max(60, root.width);
        const availH = Math.max(14, root.height - 2);
        const wide = availW >= 96;

        if (wide) {
            const sideW = Math.min(40, Math.max(32, Math.floor(availW * 0.34)));
            const hexW = Math.max(48, availW - sideW - 3);
            const inspH = Math.min(13, Math.max(9, Math.floor(availH * 0.45)));
            hexWin1.setRestoredGeometry(0, 0, hexW, availH - 1);
            inspectorWindow.visible = true;
            inspectorWindow.setRestoredGeometry(hexW + 1, 0, sideW, inspH);
            structureWindow.visible = true;
            structureWindow.setRestoredGeometry(hexW + 1, inspH, sideW, Math.max(8, availH - inspH - 1));
        } else {
            hexWin1.setRestoredGeometry(0, 0, Math.max(46, availW - 2), availH - 1);
            inspectorWindow.visible = false;
            structureWindow.visible = false;
        }

        desktop.wireChildren();
        const startFiles = HexWorkspace.initialFiles;
        for (let i = 0; i < Math.min(startFiles.length, root.hexPool.length); ++i)
            root.openBinaryInWorkspace(String(startFiles[i]));
        if (hexWin1.visible)
            desktop.activateWindow(hexWin1);
    }

    function dispatchAction(actionId: string) {
        switch (actionId) {
        case "file.open":
            fileOpenDialog.open();
            break;
        case "file.openSampleElf":
            root.openBinaryInWorkspace("demo://firmware.elf");
            break;
        case "file.openSamplePng":
            root.openBinaryInWorkspace("demo://badge.png");
            break;
        case "file.save":
            if (root.targetHexWindow)
                root.targetHexWindow.saveBuffer();
            break;
        case "file.saveAs":
            if (root.targetHexWindow) {
                saveAsDialog.suggestedPath = root.targetHexWindow.displayName;
                saveAsDialog.open();
            }
            break;
        case "file.close":
            desktop.closeActive();
            break;
        case "file.exit":
            root.requestQuit();
            break;
        case "edit.toggleOverwrite":
            if (root.targetHexWindow) {
                desktop.activateWindow(root.targetHexWindow);
                root.targetHexWindow.toggleEditMode();
            }
            break;
        case "edit.undo":
            if (root.activeDocument)
                root.activeDocument.undo();
            break;
        case "edit.redo":
            if (root.activeDocument)
                root.activeDocument.redo();
            break;
        case "search.findHex":
            if (root.activeDocument) {
                findDialog.hexMode = true;
                findDialog.open();
            }
            break;
        case "search.findText":
            if (root.activeDocument) {
                findDialog.hexMode = false;
                findDialog.open();
            }
            break;
        case "search.goto":
            if (root.activeDocument)
                goToOffsetDialog.open();
            break;
        case "view.toggleEndian":
            if (root.activeDocument)
                root.activeDocument.littleEndian = !root.activeDocument.littleEndian;
            break;
        case "view.inspector":
            root.toggleWindow(inspectorWindow);
            break;
        case "view.structure":
            root.toggleWindow(structureWindow);
            break;
        case "win.zoom":
            desktop.toggleZoomActive();
            break;
        case "win.next":
            desktop.cycleNext();
            break;
        case "win.prev":
            desktop.cyclePrevious();
            break;
        case "win.move":
            desktop.enterMoveResizeActive();
            break;
        case "win.tile":
            desktop.tile();
            break;
        case "win.cascade":
            desktop.cascade();
            break;
        case "win.closeAll":
            desktop.closeAll();
            break;
        case "options.palette":
            paletteDialog.open();
            break;
        case "options.cycleTheme":
            TvTheme.cycleScheme();
            break;
        case "options.cyclePattern":
            TvTheme.cyclePattern();
            break;
        case "help.about":
        case "help.shortcuts":
            aboutDialog.open();
            break;
        }
    }

    onCurrentHexWindowChanged: {
        if (root.currentHexWindow)
            root.lastHexWindow = root.currentHexWindow;
    }

    Component.onCompleted: root.setupInitialWorkspace()

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            if (menuBar.barActive) {
                menuBar.closeMenu();
            } else if (desktop.activeWindow && desktop.activeWindow.moveResizeMode) {
                desktop.activeWindow.moveResizeMode = false;
            } else if (root.targetHexWindow && root.targetHexWindow.editMode) {
                root.targetHexWindow.editMode = false;
            } else {
                root.requestQuit();
            }
            event.accepted = true;
            return;
        }
        if ((event.modifiers & (Qt.ControlModifier | Qt.AltModifier)) === 0 && event.text && event.text.length === 1) {
            // In default Read-Only inspection mode, plain single-letter mnemonics (`F`, `E`, `S`, `V`, `W`, `O`, `H`) open menus directly!
            if (menuBar.tryTriggerMnemonic(event.text)) {
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Q) {
                root.requestQuit();
                event.accepted = true;
            }
        }
    }

    // Global Shortcuts
    Shortcut {
        sequence: "Alt+X"
        onActivated: root.requestQuit()
    }
    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: root.requestQuit()
    }
    Shortcut {
        sequence: "F1"
        onActivated: root.dispatchAction("help.about")
    }
    Shortcut {
        sequence: "F2"
        onActivated: root.dispatchAction("file.save")
    }
    Shortcut {
        sequence: "Shift+F2"
        onActivated: root.dispatchAction("file.saveAs")
    }
    Shortcut {
        sequence: "F3"
        onActivated: root.dispatchAction("file.open")
    }
    Shortcut {
        sequence: "F4"
        onActivated: root.dispatchAction("edit.toggleOverwrite")
    }
    Shortcut {
        sequence: "F5"
        onActivated: root.dispatchAction("win.zoom")
    }
    Shortcut {
        sequence: "F6"
        onActivated: root.dispatchAction("win.next")
    }
    Shortcut {
        sequence: "Shift+F6"
        onActivated: root.dispatchAction("win.prev")
    }
    Shortcut {
        sequence: "F7"
        onActivated: root.dispatchAction("view.inspector")
    }
    Shortcut {
        sequence: "F8"
        onActivated: root.dispatchAction("view.structure")
    }
    Shortcut {
        sequence: "F9"
        onActivated: root.dispatchAction("options.cycleTheme")
    }
    Shortcut {
        sequence: "Ctrl+F5"
        onActivated: root.dispatchAction("win.move")
    }
    Shortcut {
        sequence: "Alt+F3"
        onActivated: root.dispatchAction("file.close")
    }
    Shortcut {
        sequence: "Ctrl+F"
        onActivated: root.dispatchAction("search.findHex")
    }
    Shortcut {
        sequence: "Ctrl+G"
        onActivated: root.dispatchAction("search.goto")
    }
    Shortcut {
        sequence: "Ctrl+Z"
        onActivated: root.dispatchAction("edit.undo")
    }
    Shortcut {
        sequence: "Ctrl+Y"
        onActivated: root.dispatchAction("edit.redo")
    }
    Shortcut {
        sequence: "Alt+1"
        onActivated: desktop.activateByNumber(1)
    }
    Shortcut {
        sequence: "Alt+2"
        onActivated: desktop.activateByNumber(2)
    }
    Shortcut {
        sequence: "Alt+3"
        onActivated: desktop.activateByNumber(3)
    }
    Shortcut {
        sequence: "Alt+4"
        onActivated: desktop.activateByNumber(4)
    }
    Shortcut {
        sequence: "Alt+5"
        onActivated: desktop.activateByNumber(5)
    }

    // Top 1-row MenuBar (50 cols total — fits 60x18 with 10 cols to spare)
    TvMenuBar {
        id: menuBar

        objectName: "topMenuBar"
        x: 0
        y: 0
        width: root.width
        rightStatusText: root.topBarRightText
        menus: [
            {
                title: "~F~ile",
                items: [
                    {
                        text: "~O~pen Binary...",
                        shortcut: "F3",
                        action: "file.open"
                    },
                    {
                        text: "Sample ~E~LF64 Firmware",
                        shortcut: "",
                        action: "file.openSampleElf"
                    },
                    {
                        text: "Sample ~P~NG Image",
                        shortcut: "",
                        action: "file.openSamplePng"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "~S~ave",
                        shortcut: "F2",
                        action: "file.save"
                    },
                    {
                        text: "Save ~A~s...",
                        shortcut: "Shift+F2",
                        action: "file.saveAs"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "~C~lose Window",
                        shortcut: "Alt+F3",
                        action: "file.close"
                    },
                    {
                        text: "E~x~it",
                        shortcut: "Alt+X",
                        action: "file.exit"
                    }
                ]
            },
            {
                title: "~E~dit",
                items: [
                    {
                        text: "Toggle ~O~verwrite Mode",
                        shortcut: "F4",
                        action: "edit.toggleOverwrite"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "~U~ndo Patch",
                        shortcut: "Ctrl+Z",
                        action: "edit.undo"
                    },
                    {
                        text: "~R~edo Patch",
                        shortcut: "Ctrl+Y",
                        action: "edit.redo"
                    }
                ]
            },
            {
                title: "~S~earch",
                items: [
                    {
                        text: "Find ~H~ex Bytes...",
                        shortcut: "Ctrl+F",
                        action: "search.findHex"
                    },
                    {
                        text: "Find ~A~SCII Text...",
                        shortcut: "",
                        action: "search.findText"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "~G~o to Offset...",
                        shortcut: "Ctrl+G",
                        action: "search.goto"
                    }
                ]
            },
            {
                title: "~V~iew",
                items: [
                    {
                        text: "Data ~I~nspector",
                        shortcut: "F7",
                        action: "view.inspector"
                    },
                    {
                        text: "~H~eaders & Strings",
                        shortcut: "F8",
                        action: "view.structure"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "Toggle ~E~ndianness (LE/BE)",
                        shortcut: "",
                        action: "view.toggleEndian"
                    }
                ]
            },
            {
                title: "~W~indow",
                items: [
                    {
                        text: "~Z~oom",
                        shortcut: "F5",
                        action: "win.zoom"
                    },
                    {
                        text: "~N~ext",
                        shortcut: "F6",
                        action: "win.next"
                    },
                    {
                        text: "~P~revious",
                        shortcut: "Shift+F6",
                        action: "win.prev"
                    },
                    {
                        text: "~S~ize/Move",
                        shortcut: "Ctrl+F5",
                        action: "win.move"
                    },
                    {
                        separator: true
                    },
                    {
                        text: "~T~ile",
                        shortcut: "",
                        action: "win.tile"
                    },
                    {
                        text: "C~a~scade",
                        shortcut: "",
                        action: "win.cascade"
                    },
                    {
                        text: "Cl~o~se All",
                        shortcut: "",
                        action: "win.closeAll"
                    }
                ]
            },
            {
                title: "~O~ptions",
                items: [
                    {
                        text: "~C~olors & Desktop...",
                        shortcut: "",
                        action: "options.palette"
                    },
                    {
                        text: "Cycle ~T~heme",
                        shortcut: "F9",
                        action: "options.cycleTheme"
                    },
                    {
                        text: "Cycle ~P~attern",
                        shortcut: "",
                        action: "options.cyclePattern"
                    }
                ]
            },
            {
                title: "~H~elp",
                items: [
                    {
                        text: "~K~eyboard Guide",
                        shortcut: "F1",
                        action: "help.shortcuts"
                    },
                    {
                        text: "~A~bout Turbo Hex...",
                        shortcut: "",
                        action: "help.about"
                    }
                ]
            }
        ]
        onActionTriggered: actionId => root.dispatchAction(actionId)
    }

    // Center Stippled MDI Desktop
    TvDesktop {
        id: desktop

        objectName: "mdiDesktop"
        x: 0
        y: 1
        width: root.width
        height: Math.max(1, root.height - 2)

        onCloseIntercepted: win => {
            const isHw = Boolean(win && win["isHexWindow"]);
            root.pendingQuit = false;
            root.pendingCloseWindow = win;
            unsavedDialog.targetDescription = isHw ? String(win.displayName) : String(win.title);
            unsavedDialog.open();
        }

        HexWindow {
            id: hexWin1

            objectName: "hexPrimary"
            windowId: "1"
            visible: false
        }

        HexWindow {
            id: hexWin2

            objectName: "hexSecondary"
            windowId: "2"
            visible: false
        }

        HexWindow {
            id: hexWin3

            objectName: "hexTertiary"
            windowId: "3"
            visible: false
        }

        InspectorWindow {
            id: inspectorWindow

            document: root.activeDocument
            visible: false
        }

        StructureWindow {
            id: structureWindow

            document: root.activeDocument
            visible: false
            onRangeActivated: (offset, length) => {
                if (root.targetHexWindow && root.activeDocument) {
                    root.activeDocument.selectRange(offset, length);
                }
            }
        }
    }

    // Bottom 1-row StatusLine
    TvStatusLine {
        id: statusLine

        objectName: "bottomStatusLine"
        x: 0
        y: Math.max(0, root.height - 1)
        width: root.width
        hints: root.width >= 78 ? [
            {
                key: "F3",
                label: "Open",
                action: "file.open"
            },
            {
                key: "F2",
                label: "Save",
                action: "file.save"
            },
            {
                key: "F4",
                label: "Edit",
                action: "edit.toggleOverwrite"
            },
            {
                key: "Ctrl+F",
                label: "Find",
                action: "search.findHex"
            },
            {
                key: "Ctrl+G",
                label: "Goto",
                action: "search.goto"
            },
            {
                key: "Alt+X",
                label: "Exit",
                action: "file.exit"
            }
        ] : [
            {
                key: "F3",
                label: "Open",
                action: "file.open"
            },
            {
                key: "F4",
                label: "Edit",
                action: "edit.toggleOverwrite"
            },
            {
                key: "Ctrl+G",
                label: "Goto",
                action: "search.goto"
            },
            {
                key: "Alt+X",
                label: "Exit",
                action: "file.exit"
            }
        ]
        statusText: root.rightTelemetry
        onActionTriggered: actionId => root.dispatchAction(actionId)
    }

    // Modal Dialogs
    FileOpenDialog {
        id: fileOpenDialog

        onFileChosen: filePath => root.openBinaryInWorkspace(filePath)
    }

    SaveAsDialog {
        id: saveAsDialog

        onSavePathConfirmed: targetPath => {
            if (root.targetHexWindow)
                root.targetHexWindow.saveBufferAs(targetPath);
        }
    }

    FindBytesDialog {
        id: findDialog

        document: root.activeDocument
    }

    GoToOffsetDialog {
        id: goToOffsetDialog

        document: root.activeDocument
        onOffsetSubmitted: offset => {
            if (root.activeDocument) {
                root.activeDocument.clearSelection();
                root.activeDocument.cursorOffset = offset;
            }
        }
    }

    UnsavedChangesDialog {
        id: unsavedDialog

        onSaveChosen: {
            if (root.pendingQuit) {
                const dirty = root.modifiedHexWindows();
                for (let i = 0; i < dirty.length; ++i)
                    dirty[i].saveBuffer();
                Qt.quit();
            } else if (root.pendingCloseWindow) {
                if (Boolean(root.pendingCloseWindow["isHexWindow"]))
                    root.pendingCloseWindow.saveBuffer();
                desktop.forceCloseWindow(root.pendingCloseWindow);
                root.pendingCloseWindow = null;
            }
        }
        onDiscardChosen: {
            if (root.pendingQuit) {
                Qt.quit();
            } else if (root.pendingCloseWindow) {
                desktop.forceCloseWindow(root.pendingCloseWindow);
                root.pendingCloseWindow = null;
            }
        }
    }

    PaletteDialog {
        id: paletteDialog
    }

    AboutDialog {
        id: aboutDialog
    }
}
