# QmlTui Framework Technical Guide

QmlTui is a retained-mode terminal UI framework for Qt 6.8+ and C++17.

## Retained-Mode Terminal Architecture

Unlike immediate-mode TUI libraries, QmlTui evaluates a declarative QML item tree into a double-buffered character-cell grid:

1. **Items & Hierarchies**: Subclass `qmltui::Item` to render directly into `TextGridBuffer`.
2. **Layout Engine**: Declarative positioning with `Row`, `Column`, `RowLayout`, `ColumnLayout`, `GridLayout`, `StackLayout`, and `Anchors`.
3. **QML Declarative Bindings**: Reactive properties, `State` / `Transition`, and C++ `QAbstractItemModel` views.
4. **Behavior on Properties**: Smooth animated value and color transitions in terminals.
5. **Automation & Testing**: Headless `qmltui::Runner` closed-loop automation engine.

## QML Modules

- `QmlTui`: Core primitives (`Terminal`, `Scrollback`, `Item`, `FocusScope`, `Popup`, `Rectangle`, `Text`, `TextInput`, `TextArea`, `ListView`, `TableView`, `TreeView`, `GridView`, `ScrollView`, `Canvas`).
- `QmlTui.Controls`: Interactive controls (`Button`, `CheckBox`, `RadioButton`, `Slider`, `ComboBox`, `SpinBox`, `ProgressBar`, `BusyIndicator`, `Sparkline`, `TabBar`, `StackView`, `Panel`, `Dialog`, `ToolTip`).
- `QmlTui.Image`: Terminal graphics (`Image`, `AnimatedSprite`).
- `QmlTui.PtyView`: Embedded Unix PTY subshell (`PtyView`).

## Closed-Loop Agent Workflow

Capture rendered frames or run scripted scenarios headlessly without an interactive TTY:

```bash
qmltui-runner --clock virtual --print -- ./build/apps/markdown_viewer/qmltui_markdown_viewer
```
