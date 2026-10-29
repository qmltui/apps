set(btop_sources
    BtopModel.cpp
    BtopModel.h
)

qt_add_executable(qmltui_btop
    main.cpp
    ${btop_sources}
)

qt_add_qml_module(qmltui_btop
    URI BtopDemo
    VERSION 1.0
    QML_FILES
        Main.qml
        BtopPanel.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_link_libraries(qmltui_btop PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui::core
    qmltui::controls
)

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_btop RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_btop PRIVATE qmltui::runner)

    qmltui_add_scenario(
        NAME btop_smoke
        TARGET qmltui_btop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_btop.js"
        COLUMNS 132
        ROWS 42
        LABELS AgentSmoke
    )

    # Only timer-driven refreshes may repaint while the user is idle.
    qmltui_add_scenario(
        NAME btop_idle_budget
        TARGET qmltui_btop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_idle_budget.js"
        COLUMNS 132
        ROWS 42
        LABELS AgentSmoke
    )

    qmltui_add_scenario(
        NAME btop_demo
        TARGET qmltui_btop
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 132
        ROWS 42
        CLOCK VIRTUAL
        RECORD btop.cast
        LABELS Demo
    )
endif()
