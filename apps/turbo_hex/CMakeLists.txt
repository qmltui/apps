if(NOT TARGET QmlTui::TVision AND NOT TARGET QmlTuiTVision)
    include(FetchContent)
    set(QMLTUI_TVISION_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(QMLTUI_TVISION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(_default_tvision_dir "${PROJECT_SOURCE_DIR}/../tvision")
    if(NOT FETCHCONTENT_SOURCE_DIR_QMLTUI_TVISION AND EXISTS "${_default_tvision_dir}/CMakeLists.txt")
        FetchContent_Declare(
            qmltui_tvision
            SOURCE_DIR "${_default_tvision_dir}"
        )
    else()
        FetchContent_Declare(
            qmltui_tvision
            GIT_REPOSITORY https://github.com/qmltui/tvision.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )
    endif()
    FetchContent_MakeAvailable(qmltui_tvision)
endif()

if(TARGET QmlTui::Core)
    set(_th_core_target QmlTui::Core)
    set(_th_controls_target QmlTui::Controls)
else()
    set(_th_core_target qmltui::core)
    set(_th_controls_target qmltui::controls)
endif()

if(TARGET QmlTui::Runner)
    set(_th_runner_target QmlTui::Runner)
elseif(TARGET qmltui::runner)
    set(_th_runner_target qmltui::runner)
else()
    set(_th_runner_target "")
endif()

set(_turbo_hex_deps
    DEPENDENCIES
        TARGET QmlTuiTVision
)
get_target_property(_qmltui_is_imported ${_th_core_target} IMPORTED)
if(_qmltui_is_imported)
    if(QmlTui_QML_IMPORT_PATH)
        list(APPEND _turbo_hex_deps IMPORT_PATH "${QmlTui_QML_IMPORT_PATH}")
    endif()
else()
    list(APPEND _turbo_hex_deps
        TARGET ${_th_core_target}
        TARGET ${_th_controls_target}
    )
endif()

qt_add_executable(qmltui_turbo_hex
    src/main.cpp
)

qt_add_qml_module(qmltui_turbo_hex
    URI TurboHexApp
    SOURCES
        src/BinaryDocument.h
        src/BinaryDocument.cpp
        src/HexBackend.h
        src/HexBackend.cpp
        src/HexGridView.h
        src/HexGridView.cpp
    QML_FILES
        qml/Main.qml
        qml/HexWindow.qml
        qml/InspectorWindow.qml
        qml/StructureWindow.qml
        qml/FileOpenDialog.qml
        qml/SaveAsDialog.qml
        qml/FindBytesDialog.qml
        qml/GoToOffsetDialog.qml
        qml/UnsavedChangesDialog.qml
        qml/PaletteDialog.qml
        qml/AboutDialog.qml
    ${_turbo_hex_deps}
)

target_include_directories(qmltui_turbo_hex PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(qmltui_turbo_hex PRIVATE
    Qt6::Core
    Qt6::Qml
    ${_th_core_target}
    ${_th_controls_target}
    QmlTui::TVision
)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_turbo_hex)
endif()

if(BUILD_TESTING)
    qt_add_executable(tst_turbo_hex
        tests/tst_turbo_hex.cpp
        src/BinaryDocument.h
        src/BinaryDocument.cpp
        src/HexBackend.h
        src/HexBackend.cpp
        src/HexGridView.h
        src/HexGridView.cpp
    )
    target_include_directories(tst_turbo_hex PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(tst_turbo_hex PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        ${_th_core_target}
        ${_th_controls_target}
        QmlTui::TVision
    )
    add_test(NAME tst_turbo_hex COMMAND tst_turbo_hex)
    set_tests_properties(tst_turbo_hex PROPERTIES LABELS "Unit")
endif()

if(BUILD_TESTING AND _th_runner_target)
    target_link_libraries(qmltui_turbo_hex PRIVATE ${_th_runner_target})
    qmltui_add_scenario(
        NAME test_runner_turbo_hex
        TARGET qmltui_turbo_hex
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/turbo_hex.js"
        COLUMNS 110
        ROWS 32
        CLOCK VIRTUAL
        ARTIFACT "turbo_hex/final_check.txt"
        ARTIFACT_CONTENT "Turbo Hex"
    )
    if(NOT EMSCRIPTEN)
        qmltui_add_scenario(
            NAME test_runner_turbo_hex_cli
            TARGET qmltui_turbo_hex
            SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/turbo_hex_cli.js"
            ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/sample.bin"
            COLUMNS 110
            ROWS 32
            CLOCK VIRTUAL
        )
    endif()
    qmltui_add_scenario(
        NAME turbo_hex_demo
        TARGET qmltui_turbo_hex
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 114
        ROWS 34
        CLOCK VIRTUAL
        RECORD turbo_hex.cast
        LABELS Demo
    )
endif()
