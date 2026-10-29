qt_add_executable(qmltui_twozero48 main.cpp)

qt_add_qml_module(qmltui_twozero48
    URI TwoZero48
    VERSION 1.0
    QML_FILES
        Main.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_link_libraries(qmltui_twozero48 PRIVATE Qt6::Core Qt6::Qml qmltui::core qmltui::controls)

if(COMMAND qmltui_wasm_app)   # browser build, see examples/wasm and docs/guide/webassembly.md
    qmltui_wasm_app(qmltui_twozero48)
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_twozero48 PRIVATE qmltui::runner)
    qmltui_add_scenario(
        NAME test_runner_twozero48 TARGET qmltui_twozero48
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_twozero48.js"
        ARTIFACT "twozero48/initial.txt"
        ARTIFACT_CONTENT "2048 TUI"
    )
    qmltui_add_scenario(
        NAME twozero48_demo TARGET qmltui_twozero48
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 80 ROWS 26
        CLOCK VIRTUAL
        RECORD twozero48.cast
        LABELS Demo
    )
endif()
