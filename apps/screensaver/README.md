# Retro Screensaver Showcase

A collection of classic demoscene and retro OS screensavers built with [QmlTui](https://github.com/qmltui-dev/qmltui), showcasing multiple diverse terminal rendering paradigms: solid 1:1 half-block subpixels (**`Canvas.HalfBlock`**), ultra-high-resolution 2x4 dot subpixels (**`Canvas.Braille`**), authentic Unicode box-drawing characters, mathematical ASCII luminance raymarching, and cellular automata.

## Screensaver Modes

### Half-Block Subpixel Graphics (`Canvas.HalfBlock`)
1. **Starfield Warp** (`[1]`):
   Classic 3D depth-projected starfield with speed streaks and depth-based brightness grading.
2. **Mystify** (`[2]`):
   Windows 3.1 style bouncing vector polygon loops with fading trail ribbons.
3. **DOOM Fire** (`[3]`):
   The legendary 1990s PSX/PC DOOM cellular convection fire algorithm mapped to a 36-color furnace palette.
4. **3D Tesseract** (`[4]`):
   A 3D perspective wireframe hypercube with nested counter-rotating frames and glowing vertices.
5. **Plasma Waves** (`[5]`):
   Algorithmic trigonometric wave interference plasma mapped across a synthwave gradient.
6. **Matrix Digital Rain** (`[6]`):
   Cascading neon phosphor streams with bright leading sparks and fading trails.

### Character & Box-Drawing Graphics
7. **3D Pipes** (`[7]`):
   The iconic Windows 95/98 3D Pipes screensaver rendered using authentic Unicode box-drawing characters (`│`, `─`, `┌`, `┐`, `└`, `┘`, `┼`, `●`, `■`), featuring multi-pipe concurrent crawling, 3D depth layering, elbow joints, cross-intersections, and auto-clearing cycles.
8. **ASCII Donut** (`[8]`):
   Andy Sloane's legendary 3D rotating torus raymarching engine rendered with real-time ASCII luminance ramp (`.,-~:;=!*#$@`), z-buffering, and theme-shaded illumination.
9. **Game of Life** (`[9]`):
   Conway's Cellular Automata with toroidal wrapping, generation-based age coloration (bright newborns transitioning to mature structures), CRT phosphor decay persistence (`·`), and intelligent stagnation detection that injects gliders, pulsars, and spaceships.
10. **Bouncing DVD Logo** (`[0]`):
    The beloved retro DVD video screensaver badge floating across the terminal with exact corner-hit anticipation, wall bounce ripples, color cycling, motion trail echoes, and a celebratory corner-hit banner.

### Braille Subpixel Graphics (`Canvas.Braille`)
11. **Lorenz Attractor** (`[L]`):
    Edward Lorenz's chaotic strange attractor simulated with differential equations in real-time 3D, rendered using **`Canvas.Braille`** at 2x4 subpixel resolution per character cell for razor-sharp, flowing butterfly trajectories.

## Keyboard Controls

| Key | Action |
| :--- | :--- |
| `1` – `9` | Directly select modes 1 through 9 |
| `0` | Select mode 10 (DVD Logo) |
| `L` | Select mode 11 (Lorenz Attractor) |
| `Tab` / `Shift+Tab` | Cycle through all screensaver modes |
| `Space` | Pause / resume animation |
| `+` / `-` | Speed up / slow down animation |
| `C` | Cycle color theme (`Cyber`, `Emerald`, `Amber`, `CGA`) |
| `H` | Toggle HUD bezel for immersive borderless screensaver |
| `Q` / `Esc` | Quit screensaver |

## Running

```sh
# From the build directory
./apps/screensaver/qmltui_screensaver
```
