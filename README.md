# QmlTui applications

The larger QmlTui example applications live here. The framework repository keeps
component demos, topic examples, tutorials, the gallery, and the WebAssembly site.

Requires C++20, Qt 6.8+ with QmlPrivate, and CMake 3.21+. Private Qt features
(Behavior and StateGroup) are enabled; PtyView is enabled on Unix. Optional
applications also use Qt Multimedia or Network. The Markdown viewer downloads
md4qt during configuration.

```sh
cmake -S . -B build -G Ninja -DFETCHCONTENT_SOURCE_DIR_QMLTUI=/home/yosri/qml-tui
cmake --build build -j 6
ctest --test-dir build --output-on-failure
```

Without the source override, FetchContent uses `yosriayed/qml-tui` tag `v0.5.0`.
The release CI job is gated by `QMLTUI_RELEASE_AVAILABLE` until that tag is public.
Use `-DQMLTUI_BUILD_MARKDOWN_VIEWER=OFF` to configure without downloading md4qt.

## Applications

- [agent_stream](apps/agent_stream/)
- [audio_player](apps/audio_player/)
- [btop](apps/btop/)
- [container_fleet](apps/container_fleet/)
- [file_manager](apps/file_manager/)
- [industrial_plc_hmi](apps/industrial_plc_hmi/)
- [markdown_viewer](apps/markdown_viewer/)
- [package_manager](apps/package_manager/)
- [plotting_dashboard](apps/plotting_dashboard/)
- [screensaver](apps/screensaver/)
- [snake_game](apps/snake_game/)
- [starter_template](apps/starter_template/)
- [twozero48](apps/twozero48/)
