# Vector Plotting Dashboard (`plotting_dashboard`)

Real-time vector plotting and oscilloscope dashboard built on [`QmlTui.Charts`](https://github.com/qmltui/charts) (`ChartView`, `ValueAxis`, `ReferenceLine`, and Braille `LineSeries` with interactive crosshair inspection), featuring selectable waveforms (sine, square, triangle, sawtooth), frequency/noise controls, and closed XY Lissajous phase orbits.

```sh
./build/apps/plotting_dashboard/qmltui_plotting_dashboard
```

## Controls

| Key | Action |
| :--- | :--- |
| `Tab` / `Shift+Tab` | Move focus across waveform selector, sliders, and toggles |
| `Left` / `Right` / `Enter` / `Space` | Adjust focused `ComboBox`, `Slider`, `CheckBox`, or `Button` |
| `Space` | Toggle waveform pause/resume (outside focused buttons) |
| `I` | Toggle opt-in interactive crosshair inspection (`ON`/`OFF`) |
| `Q` / `Esc` | Quit |
