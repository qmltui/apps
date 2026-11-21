cmake_minimum_required(VERSION 3.20)

set(agent_stream_qml_files
    qml/StatusBadge.qml
    qml/Main.qml
    qml/Theme.qml
    qml/BannerCard.qml
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

qt_add_executable(qmltui_agent_stream src/main.cpp)

qt_add_qml_module(qmltui_agent_stream
    URI AgentStreamDemo
    QML_FILES ${agent_stream_qml_files}
    DEPENDENCIES
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
)

target_link_libraries(qmltui_agent_stream PRIVATE Qt6::Core Qt6::Qml QmlTui::Core QmlTui::Controls)

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
