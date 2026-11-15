# Qt Package Manager (`package_manager`)

Multi-stage CLI package installer simulation (`qpm`) built on `Terminal.Inline` and `Scrollback`. It walks through an interactive transaction plan (`TableView` + dependency `TreeView`), a 4-worker parallel download/build pool with throughput `Sparkline`, an expandable compiler log tail, and `DelegateChooser`-driven `Scrollback` receipt cards.

```sh
./build/apps/package_manager/qmltui_package_manager [--yes | -y] [--auto-exit]
```

- `-y`, `--yes`: skip the interactive confirmation prompt and start building immediately.
- `--auto-exit`: exit automatically when installation completes.

## Controls

| Key | Action |
| :--- | :--- |
| `d` | Switch transaction plan between package `TableView` and dependency `TreeView` |
| `Space` | Toggle focused optional feature `CheckBox` (`ptyview`, `sixel`, `wasm-sdk`) |
| `y` / `Enter` | Lock transaction plan into scrollback and start execution |
| `v` | Expand or collapse the live 4-row network/compiler log stream during execution |
| `n` / `q` / `Esc` | Cancel transaction or quit |
