# Terminal Markdown Viewer (`markdown_viewer`)

Terminal Markdown document reader built on [`QmlTui.Markdown`](https://github.com/qmltui/markdown) (`MdView`, `MdDoc`, `MdDocStyle`, `MdViewStyle`, powered by KDE `md4qt`) with syntax-highlighted code blocks, Unicode box-drawing tables, heading navigation, OSC 8 hyperlinks, live search highlighting, and interactive typography/style presets.

```sh
./build/apps/markdown_viewer/qmltui_markdown_viewer [path/to/file.md]
```

## Controls

| Key | Action |
| :--- | :--- |
| `J` / `Down`, `K` / `Up` | Scroll document line by line |
| `Space` / `PageDown`, `PageUp` | Scroll page down or up |
| `G` / `Home`, `Shift+G` / `End` | Jump to top or bottom of document |
| `Tab` / `Shift+Tab` | Jump to next or previous Markdown heading |
| `/` | Search in document (`n` / `N` next or previous match) |
| `S` | Cycle document style (`Modern`, `Classic`, `Minimal`) |
| `T` | Cycle color theme |
| `E` | Toggle live style editor drawer (`Up`/`Down` select, `Left`/`Right`/`Enter` tweak) |
| `L` | Toggle code-block line numbers |
| `O` | Cycle built-in sample Markdown documents |
| `Q` / `Esc` | Close search or style editor first, otherwise quit |
