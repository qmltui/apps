qt_add_executable(qmltui_plotting_dashboard src/main.cpp)

qt_add_qml_module(qmltui_plotting_dashboard
    URI PlottingDashboard
    QML_FILES
        qml/Main.qml
    DEPENDENCIES
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
)

target_link_libraries(qmltui_plotting_dashboard PRIVATE
    Qt6::Core
    Qt6::Qml
    QmlTui::Core
    QmlTui::Controls
)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_plotting_dashboard)
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_plotting_dashboard PRIVATE QmlTui::Runner)
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
