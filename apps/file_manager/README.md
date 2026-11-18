# Terminal File Manager (`file_manager`)

Three-column Miller file manager (Parent, Current, Preview) built with C++17 and QmlTui:

- **Streaming filesystem backend**: Enumerates entries via `QDirListing` and reports mount usage via `QStorageInfo`.
- **Multi-format live preview**: Renders syntax-highlighted text/Markdown, directory subtrees, binary metadata, and asynchronous off-thread image, SVG, and multi-page PDF previews via `qmltui::AsyncImageProvider` (`Qt6::Gui`, `Qt6::Svg`, `Qt6::Pdf`).
- **Integrated shell**: Hosts an interactive Unix shell drawer (`QmlTui.PtyView`) synchronized with the active directory.

```sh
./build/apps/file_manager/qmltui_file_manager
```

## Controls

| Key | Action |
| :--- | :--- |
| `h` / `Left` | Navigate to parent directory (`cd ..`) |
| `l` / `Right` / `Enter` | Enter directory or open file |
| `j` / `Down`, `k` / `Up` | Move cursor down or up |
| `g` / `Home`, `G` / `End` | Jump to first or last item |
| `Space` / `v` | Toggle selection on current item |
| `V` / `Ctrl+A` | Select all or invert selection |
| `y` / `x` / `p` | Copy (yank), Cut, or Paste |
| `a` / `n` | Create file or directory |
| `r` / `F2` | Rename item |
| `d` / `Delete` | Delete selected or current item |
| `/` / `f` | Filter current directory |
| `.` | Toggle hidden dotfiles |
| `s` | Cycle sort order (Name, Size, Modified Time, Type) |
| `m` | Cycle image preview mode (`Auto`, `HalfBlock`, `Braille`, `ASCII`) |
| `[` / `]` | Previous or next PDF page in preview pane |
| `!` / `!!` | Focus split shell drawer (`!`) or toggle fullscreen shell (`!!`) |
| `Esc` / `Ctrl+\` | Unfocus shell (`Ctrl+\` always unfocuses; `Esc` passes through to foreground TUIs), then hide drawer |
| `?` / `F1` | Open keybindings help dialog |
| `q` | Quit |
