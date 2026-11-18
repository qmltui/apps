cmake_minimum_required(VERSION 3.20)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(package_manager_sources
    src/PackageModels.h
    src/PackageModels.cpp
)

set(package_manager_qml_files
    qml/Main.qml
    qml/Theme.qml
    qml/BannerCard.qml
    qml/StepLogRow.qml
    qml/SummaryPanelCard.qml
    qml/TransactionPromptView.qml
    qml/LiveExecutionPanel.qml
)

qt_add_executable(qmltui_package_manager src/main.cpp)

qt_add_qml_module(qmltui_package_manager
    URI PackageManagerApp
    RESOURCE_PREFIX "/qt/qml"
    SOURCES ${package_manager_sources}
    QML_FILES ${package_manager_qml_files}
    DEPENDENCIES
        TARGET qmltui::core
        TARGET qmltui::controls
)

target_include_directories(qmltui_package_manager PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(qmltui_package_manager PRIVATE Qt6::Core Qt6::Qml qmltui::core qmltui::controls)

if(BUILD_TESTING AND TARGET qmltui::runner)
    target_link_libraries(qmltui_package_manager PRIVATE qmltui::runner)
    qmltui_add_scenario(
        NAME package_manager_install
        TARGET qmltui_package_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/install.js"
        COLUMNS 104
        ROWS 28
        CLOCK VIRTUAL
        LABELS PackageManagerSmoke
    )
    qmltui_add_scenario(
        NAME package_manager_demo
        TARGET qmltui_package_manager
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 108
        ROWS 30
        CLOCK REAL
        RECORD package_manager.cast
        LABELS Demo
    )
endif()

if(BUILD_TESTING)
    qt_add_executable(test_package_manager_engine
        tests/package_manager_engine.cpp
        ${package_manager_sources}
    )
    target_include_directories(test_package_manager_engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(test_package_manager_engine PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        qmltui::core
    )
    add_test(NAME package_manager_engine COMMAND test_package_manager_engine)
    set_tests_properties(package_manager_engine PROPERTIES LABELS PackageManagerSmoke)
endif()
