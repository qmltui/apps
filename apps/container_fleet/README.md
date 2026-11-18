# Container Fleet Dashboard (`container_fleet`)

Container fleet monitor built with `TableView`, `QSortFilterProxyModel` column sorting and status filtering, `TabBar` filter tabs, per-node CPU `Sparkline` charts, and an expandable streaming container log drawer.

```sh
./build/apps/container_fleet/qmltui_container_fleet
```

## Controls

| Key | Action |
| :--- | :--- |
| `Up` / `Down`, `PageUp` / `PageDown` | Navigate container rows |
| `1` / `2` / `3` | Filter by status (`All`, `Running`, `Degraded/Stopped`) |
| `/` | Focus live search filter (`Esc` clears search) |
| `S` / Click column header | Cycle sort column and order |
| `R` | Restart selected container |
| `A` | Restart and heal all degraded containers |
| `L` | Toggle streaming container log drawer |
| `Q` / `Ctrl+Q` / `Esc` | Quit (when search is empty) |
