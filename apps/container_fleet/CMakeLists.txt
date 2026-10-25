qt_add_executable(qmltui_container_fleet main.cpp FleetListModel.cpp FleetListModel.h)

qt_add_qml_module(qmltui_container_fleet
    URI ContainerFleetApp
    VERSION 1.0
    SOURCES
        FleetListModel.cpp
        FleetListModel.h
    QML_FILES
        Main.qml

 DEPENDENCIES TARGET qmltui::core qmltui::controls
)

target_link_libraries(qmltui_container_fleet PRIVATE Qt6::Core Qt6::Qml qmltui::core qmltui::controls qmltui::controls)

if(TARGET qmltui::runner)
    target_link_libraries(qmltui_container_fleet PRIVATE qmltui::runner)
endif()

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_container_fleet RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(BUILD_TESTING AND TARGET qmltui::runner)
    qmltui_add_scenario(
        NAME test_runner_container_fleet TARGET qmltui_container_fleet
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_fleet.js"
        ARTIFACT "fleet/critical.txt"
        ARTIFACT_CONTENT "CRITICAL"
    )
    qmltui_add_scenario(
        NAME container_fleet_demo TARGET qmltui_container_fleet
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
        COLUMNS 110 ROWS 32
        CLOCK VIRTUAL
        RECORD container_fleet.cast
        LABELS Demo
    )
endif()
