set_source_files_properties(qml/HmiTheme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/PlcTypes.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(plc_qml_files
    qml/Main.qml
    qml/PlcController.qml
    qml/HmiTheme.qml
    qml/PlcTypes.qml
    qml/HeaderPanel.qml
    qml/NavTabBar.qml
    qml/StatusAnnunciatorBar.qml
    qml/KeyHint.qml
    qml/KeyMapBar.qml
    qml/DashboardView.qml
    qml/TelemetryPanel.qml
    qml/TrendScopePanel.qml
    qml/ProcessMimicPanel.qml
    qml/ControlsPanel.qml
    qml/Gauge.qml
    qml/ActuatorLed.qml
    qml/AlarmsView.qml
    qml/DiagnosticsView.qml
    qml/SettingsView.qml
    qml/ConfirmDialog.qml
)

qt_add_executable(qmltui_industrial_plc_hmi src/main.cpp)

set(plc_resources)
if(BUILD_TESTING)
    list(APPEND plc_resources tests/plc.js)
endif()

qt_add_qml_module(qmltui_industrial_plc_hmi
    URI IndustrialPlcHmi
    QML_FILES ${plc_qml_files}
    RESOURCES ${plc_resources}

    DEPENDENCIES
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
)

target_link_libraries(qmltui_industrial_plc_hmi PRIVATE
    Qt6::Core
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
)

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_industrial_plc_hmi PRIVATE QmlTui::Runner)
    # Stays on the real clock: the scenario waits on simulation state with
    # waitForCondition(), which does not advance virtual time.
    qmltui_add_scenario(
        NAME test_runner_plc TARGET qmltui_industrial_plc_hmi
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/plc.js"
        LABELS AgentFull
        PROCESS_TIMEOUT 35
        OPERATION_TIMEOUT_MS 5000
        SCENARIO_TIMEOUT_MS 30000
        ARTIFACT "plc/reset.txt"
        ARTIFACT_CONTENT "STATE: IDLE (ALARM RESET)"
    )
    qmltui_add_scenario(
        NAME industrial_plc_hmi_demo TARGET qmltui_industrial_plc_hmi
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 110 ROWS 34
        CLOCK REAL
        RECORD industrial_plc_hmi.cast
        LABELS Demo
    )
endif()
