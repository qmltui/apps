set(qtop_sources
    src/QtopModel.cpp
    src/QtopModel.h
)

qt_add_executable(qmltui_qtop
    src/main.cpp
    ${qtop_sources}
)

qt_add_qml_module(qmltui_qtop
    URI QtopDemo
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/QtopPanel.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_include_directories(qmltui_qtop PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(qmltui_qtop PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui::core
    qmltui::controls
)

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_qtop RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_qtop PRIVATE qmltui::runner)

    qmltui_add_scenario(
        NAME qtop_smoke
        TARGET qmltui_qtop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/qtop.js"
        COLUMNS 132
        ROWS 42
        LABELS AgentSmoke
    )

    # Only timer-driven refreshes may repaint while the user is idle.
    qmltui_add_scenario(
        NAME qtop_idle_budget
        TARGET qmltui_qtop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/idle_budget.js"
        COLUMNS 132
        ROWS 42
        LABELS AgentSmoke
    )

    qmltui_add_scenario(
        NAME qtop_demo
        TARGET qmltui_qtop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 132
        ROWS 42
        CLOCK REAL
        RECORD qtop.cast
        LABELS Demo
    )
endif()
