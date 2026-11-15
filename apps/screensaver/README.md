# Retro Screensaver (`screensaver`)

Collection of 11 classic terminal screensavers demonstrating `Canvas.HalfBlock`, `Canvas.Braille`, Unicode box-drawing geometry, ASCII luminance raymarching, and custom C++ `qmltui::Item` rendering.

```sh
./build/apps/screensaver/qmltui_screensaver
```

## Modes

| Key | Mode | Rendering Technique |
| :--- | :--- | :--- |
| `1` | Starfield Warp | `Canvas.HalfBlock` 3D depth-projected streaks |
| `2` | Mystify | `Canvas.HalfBlock` bouncing polygon trails |
| `3` | DOOM Fire | Custom C++ `Item` (`Canvas.HalfBlock` 36-color cellular fire) |
| `4` | 3D Tesseract | `Canvas.HalfBlock` perspective wireframe hypercube |
| `5` | Plasma Waves | `Canvas.HalfBlock` trigonometric wave interference |
| `6` | Matrix Digital Rain | Character-grid cascading phosphor streams |
| `7` | 3D Pipes | Custom C++ `Item` with Unicode box-drawing (`│ ─ ┌ ┐ └ ┘ ┼`) |
| `8` | ASCII Donut | Custom C++ `Item` 3D torus raymarching (`.,-~:;=!*#$@`) |
| `9` | Game of Life | Custom C++ `Item` toroidal cellular automata with age shading |
| `0` | Bouncing DVD Logo | Character-grid badge with bounce trails and corner-hit banner |
| `L` | Lorenz Attractor | `Canvas.Braille` (2×4 subpixel) 3D chaotic trajectory |

## Controls

| Key | Action |
| :--- | :--- |
| `Tab` / `Shift+Tab` | Cycle through screensaver modes |
| `Space` | Pause or resume animation |
| `+` / `-` | Increase or decrease animation speed |
| `C` | Cycle color theme (`Cyber`, `Emerald`, `Amber`, `CGA`) |
| `H` | Toggle HUD bezel |
| `Q` / `Esc` | Quit |
