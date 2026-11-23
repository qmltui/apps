pragma ComponentBehavior: Bound

import QmlTui

Column {
    id: treeCard

    required property string title

    width: parent ? parent.width : 76
    spacing: 0

    Row {
        spacing: 1

        Text {
            text: "🌲 Workspace Hierarchy:"
            color: Theme.cyan
            bold: true
        }
        Text {
            text: " " + (treeCard.title || "~/QmlTui")
            color: "white"
            bold: true
        }
    }

    TreeView {
        id: repoTreeView
        width: parent.width > 0 ? Math.min(parent.width, 76) : 76
        height: 7
        focus: false
        focusPolicy: Qt.NoFocus
        textColor: "white"
        branchColor: Theme.cyan

        Component.onCompleted: {
            repoTreeView.model = {
                "text": "QmlTui (Repository Root)",
                "expanded": true,
                "children": [
                    {
                        "text": "src/QmlTui/Engine",
                        "expanded": true,
                        "children": [
                            {
                                "text": "InlineRenderer.cpp [Optimized]",
                                "expanded": false
                            },
                            {
                                "text": "TextGridBuffer.cpp [Verified]",
                                "expanded": false
                            }
                        ]
                    },
                    {
                        "text": "apps/agent_stream",
                        "expanded": true,
                        "children": [
                            {
                                "text": "Main.qml [Active]",
                                "expanded": false
                            },
                            {
                                "text": "PinnedPromptBar.qml [Clean]",
                                "expanded": false
                            }
                        ]
                    }
                ]
            };
            // Scrollback history is non-interactive; clear the auto-selected first row after loading the model.
            repoTreeView.currentIndex = -1;
        }
    }
}
