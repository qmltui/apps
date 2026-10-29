qt_add_executable(qmltui_screensaver
    main.cpp
    DoomFireItem.h
    DoomFireItem.cpp
    PipesItem.h
    PipesItem.cpp
    DonutItem.h
    DonutItem.cpp
    LifeItem.h
    LifeItem.cpp
)

qt_add_qml_module(qmltui_screensaver
    URI Screensaver
    VERSION 1.0
    SOURCES
        DoomFireItem.h
        DoomFireItem.cpp
        PipesItem.h
        PipesItem.cpp
        DonutItem.h
        DonutItem.cpp
        LifeItem.h
        LifeItem.cpp
    QML_FILES
        Main.qml
        StarfieldSaver.qml
        MystifySaver.qml
        DoomFireSaver.qml
        Wireframe3DSaver.qml
        PlasmaSaver.qml
        MatrixRainSaver.qml
        PipesSaver.qml
        DonutSaver.qml
        LifeSaver.qml
        DvdSaver.qml
        LorenzSaver.qml

    DEPENDENCIES
        TARGET qmltui::core
)

target_link_libraries(qmltui_screensaver PRIVATE Qt6::Core Qt6::Qml qmltui::core)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_screensaver)
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_screensaver PRIVATE qmltui::runner)
    qmltui_add_scenario(
        NAME test_runner_screensaver TARGET qmltui_screensaver
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_screensaver.js"
        CLOCK VIRTUAL
        ARTIFACT "screensaver/active.txt"
        ARTIFACT_CONTENT "SCREENSAVER"
    )
    qmltui_add_scenario(
        NAME screensaver_demo TARGET qmltui_screensaver
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 100 ROWS 30
        CLOCK VIRTUAL
        RECORD screensaver.cast
        LABELS Demo
    )
endif()
