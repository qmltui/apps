qt_add_executable(qmltui_container_fleet
    src/main.cpp
)

qt_add_qml_module(qmltui_container_fleet
    URI ContainerFleetApp
    SOURCES
        src/FleetListModel.cpp
        src/FleetListModel.h
    QML_FILES
        qml/Main.qml
    DEPENDENCIES
        TARGET QmlTui::Core
        TARGET QmlTui::Controls
)

target_include_directories(qmltui_container_fleet PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(qmltui_container_fleet PRIVATE Qt6::Core Qt6::Qml QmlTui::Core QmlTui::Controls)

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_container_fleet PRIVATE QmlTui::Runner)
    qmltui_add_scenario(
        NAME test_runner_container_fleet TARGET qmltui_container_fleet
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/fleet.js"
        ARTIFACT "fleet/critical.txt"
        ARTIFACT_CONTENT "CRITICAL"
    )
    qmltui_add_scenario(
        NAME container_fleet_demo TARGET qmltui_container_fleet
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 110 ROWS 32
        CLOCK REAL
        RECORD container_fleet.cast
        LABELS Demo
    )
endif()
