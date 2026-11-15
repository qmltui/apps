pragma ComponentBehavior: Bound
import QmlTui
import QmlTui.Controls

Panel {
    id: root

    required property PackageEngine engine

    readonly property int bodyRows: engine.showTreePreview ? Math.min(12, engine.treeRowCount) : Math.min(12, engine.totalPackages + 2)

    width: parent ? Math.min(parent.width, 104) : 96
    height: 4 + bodyRows
    color: "transparent"
    border.style: Border.Rounded
    border.color: Theme.cyan

    title: Text {
        spans: [
            {
                text: "⚡ Transaction Plan · ",
                color: Theme.cyan,
                bold: true
            },
            {
                text: root.engine.targetName + " v" + root.engine.targetVersion,
                color: Theme.ink,
                bold: true
            }
        ]
    }

    titleTrailing: Text {
        spans: [
            {
                text: root.engine.totalPackages + " pkgs · ",
                color: Theme.secondary
            },
            {
                text: root.engine.totalDownloadText + " fetch",
                color: Theme.cyan,
                bold: true
            },
            {
                text: " · +" + root.engine.totalInstallText,
                color: Theme.green,
                bold: true
            }
        ]
    }

    footer: Text {
        spans: [
            {
                text: "[Enter/y]",
                color: Theme.green,
                bold: true
            },
            {
                text: " Confirm  ",
                color: Theme.secondary
            },
            {
                text: "[1-3]",
                color: Theme.amber,
                bold: true
            },
            {
                text: " Toggle Feature  ",
                color: Theme.secondary
            },
            {
                text: "[d]",
                color: Theme.cyan,
                bold: true
            },
            {
                text: root.engine.showTreePreview ? " Table View  " : " Dep Tree  ",
                color: Theme.secondary
            },
            {
                text: "[q/Esc]",
                color: Theme.muted,
                bold: true
            },
            {
                text: " Cancel",
                color: Theme.muted
            }
        ]
    }

    Column {
        anchors.fill: parent
        spacing: 0

        Row {
            width: parent.width
            height: 1
            spacing: 2

            Text {
                text: " Features:"
                color: Theme.amber
                bold: true
            }

            CheckBox {
                checked: root.engine.ptyviewEnabled
                text: "1:ptyview (+2.1M)"
                textColor: checked ? Theme.green : Theme.muted
                onToggled: root.engine.ptyviewEnabled = checked
            }

            CheckBox {
                checked: root.engine.sixelEnabled
                text: "2:sixel (+4.4M)"
                textColor: checked ? Theme.green : Theme.muted
                onToggled: root.engine.sixelEnabled = checked
            }

            CheckBox {
                checked: root.engine.wasmEnabled
                text: "3:wasm-sdk (+11.4M)"
                textColor: checked ? Theme.purple : Theme.muted
                onToggled: root.engine.wasmEnabled = checked
            }
        }

        Text {
            width: parent.width
            height: 1
            text: "─".repeat(Math.max(0, parent.width))
            color: Theme.border
        }

        TableView {
            visible: !root.engine.showTreePreview
            width: parent.width
            height: root.bodyRows
            selectedRow: -1
            headerColor: Theme.cyan
            headerBackgroundColor: "transparent"
            showColumnGrid: false
            showRowGrid: false
            showOuterBorder: false
            alternatingRows: false
            stretchLastColumn: true
            model: root.engine.planModel

            columns: [
                {
                    title: " Package",
                    role: "name",
                    width: 24,
                    align: "left"
                },
                {
                    title: "Version",
                    role: "version",
                    width: 10,
                    align: "left"
                },
                {
                    title: "Feature",
                    role: "featureLabel",
                    width: 14,
                    align: "left"
                },
                {
                    title: "Download",
                    role: "downloadText",
                    width: 13,
                    align: "right"
                },
                {
                    title: "Install",
                    role: "installText",
                    width: 13,
                    align: "right"
                },
                {
                    title: "  Source",
                    role: "sourceLabel",
                    width: 16,
                    align: "left"
                }
            ]

            cellFormat: (row, col, value) => {
                const str = String(value);
                if (col === 0) {
                    return {
                        text: " " + str,
                        color: str.startsWith("qmltui") ? Theme.cyan : Theme.ink,
                        bold: str.startsWith("qmltui")
                    };
                }
                if (col === 1) {
                    return {
                        text: str,
                        color: Theme.secondary
                    };
                }
                if (col === 2) {
                    const isOpt = str.startsWith("+");
                    return {
                        text: str,
                        color: str === "+wasm-sdk" ? Theme.purple : (isOpt ? Theme.amber : Theme.muted),
                        bold: isOpt
                    };
                }
                if (col === 3) {
                    return {
                        text: str,
                        color: str === "0 KiB" ? Theme.muted : Theme.ink
                    };
                }
                if (col === 4) {
                    return {
                        text: str,
                        color: Theme.secondary
                    };
                }
                if (col === 5) {
                    const isCached = str === "cache hit";
                    return {
                        text: isCached ? "  ✓ cached" : "  ↓ registry",
                        color: isCached ? Theme.green : Theme.cyan,
                        bold: isCached
                    };
                }
                return {
                    text: str
                };
            }
        }

        TreeView {
            visible: root.engine.showTreePreview
            width: parent.width
            height: root.bodyRows
            currentIndex: -1
            expandedIcon: "▾ "
            collapsedIcon: "▸ "
            textColor: Theme.ink
            branchColor: Theme.cyan
            selectedTextColor: Theme.ink
            selectionColor: "#1e293b"
            model: root.engine.dependencyTree

            nodeFormat: (row, value) => {
                const text = String(value);
                if (row === 0) {
                    return {
                        text: text,
                        color: Theme.green,
                        bold: true
                    };
                }
                if (text.indexOf("[+wasm-sdk]") !== -1) {
                    return {
                        text: text,
                        color: Theme.purple,
                        bold: true
                    };
                }
                if (text.indexOf("[+") !== -1) {
                    return {
                        text: text,
                        color: Theme.amber,
                        bold: true
                    };
                }
                if (text.indexOf("(cached)") !== -1) {
                    return {
                        text: text,
                        color: Theme.muted
                    };
                }
                return {
                    text: text,
                    color: Theme.ink
                };
            }
        }
    }
}
