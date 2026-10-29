set_source_files_properties(HmiTheme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(PlcTypes.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(plc_qml_files
    Main.qml
    HmiTheme.qml
    PlcTypes.qml
    Gauge.qml
    ActuatorLed.qml
    HeaderPanel.qml
    ConfirmDialog.qml
    NavTabBar.qml
    AlarmsView.qml
    DiagnosticsView.qml
    SettingsView.qml
)

qt_add_executable(qmltui_industrial_plc_hmi main.cpp)

qt_add_qml_module(qmltui_industrial_plc_hmi
    URI IndustrialPlcHmi
    VERSION 1.0
    QML_FILES ${plc_qml_files}

    RESOURCES
        test_plc.js

    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_link_libraries(qmltui_industrial_plc_hmi PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui::core
    qmltui::controls
)

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_industrial_plc_hmi PRIVATE qmltui::runner)
    # Stays on the real clock: the scenario waits on simulation state with
    # waitForCondition(), which does not advance virtual time.
    qmltui_add_scenario(
        NAME test_runner_plc TARGET qmltui_industrial_plc_hmi
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_plc.js"
        LABELS AgentFull
        PROCESS_TIMEOUT 35
        OPERATION_TIMEOUT_MS 5000
        SCENARIO_TIMEOUT_MS 30000
        ARTIFACT "plc/reset.txt"
        ARTIFACT_CONTENT "STATE: IDLE (ALARM RESET)"
    )
    qmltui_add_scenario(
        NAME industrial_plc_hmi_demo TARGET qmltui_industrial_plc_hmi
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 110 ROWS 34
        CLOCK VIRTUAL
        RECORD industrial_plc_hmi.cast
        LABELS Demo
    )
endif()
