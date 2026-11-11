qt_add_executable(qmltui_snake_game src/main.cpp)

qt_add_qml_module(qmltui_snake_game
    URI SnakeGame
    VERSION 1.0
    QML_FILES
        qml/Main.qml

    DEPENDENCIES
        TARGET qmltui::core
)

target_link_libraries(qmltui_snake_game PRIVATE Qt6::Core Qt6::Qml qmltui::core)

if(COMMAND qmltui_wasm_app)   # browser build, see examples/wasm and docs/guide/webassembly.md
    qmltui_wasm_app(qmltui_snake_game)
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_snake_game PRIVATE qmltui::runner)
    # The game ticks on QML Timers, so the whole run is virtual time.
    qmltui_add_scenario(
        NAME test_runner_snake TARGET qmltui_snake_game
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/snake.js"
        CLOCK VIRTUAL
        ARTIFACT "snake/paused.txt"
        ARTIFACT_CONTENT "PAUSED"
    )
    qmltui_add_scenario(
        NAME test_runner_snake_ai TARGET qmltui_snake_game
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/snake_ai.js"
        LABELS AgentFull
        CLOCK VIRTUAL
        SCENARIO_TIMEOUT_MS 15000
        ARTIFACT "snake/ai_gameplay.txt"
        ARTIFACT_CONTENT "SCORE"
    )
    qmltui_add_scenario(
        NAME snake_game_demo TARGET qmltui_snake_game
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 80 ROWS 24
        CLOCK REAL
        SCENARIO_TIMEOUT_MS 30000
        RECORD snake_game.cast
        LABELS Demo
    )
endif()
