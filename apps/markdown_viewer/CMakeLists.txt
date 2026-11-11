find_package(Qt6 REQUIRED COMPONENTS Gui)
include(FetchContent)

set(BUILD_MD4QT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_MD4QT_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BUILD_MD2HTML_APP OFF CACHE BOOL "" FORCE)
set(BUILD_MD2QDOC_APP OFF CACHE BOOL "" FORCE)
set(INSTALL_MD4QT OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    md4qt
    GIT_REPOSITORY https://github.com/KDE/md4qt.git
    GIT_TAG 1a878810adeeb534de4de395f4951edd54ab6072
)
FetchContent_MakeAvailable(md4qt)

qt_add_library(qmltui_markdown_viewer_module STATIC)

qt_add_qml_module(qmltui_markdown_viewer_module
    URI QmlTui.Examples.MarkdownViewer
    VERSION 1.0
    SOURCES
        src/MdRole.h
        src/MdPalette.h
        src/MdPalette.cpp
        src/MdDocStyle.h
        src/MdDocStyle.cpp
        src/MdViewStyle.h
        src/MdViewStyle.cpp
        src/MdDoc.h
        src/MdDoc.cpp
        src/SyntaxHighlighter.h
        src/SyntaxHighlighter.cpp
    QML_FILES
        qml/Main.qml
        qml/MdView.qml
        qml/Theme.qml
        qml/ModernDocStyle.qml
        qml/ClassicDocStyle.qml
        qml/MinimalDocStyle.qml
        qml/StyleEditor.qml
        qml/HeaderBar.qml
        qml/StatusBar.qml
        qml/SearchBar.qml
    RESOURCES
        resources/modern_showcase.md
        resources/welcome.md
        resources/qmltui_guide.md
        resources/cheatsheet.md
        resources/edge_cases.md
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_include_directories(qmltui_markdown_viewer_module
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE ${md4qt_SOURCE_DIR}/src ${md4qt_SOURCE_DIR}
)

target_compile_definitions(qmltui_markdown_viewer_module PRIVATE MD4QT_QT_SUPPORT)

target_link_libraries(qmltui_markdown_viewer_module PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Gui
    qmltui::core
    qmltui::controls
    md4qt::md4qt
)

qt_add_executable(qmltui_markdown_viewer src/main.cpp)
target_link_libraries(qmltui_markdown_viewer PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui_markdown_viewer_moduleplugin
)

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_markdown_viewer RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_markdown_viewer PRIVATE qmltui::runner)

    qmltui_add_scenario(
        NAME markdown_viewer_scenario_wide
        TARGET qmltui_markdown_viewer
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/markdown_viewer.js"
        COLUMNS 100
        ROWS 32
        SCENARIO_TIMEOUT_MS 15000
        LABELS AgentSmoke
    )
    qmltui_add_scenario(
        NAME markdown_viewer_scenario_narrow
        TARGET qmltui_markdown_viewer
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/markdown_viewer.js"
        COLUMNS 70
        ROWS 24
        SCENARIO_TIMEOUT_MS 15000
        LABELS AgentSmoke
    )
    qmltui_add_scenario(
        NAME markdown_viewer_demo
        TARGET qmltui_markdown_viewer
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 100
        ROWS 32
        CLOCK REAL
        SCENARIO_TIMEOUT_MS 30000
        RECORD markdown_viewer.cast
        LABELS Demo
    )
endif()

if(BUILD_TESTING)
    qt_add_executable(test_markdown_viewer_document tests/markdown_viewer.cpp)
    target_link_libraries(test_markdown_viewer_document PRIVATE
        Qt6::Core
        Qt6::Test
        qmltui::core
        qmltui_markdown_viewer_module
    )
    add_test(NAME markdown_viewer_document COMMAND test_markdown_viewer_document)
    set_tests_properties(markdown_viewer_document PROPERTIES LABELS AgentSmoke)
endif()
