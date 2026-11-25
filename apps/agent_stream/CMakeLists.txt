cmake_minimum_required(VERSION 3.20)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(agent_stream_sources
    src/AgentTransport.h
    src/MockAgentTransport.h
    src/MockAgentTransport.cpp
    src/AgentModels.h
    src/AgentModels.cpp
    src/AgentSession.h
    src/AgentSession.cpp
)

set(agent_stream_qml_files
    qml/StatusBadge.qml
    qml/Main.qml
    qml/Theme.qml
    qml/BannerCard.qml
    qml/LiveStreamCard.qml
    qml/UserPromptCard.qml
    qml/ThoughtCard.qml
    qml/ToolCallCard.qml
    qml/ToolResultCard.qml
    qml/DiffCard.qml
    qml/PermissionCard.qml
    qml/AgentResponseCard.qml
    qml/SystemMessageCard.qml
    qml/ErrorMessageCard.qml
    qml/PinnedPromptBar.qml
    qml/SwarmTableCard.qml
    qml/RepoTreeCard.qml
    qml/CommandPalette.qml
    qml/ModelPickerModal.qml
    qml/DiffInspectorModal.qml
    qml/ToastNotification.qml
)

set(_agent_stream_deps
    DEPENDENCIES
        TARGET QmlTuiMarkdown
)
get_target_property(_qmltui_is_imported QmlTui::Core IMPORTED)
if(_qmltui_is_imported)
    if(QmlTui_QML_IMPORT_PATH)
        list(APPEND _agent_stream_deps IMPORT_PATH "${QmlTui_QML_IMPORT_PATH}")
    endif()
else()
    list(APPEND _agent_stream_deps
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
    )
endif()

qt_add_executable(qmltui_agent_stream src/main.cpp)

qt_add_qml_module(qmltui_agent_stream
    URI AgentStreamDemo
    SOURCES ${agent_stream_sources}
    QML_FILES ${agent_stream_qml_files}
    ${_agent_stream_deps}
)

target_include_directories(qmltui_agent_stream PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(qmltui_agent_stream PRIVATE
    Qt6::Core
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
    QmlTui::Markdown
)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_agent_stream)
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_agent_stream PRIVATE QmlTui::Runner)
    qmltui_add_scenario(NAME agent_stream_inline TARGET qmltui_agent_stream
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/inline.js"
        COLUMNS 120 ROWS 35 CLOCK VIRTUAL LABELS AgentSmoke
        ARTIFACT "banner.txt" ARTIFACT_CONTENT "QmlTui")
    qmltui_add_scenario(NAME agent_stream_prompt_focus TARGET qmltui_agent_stream
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/prompt_focus.js"
        COLUMNS 120 ROWS 35 CLOCK VIRTUAL LABELS AgentSmoke)
    qmltui_add_scenario(NAME agent_stream_mouse TARGET qmltui_agent_stream
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/mouse.js"
        COLUMNS 120 ROWS 35 CLOCK VIRTUAL LABELS AgentSmoke)
    qmltui_add_scenario(NAME agent_stream_tree TARGET qmltui_agent_stream
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/tree.js"
        COLUMNS 120 ROWS 35 CLOCK VIRTUAL LABELS AgentSmoke
        ARTIFACT "tree_styled.txt" ARTIFACT_CONTENT "y=2 x=0-3 fg=#38BDF8FF bg=default")
    qmltui_add_scenario(NAME agent_stream_demo TARGET qmltui_agent_stream
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 120 ROWS 35 CLOCK REAL
        RECORD agent_stream.cast LABELS Demo)
endif()

if(BUILD_TESTING)
    qt_add_executable(test_agent_stream_models
        tests/test_agent_models.cpp
        ${agent_stream_sources}
    )
    target_include_directories(test_agent_stream_models PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(test_agent_stream_models PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        QmlTui::Core
    )
    add_test(NAME agent_stream_models COMMAND test_agent_stream_models)
    set_tests_properties(agent_stream_models PROPERTIES LABELS "Unit;AgentSmoke")
endif()
