set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

find_package(Qt6 6.8 OPTIONAL_COMPONENTS Gui Svg Pdf QUIET)

if(NOT TARGET Qt6::Gui)
    message(STATUS "Skipping file_manager: Qt6::Gui not found")
    return()
endif()

set(_file_manager_deps
    DEPENDENCIES
        TARGET QmlTuiMarkdown
        TARGET QmlTuiPtyView
)
get_target_property(_qmltui_is_imported QmlTui::Core IMPORTED)
if(_qmltui_is_imported)
    if(QmlTui_QML_IMPORT_PATH)
        list(APPEND _file_manager_deps IMPORT_PATH "${QmlTui_QML_IMPORT_PATH}")
    endif()
else()
    list(APPEND _file_manager_deps
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
        TARGET QmlTui::Image
    )
endif()

qt_add_executable(qmltui_file_manager src/main.cpp)

qt_add_qml_module(qmltui_file_manager
    URI QmlTui.Examples.FileManager
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
    ${_file_manager_deps}
)

target_include_directories(qmltui_file_manager PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(qmltui_file_manager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
    QmlTui::Image
    QmlTui::PtyView
    QmlTui::Markdown
)

if(TARGET Qt6::Svg)
    target_link_libraries(qmltui_file_manager PRIVATE Qt6::Svg)
    target_compile_definitions(qmltui_file_manager PRIVATE FILE_MANAGER_HAVE_SVG=1)
else()
    target_compile_definitions(qmltui_file_manager PRIVATE FILE_MANAGER_HAVE_SVG=0)
    message(STATUS "file_manager: Qt6::Svg not found, live SVG preview disabled")
endif()

if(TARGET Qt6::Pdf)
    target_link_libraries(qmltui_file_manager PRIVATE Qt6::Pdf)
    target_compile_definitions(qmltui_file_manager PRIVATE FILE_MANAGER_HAVE_PDF=1)
else()
    target_compile_definitions(qmltui_file_manager PRIVATE FILE_MANAGER_HAVE_PDF=0)
    message(STATUS "file_manager: Qt6::Pdf not found, live PDF preview disabled")
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_file_manager PRIVATE QmlTui::Runner)

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
