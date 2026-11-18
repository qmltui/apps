cmake_minimum_required(VERSION 3.21)
project(QmlTuiApps VERSION 0.6.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)

option(QMLTUI_USE_INSTALLED "Consume an installed QmlTui 0.6 package instead of FetchContent" OFF)
option(QMLTUI_ENABLE_PRIVATE_QT "Enable features requiring private Qt modules (Behavior, advanced states)" ON)
option(QMLTUI_ENABLE_IMAGE "Build the optional QmlTui Image module (requires Qt Gui and Network)" ON)
option(QMLTUI_ENABLE_RUNNER "Build the optional QmlTui Runner library and CLI" ON)
if(UNIX)
    option(QMLTUI_ENABLE_PTYVIEW "Build the optional Unix PTY PtyView module" ON)
endif()

if(QMLTUI_USE_INSTALLED)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml)
    qt_standard_project_setup(REQUIRES 6.8)
    find_package(QmlTui 0.6 REQUIRED)
else()
    include(FetchContent)
    set(QMLTUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QMLTUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(QMLTUI_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(qmltui
        GIT_REPOSITORY https://github.com/qmltui/qmltui.git
        GIT_TAG main # Pin a release tag (e.g. v0.6.0) once published
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(qmltui)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml)
    qt_standard_project_setup(REQUIRES 6.8)
endif()

set(QMLTUI_RECORD_DIR "${PROJECT_SOURCE_DIR}/docs/assets/casts")
file(MAKE_DIRECTORY "${QMLTUI_RECORD_DIR}")
find_program(QMLTUI_AGG_EXECUTABLE NAMES agg HINTS "$ENV{HOME}/.cargo/bin")
if(QMLTUI_AGG_EXECUTABLE)
    set(QMLTUI_RECORD_POST_COMMAND
        "${QMLTUI_AGG_EXECUTABLE}"
        --theme asciinema
        --text-font-family "JetBrains Mono,Fira Code,SF Mono,Menlo,Consolas,Adwaita Mono,JetBrainsMono Nerd Font,Noto Sans Symbols 2,DejaVu Sans Mono,Liberation Mono"
        --quiet
        "<CAST>"
        "<CAST_STEM>.gif"
    )
endif()
if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
endif()
add_subdirectory(apps)
if(COMMAND qmltui_scenario_editor_support)
    qmltui_scenario_editor_support()
endif()
