set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

find_package(Qt6 6.8 REQUIRED COMPONENTS Gui Svg Pdf)

qt_add_executable(qmltui_file_manager src/main.cpp)

qt_add_qml_module(qmltui_file_manager
    URI QmlTui.Examples.FileManager
    VERSION 1.0
    SOURCES
        src/FileEntryFormatter.h
        src/FileEntryFormatter.cpp
        src/FileManagerBackend.h
        src/FileManagerBackend.cpp
        src/MockFileSystem.h
        src/MockFileSystem.cpp
        src/PreviewImageProvider.h
        src/PreviewImageProvider.cpp
        src/PreviewSyntaxHighlighter.h
        src/PreviewSyntaxHighlighter.cpp
        src/ShellProcessHelper.h
        src/ShellProcessHelper.cpp
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/HeaderBar.qml
        qml/StatusBar.qml
        qml/ParentColumn.qml
        qml/CurrentColumn.qml
        qml/PreviewColumn.qml
        qml/HelpDialog.qml
        qml/InputDialog.qml
        qml/ConfirmDialog.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
        TARGET qmltui::image
        TARGET qmltui::pty_view
)

target_include_directories(qmltui_file_manager PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(qmltui_file_manager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Svg
    Qt6::Pdf
    qmltui::core
    qmltui::controls
    qmltui::image
    qmltui::pty_view
)

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_file_manager RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_file_manager PRIVATE qmltui::runner)

    # idle_budget asserts the application does not repaint while nothing changes.
    foreach(scenario navigation preview search actions idle_budget)
        qmltui_add_scenario(
            NAME file_manager_${scenario}
            TARGET qmltui_file_manager
            SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/${scenario}.js"
            COLUMNS 100
            ROWS 30
            LABELS AgentSmoke
        )
    endforeach()

    qmltui_add_scenario(
        NAME file_manager_demo
        TARGET qmltui_file_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 120
        ROWS 30
        CLOCK REAL
        RECORD file_manager.cast
        LABELS Demo
    )
endif()
