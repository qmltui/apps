pragma ComponentBehavior: Bound

import QtQml
import QmlTui

Rectangle {
    id: searchBar

    required property var theme
    property string queryText: ""
    property int currentMatch: 0
    property int totalMatches: 0
    property bool isSearching: false

    signal queryChanged(string query)
    signal searchAccepted
    signal nextMatch
    signal previousMatch
    signal closeSearch

    height: 3
    color: searchBar.theme.surface
    border {
        style: Border.Single
        color: searchBar.theme.warning
    }

    Row {
        anchors {
            fill: parent
            leftMargin: 1
            rightMargin: 1
        }
        spacing: 2
        clip: true

        Text {
            text: "🔍 Search:"
            color: searchBar.theme.warning
            bold: true
        }

        TextInput {
            id: input

            width: 30
            height: 1
            focus: searchBar.isSearching
            color: searchBar.theme.textPrimary
            cursorColor: searchBar.theme.warning
            placeholderText: "Type search phrase..."
            placeholderColor: searchBar.theme.textMuted
            text: searchBar.queryText

            onTextChanged: {
                searchBar.queryText = input.text;
                searchBar.queryChanged(input.text);
            }

            onAccepted: {
                searchBar.searchAccepted();
            }
        }

        Text {
            text: "│"
            color: searchBar.theme.border
        }

        Text {
            text: (searchBar.totalMatches > 0) ? ("Match " + (searchBar.currentMatch + 1) + " of " + searchBar.totalMatches) : (searchBar.queryText.length > 0 ? "No matches" : "")
            color: (searchBar.totalMatches > 0) ? searchBar.theme.accent : searchBar.theme.textMuted
            bold: true
        }

        Text {
            text: "│"
            color: searchBar.theme.border
        }

        Text {
            text: "[Enter] Done  [n/N] Next/Prev  [Esc] Close"
            color: searchBar.theme.textMuted
        }
    }
}
