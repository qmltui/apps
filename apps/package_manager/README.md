# Qt Package Manager (`qpm`)

Minimalistic `cargo install`-style CLI package installer simulation using `Terminal.Inline` and `Scrollback`.

Build with `cmake --build build --target qmltui_package_manager`.
Run `./build/apps/package_manager/qmltui_package_manager`.
See [main.cpp](main.cpp) for startup and [Main.qml](Main.qml) for the inline progress bar and scrollback output.

Run its scenario with `ctest --test-dir build -R package_manager --output-on-failure`.
