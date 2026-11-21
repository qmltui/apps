qt_add_executable(qmltui_twozero48 src/main.cpp)

qt_add_qml_module(qmltui_twozero48
    URI TwoZero48
    QML_FILES
        qml/Main.qml
    DEPENDENCIES
        TARGET QmlTui::Core
)

target_link_libraries(qmltui_twozero48 PRIVATE Qt6::Core Qt6::Qml QmlTui::Core)

if(COMMAND qmltui_wasm_app)   # browser build, see docs/guides/webassembly.md
    qmltui_wasm_app(qmltui_twozero48)
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_twozero48 PRIVATE QmlTui::Runner)
    qmltui_add_scenario(
        NAME test_runner_twozero48 TARGET qmltui_twozero48
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/twozero48.js"
        ARTIFACT "twozero48/initial.txt"
        ARTIFACT_CONTENT "2048 TUI"
    )
    qmltui_add_scenario(
        NAME twozero48_demo TARGET qmltui_twozero48
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 80 ROWS 26
        CLOCK REAL
        RECORD twozero48.cast
        LABELS Demo
    )
endif()
