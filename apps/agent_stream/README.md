# Agent Stream

Interactive coding-agent session combining `Terminal.Inline`, `Scrollback`, prompt input, and modal dialogs.

Build with `cmake --build build --target qmltui_agent_stream`.
Run `./build/apps/agent_stream/qmltui_agent_stream`.
See [main.cpp](main.cpp) for startup and local QML files for the interface.

Run its scenarios with `ctest --test-dir build -R agent_stream --output-on-failure`.

The banner defaults to a five-row half-block Canvas logo. Set `logoRenderMode`
on `BannerCard` to `Canvas.Braille` for the alternative braille logo. Both use
`Circle` and `Polygon` shapes and the same white-and-green colors.
