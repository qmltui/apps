cmake_minimum_required(VERSION 3.20)

qt_add_executable(qmltui_package_manager src/main.cpp)

qt_add_qml_module(qmltui_package_manager
    URI PackageManagerApp
    VERSION 1.0
    QML_FILES qml/Main.qml
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_link_libraries(qmltui_package_manager PRIVATE Qt6::Core Qt6::Qml qmltui::core qmltui::controls)

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_package_manager PRIVATE qmltui::runner)
    qmltui_add_scenario(NAME package_manager_install TARGET qmltui_package_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/install.js"
        COLUMNS 88 ROWS 24 CLOCK VIRTUAL LABELS PackageManagerSmoke)
    qmltui_add_scenario(NAME package_manager_demo TARGET qmltui_package_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 96 ROWS 24 CLOCK REAL
        RECORD package_manager.cast LABELS Demo)
endif()
