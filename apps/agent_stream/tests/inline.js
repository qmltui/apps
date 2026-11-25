
/** @param {Runner} terminal */
function main(terminal) {
    terminal.expectText("Ask anything")
    terminal.expectNoText("shell")
    terminal.captureText("inline.txt")

    // The session banner lives in the scrollback, never in the live frame.
    terminal.expectScrollbackText("QmlTui")
    terminal.expectScrollbackText("agent stream demo")
    terminal.expectNoScrollbackText("Ask anything")
    terminal.captureScrollback("banner.txt")

    terminal.typeText("persistent prompt")
    for (const key of ["F1", "T", "B"]) {
        terminal.press(key, key === "F1" ? [] : ["Ctrl"])
        terminal.expectText("persistent prompt")
        terminal.expectText("[F2] model")
        terminal.expectNoText("[Shift+PgUp/PgDn]")
    }
    terminal.captureText("prompt.txt")

    terminal.press("C", ["Ctrl"])
    terminal.press("F2")
    terminal.expectText("Neural model selector")
    terminal.captureText("model.txt")
    terminal.press("Escape")
    terminal.expectText("Ask anything")

    console.log("Waiting for the agent to be ready...")

    terminal.typeText("/demo 1")
    terminal.press("Enter")
    terminal.advanceTime(4000)
    terminal.waitForText("Allow once", 7000)
    terminal.captureText("permission.txt")
    terminal.press("3")
    terminal.expectText("Ask anything")
    terminal.captureText("ready.txt")

    // Scenario 3 (/demo 3) streams live thought deltas in the inline viewport,
    // commits them to Scrollback, updates DiffCard & DiffInspectorModal to
    // TextGridBuffer.cpp, and runs step 4 stress tests after permission approval.
    terminal.typeText("/demo 3")
    terminal.press("Enter")
    terminal.advanceTime(200)
    terminal.expectText("streaming...")
    terminal.expectText("Scanning text buffer bounds ▍")
    terminal.expectNoScrollbackText("against UTF-8 overflow vectors.")
    terminal.advanceTime(2800)
    terminal.waitForText("Allow once", 7000)
    terminal.expectNoText("streaming...")
    terminal.expectScrollbackText("Scanning text buffer bounds against UTF-8 overflow vectors.")
    terminal.expectScrollbackText("TextGridBuffer::putCluster")
    terminal.press("F3")
    terminal.expectText("TextGridBuffer.cpp")
    terminal.expectText("+4 additions")
    terminal.press("Escape")
    terminal.press("1")
    terminal.advanceTime(1500)
    terminal.waitForText("Ask anything", 5000)
    terminal.expectScrollbackText("Passed 24/24 buffer stress tests")
    terminal.expectScrollbackText("Security patch applied to TextGridBuffer.cpp")
    terminal.expectScrollbackText("overflow risk resolved")
    terminal.expectNoScrollbackText("~~overflow risk~~")
    terminal.expectNoScrollbackText("**24/24**")
    terminal.captureScrollbackStyled("response_markdown_styled.txt")
}
