set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_executable(qmltui_file_manager main.cpp)

qt_add_qml_module(qmltui_file_manager
    URI QmlTui.Examples.FileManager
    VERSION 1.0
    SOURCES
        FileManagerBackend.h
        FileManagerBackend.cpp
    QML_FILES
        Main.qml
        Theme.qml
        HeaderBar.qml
        StatusBar.qml
        ParentColumn.qml
        CurrentColumn.qml
        PreviewColumn.qml
        HelpDialog.qml
        InputDialog.qml
        ConfirmDialog.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
        TARGET qmltui::pty_view
)

target_link_libraries(qmltui_file_manager PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui::core
    qmltui::controls
    qmltui::pty_view
)

if(TARGET qmltui::runner)
    target_link_libraries(qmltui_file_manager PRIVATE qmltui::runner)
endif()

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_file_manager RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(TARGET qmltui::runner AND QMLTUI_BUILD_TESTS)
    # idle_budget asserts the application does not repaint while nothing changes.
    foreach(scenario navigation preview search actions idle_budget)
        qmltui_add_scenario_test(
            NAME file_manager_${scenario}
            TARGET qmltui_file_manager
            SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_${scenario}.js"
            COLUMNS 100
            ROWS 30
            LABELS AgentSmoke
        )
    endforeach()

    qmltui_add_scenario_test(
        NAME file_manager_demo
        TARGET qmltui_file_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 120
        ROWS 30
        CLOCK REAL
        RECORD file_manager.cast
        LABELS Demo
    )
endif()
