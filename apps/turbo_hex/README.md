# Turbo Hex (`turbo_hex`)

Multi-window **Hex & Binary Inspector** built on [`QmlTui.TVision`](https://github.com/qmltui/tvision) (inspired by IDA `idat` and Borland Turbo Debugger). Pairs up to 3 tileable **Hex + ASCII** MDI windows with a live **Data Inspector** (`int8..64`, `float32`, bits, LE/BE endianness) and a **Headers & Strings** window that parses **ELF**, **PNG**, **ZIP**, **PE/MZ**, and **WASM** structures plus printable strings—clicking any field highlights its exact byte range.

```sh
# Inspect one or more real binary files (or browse from a directory)
./build/apps/turbo_hex/qmltui_turbo_hex /bin/ls

# Or launch with built-in ELF64 and PNG sample binaries (used automatically on WebAssembly)
./build/apps/turbo_hex/qmltui_turbo_hex --demo
```

## Controls

| Key / Mouse | Action |
| :--- | :--- |
| `F3` / `F2` / `Shift+F2` | **Open Binary** dialog / **Save** patched buffer / **Save As** |
| `F4` | Toggle **Overwrite Edit Mode** (type `0-9A-F` in Hex pane or printable chars in ASCII pane) |
| `Tab` | Switch cursor between **Hex** nibble column and **ASCII** character column |
| `Ctrl+F` / `Ctrl+G` | **Find Hex Bytes / ASCII Text** / **Go to Offset** (`0x78` or decimal `120`) |
| `Ctrl+Z` / `Ctrl+Y` | **Undo** / **Redo** byte patches |
| `F7` / `F8` | Toggle **Data Inspector** / **Headers & Strings** window |
| `F5` / `F6` / `Ctrl+F5` | **Zoom** active window / **Next** MDI window / Keyboard **Move & Resize** |
| `F9` | Cycle color palette (`Borland Blue`, `Turbo Cyan`, `Dark DOS`, `Monochrome`) |
| `F`/`E`/`S`/`V`/`W`/`O`/`H` (or `F10` / `Alt+Key`) | Open top pull-down menus directly in default Read-Only mode |
| `Alt+X` / `Ctrl+Q` / `Esc` | Close active menu/modal, exit `F4` edit mode, or quit (with unsaved-patch check) |
