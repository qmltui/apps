# Vector Plotting Dashboard (`plotting_dashboard`)

Real-time vector plotting and oscilloscope dashboard built on `Canvas.Braille` (2×4 subpixels per cell), featuring selectable waveforms (sine, square, triangle, sawtooth), frequency/amplitude/noise controls, and XY Lissajous phase orbits.

```sh
./build/apps/plotting_dashboard/qmltui_plotting_dashboard
```

## Controls

| Key | Action |
| :--- | :--- |
| `Tab` / `Shift+Tab` | Move focus across waveform selector, sliders, and toggles |
| `Left` / `Right` / `Enter` / `Space` | Adjust focused `ComboBox`, `Slider`, `CheckBox`, or `Button` |
| `Space` | Toggle waveform pause/resume (outside focused buttons) |
| `Q` / `Esc` | Quit |
