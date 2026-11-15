# `qtop`: `btop++`-Inspired System Monitor UI Study

`qtop` is a declarative UI study inspired by [`btop++`](https://github.com/aristocratos/btop). Rather than replicating `btop++`'s full multi-OS monitoring backend, it expresses Braille CPU history graphs, per-core meters, memory/disk bars, network sparklines, a sortable `QAbstractTableModel` process table, and `Behavior`-animated panel collapsing in ~770 lines of QML (`qml/Main.qml` and `qml/QtopPanel.qml`).

```sh
./build/apps/qtop/qmltui_qtop
```

## Controls

| Key | Action |
| :--- | :--- |
| `1` / `2` / `3` / `4` | Toggle CPU, Memory, Network, and Processes panels (`Behavior` animated) |
| `c` / `m` / `p` | Sort process `TableView` by CPU %, Memory, or PID |
| `r` | Cycle refresh interval (`500 ms`, `1000 ms`, `2000 ms`, `5000 ms`) |
| `Space` | Pause or resume live telemetry sampling |
| `q` / `Esc` | Quit |
