set(qtop_sources
    src/QtopModel.cpp
    src/QtopModel.h
)

qt_add_executable(qmltui_qtop
    src/main.cpp
)

qt_add_qml_module(qmltui_qtop
    URI QtopDemo
    SOURCES
        ${qtop_sources}
    QML_FILES
        qml/Main.qml
        qml/QtopPanel.qml
    DEPENDENCIES
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
)

target_include_directories(qmltui_qtop PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(qmltui_qtop PRIVATE
    Qt6::Core
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
)

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_qtop PRIVATE QmlTui::Runner)

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
