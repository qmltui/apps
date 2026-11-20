/** @param {Runner} terminal */
function main(terminal) {
    terminal.expectText("Ask anything")

    // First /tree submission into scrollback must not highlight row 0.
    const firstMark = terminal.scrollbackRowCount()
    terminal.typeText("/tree")
    terminal.press("Enter")
    terminal.expectScrollbackText("QmlTui (Repository Root)")
    terminal.captureScrollbackStyled("tree_first.txt", firstMark)

    // Second /tree reuses the cached Scrollback delegate and must remain unselected.
    const secondMark = terminal.scrollbackRowCount()
    terminal.typeText("/tree")
    terminal.press("Enter")
    terminal.expectScrollbackText("PinnedPromptBar.qml [Clean]")
    terminal.captureScrollbackStyled("tree_styled.txt", secondMark)
}
