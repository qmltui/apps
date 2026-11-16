qt_add_executable(qmltui_plotting_dashboard src/main.cpp)

qt_add_qml_module(qmltui_plotting_dashboard
    URI PlottingDashboard
    VERSION 1.0
    QML_FILES
        qml/Main.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_link_libraries(qmltui_plotting_dashboard PRIVATE
    Qt6::Core
    Qt6::Qml
    qmltui::core
    qmltui::controls
)

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_plotting_dashboard PRIVATE qmltui::runner)
    qmltui_add_scenario(
        NAME test_runner_plotting_dashboard
        TARGET qmltui_plotting_dashboard
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/plotting_dashboard.js"
        COLUMNS 100
        ROWS 32
        LABELS AgentSmoke
    )
    qmltui_add_scenario(
        NAME plotting_dashboard_demo
        TARGET qmltui_plotting_dashboard
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 100
        ROWS 32
        CLOCK REAL
        RECORD plotting_dashboard.cast
        LABELS Demo
    )
endif()
