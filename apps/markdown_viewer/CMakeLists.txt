if(NOT TARGET QmlTui::Markdown AND NOT TARGET QmlTuiMarkdown)
    include(FetchContent)
    set(QMLTUI_MARKDOWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(QMLTUI_MARKDOWN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(_default_markdown_dir "${PROJECT_SOURCE_DIR}/../markdown")
    if(NOT FETCHCONTENT_SOURCE_DIR_QMLTUI_MARKDOWN AND EXISTS "${_default_markdown_dir}/CMakeLists.txt")
        FetchContent_Declare(
            qmltui_markdown
            SOURCE_DIR "${_default_markdown_dir}"
        )
    else()
        FetchContent_Declare(
            qmltui_markdown
            GIT_REPOSITORY https://github.com/qmltui/markdown.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )
    endif()
    FetchContent_MakeAvailable(qmltui_markdown)
endif()

set(_markdown_viewer_deps
    DEPENDENCIES
        TARGET QmlTuiMarkdown
)
get_target_property(_qmltui_is_imported QmlTui::Core IMPORTED)
if(_qmltui_is_imported)
    if(QmlTui_QML_IMPORT_PATH)
        list(APPEND _markdown_viewer_deps IMPORT_PATH "${QmlTui_QML_IMPORT_PATH}")
    endif()
else()
    list(APPEND _markdown_viewer_deps
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
    )
endif()

qt_add_executable(qmltui_markdown_viewer src/main.cpp)

qt_add_qml_module(qmltui_markdown_viewer
    URI QmlTui.Examples.MarkdownViewer
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
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
    ${_markdown_viewer_deps}
)

target_link_libraries(qmltui_markdown_viewer PRIVATE
    Qt6::Core
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
    QmlTui::Markdown
)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_markdown_viewer)
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_markdown_viewer PRIVATE QmlTui::Runner)

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
