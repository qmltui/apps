pragma ComponentBehavior: Bound
import QtQml
import QmlTui

// Dedicated 24-band FFT spectrum analyzer and stereo VU meter panel.
Item {
    id: specDeck

    required property AudioEngine engine
    property bool showVu: true

    readonly property int bandCount: engine.bandCount

    function spectrumRow(row: int, rows: int, barWidth: int, gap: int, pad: int): string {
        const levels = specDeck.engine.bandLevels;
        const peaks = specDeck.engine.peakLevels;
        let line = " ".repeat(pad);
        const sep = " ".repeat(gap);
        for (let i = 0; i < specDeck.bandCount; ++i) {
            const lvl = (levels && i < levels.length) ? levels[i] : 0.0;
            const pk = (peaks && i < peaks.length) ? peaks[i] : 0.0;
            const fill = lvl * rows - row;
            let glyph;
            if (fill >= 1.0) {
                glyph = "█";
            } else if (fill > 0.0) {
                glyph = Theme.blockGlyphs[Math.round(fill * 8)];
            } else {
                const peakRow = Math.min(rows - 1, Math.floor(pk * rows));
                glyph = (peakRow === row && pk > 0) ? "▔" : " ";
            }
            line += glyph.repeat(barWidth);
            if (i < specDeck.bandCount - 1)
                line += sep;
        }
        return line;
    }

    function labelRow(barWidth: int, gap: int, pad: int, maxWidth: int): string {
        const labels = specDeck.engine.bandLabels;
        const pitch = barWidth + gap;
        const step = pitch >= 4 ? 1 : (pitch >= 2 ? 2 : 4);
        const slotCols = pitch * step;
        let line = " ".repeat(pad);
        for (let i = 0; i < specDeck.bandCount; i += step)
            line += String(labels[i] || "").padEnd(slotCols, " ");
        return line.substring(0, maxWidth);
    }

    Column {
        id: specInner
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        spacing: 1

        readonly property bool vuVisible: specDeck.showVu && height >= 6 && width >= 32
        readonly property int specHeight: Math.max(2, height - 2 - (vuVisible ? 2 : 0))
        readonly property int halfVuWidth: Math.max(14, Math.floor((width - 3) / 2))

        // Header Row
        Text {
            width: parent.width
            height: 1
            text: Theme.fitText("SPECTRUM // 24-BAND FFT", width)
            color: Theme.header
            bold: true
        }

        // 24-Band FFT Bars + Frequency Scale
        Item {
            id: specBox
            width: parent.width
            height: specInner.specHeight

            readonly property bool showLabels: height >= 4
            readonly property int barRows: Math.max(1, height - (showLabels ? 1 : 0))
            readonly property int gap: width >= (specDeck.bandCount * 2 - 1) ? 1 : 0
            readonly property int barWidth: Math.max(1,
                Math.floor((width - (specDeck.bandCount - 1) * gap) / specDeck.bandCount))
            readonly property int usedWidth: specDeck.bandCount * barWidth + (specDeck.bandCount - 1) * gap
            readonly property int leftPad: Math.max(0, Math.floor((width - usedWidth) / 2))

            Column {
                anchors.top: parent.top
                anchors.left: parent.left
                width: parent.width
                height: specBox.barRows
                spacing: 0

                Repeater {
                    model: specBox.barRows

                    delegate: Text {
                        required property int index
                        readonly property int rowFromBottom: specBox.barRows - 1 - index
                        readonly property real heightRatio: (rowFromBottom + 1) / specBox.barRows

                        height: 1
                        bold: true
                        text: specDeck.spectrumRow(rowFromBottom, specBox.barRows,
                                                   specBox.barWidth, specBox.gap, specBox.leftPad)
                        color: heightRatio > 0.8 ? Theme.secondary
                             : heightRatio > 0.5 ? Theme.mid : Theme.accent
                    }
                }
            }

            Text {
                visible: specBox.showLabels
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                height: 1
                text: specDeck.labelRow(specBox.barWidth, specBox.gap, specBox.leftPad, specBox.width)
                color: Theme.text
                dim: true
            }
        }

        // 1-Row Stereo VU Meters
        Row {
            visible: specInner.vuVisible
            width: parent.width
            height: 1
            spacing: 3

            VuMeter {
                label: "L"
                level: specDeck.engine.vuLeft
                accentColor: Theme.vuLow
                labelColor: Theme.scopeAccent
                meterWidth: specInner.halfVuWidth
            }
            VuMeter {
                label: "R"
                level: specDeck.engine.vuRight
                accentColor: Theme.vuLow
                labelColor: Theme.scopeRight
                meterWidth: specInner.halfVuWidth
            }
        }
    }
}
