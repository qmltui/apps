qt_add_executable(qmltui_screensaver
    src/main.cpp
)

qt_add_qml_module(qmltui_screensaver
    URI Screensaver
    SOURCES
        src/DoomFireItem.h
        src/DoomFireItem.cpp
        src/PipesItem.h
        src/PipesItem.cpp
        src/DonutItem.h
        src/DonutItem.cpp
        src/LifeItem.h
        src/LifeItem.cpp
    QML_FILES
        qml/Main.qml
        qml/StarfieldSaver.qml
        qml/MystifySaver.qml
        qml/DoomFireSaver.qml
        qml/Wireframe3DSaver.qml
        qml/PlasmaSaver.qml
        qml/MatrixRainSaver.qml
        qml/PipesSaver.qml
        qml/DonutSaver.qml
        qml/LifeSaver.qml
        qml/DvdSaver.qml
        qml/LorenzSaver.qml

    DEPENDENCIES
        TARGET qmltui::core
)

target_include_directories(qmltui_screensaver PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(qmltui_screensaver PRIVATE Qt6::Core Qt6::Qml qmltui::core)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_screensaver)
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_screensaver PRIVATE qmltui::runner)
    qmltui_add_scenario(
        NAME test_runner_screensaver TARGET qmltui_screensaver
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/screensaver.js"
        CLOCK VIRTUAL
        ARTIFACT "screensaver/active.txt"
        ARTIFACT_CONTENT "SCREENSAVER"
    )
    qmltui_add_scenario(
        NAME screensaver_demo TARGET qmltui_screensaver
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 100 ROWS 30
        CLOCK REAL
        RECORD screensaver.cast
        LABELS Demo
    )
endif()
