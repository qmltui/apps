# Industrial PLC & SCADA HMI (`industrial_plc_hmi`)

ISA-101-inspired SCADA operator console and programmable logic controller (PLC) simulator. Demonstrates rounded `Panel` chrome, thresholded `ProgressBar` gauges with rolling `Sparkline` histories, multi-trace Braille `Canvas` trend plotting, an animated P&ID reactor mimic, modal `Popup` safety interlocks, a `TableView` audit log, and modular I/O rack diagnostics.

```sh
./build/apps/industrial_plc_hmi/qmltui_industrial_plc_hmi
```

## Controls

| Key | Action |
| :--- | :--- |
| `F1`–`F4` or `1`–`4` | Switch between **Dashboard**, **Alarms**, **Diagnostics**, and **Settings** (or click tabs) |
| `S` / `X` | Start (`PROCESSING`) or Stop (`IDLE`) station when not locked out by a fault |
| `M` | Open safety confirmation modal to enter or exit `150 RPM` **Maintenance** jog |
| `R` | Open safety confirmation modal to acknowledge and clear an **Over-Temperature Fault** |
| `T` | Cycle Braille trend scope trace mode (`TEMP + RPM`, `TEMP ONLY`, `RPM ONLY`) |
| `K` | Cycle SCADA color skin (`ISA-SLATE`, `AMBER CRT`, `PHOSPHOR`) |
| `D` | Restore factory calibration defaults (`1500 RPM`, `85.0 °C` trip limit) |
| `Tab` / `Shift+Tab` | Move focus across setpoint inputs and command pushbuttons |
| `↑` / `↓` (in input) | Step focused RPM (`±100 RPM`) or temperature limit (`±2.5 °C`) setpoint |
| `Enter` (in input) | Validate and apply typed setpoint |
| `Y` / `N` / `Esc` | Confirm or cancel an open safety interlock modal |
| `Q` / `Esc` | Clear input focus, close modal, or quit application |
