if(NOT TARGET QmlTui::Markdown AND NOT TARGET QmlTuiMarkdown)
    include(FetchContent)
    set(QMLTUI_MARKDOWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(QMLTUI_MARKDOWN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(_default_markdown_dir "${PROJECT_SOURCE_DIR}/../markdown")
    if(NOT FETCHCONTENT_SOURCE_DIR_QMLTUI_MARKDOWN AND EXISTS "${_default_markdown_dir}/CMakeLists.txt")
        FetchContent_Declare(
            qmltui_markdown
            SOURCE_DIR "${_default_markdown_dir}"
        )
    else()
        FetchContent_Declare(
            qmltui_markdown
            GIT_REPOSITORY https://github.com/qmltui/markdown.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )
    endif()
    FetchContent_MakeAvailable(qmltui_markdown)
endif()

add_subdirectory(agent_stream)
add_subdirectory(container_fleet)
add_subdirectory(package_manager)
if(QMLTUI_ENABLE_PRIVATE_QT)
    add_subdirectory(industrial_plc_hmi)
endif()
add_subdirectory(audio_player)
add_subdirectory(plotting_dashboard)
# /proc readers and Linux system providers: POSIX only.
if(UNIX AND NOT EMSCRIPTEN)
    if(QMLTUI_ENABLE_PRIVATE_QT)
        add_subdirectory(qtop)
    endif()
endif()
add_subdirectory(snake_game)
add_subdirectory(screensaver)
option(QMLTUI_BUILD_TURBO_HEX "Build the turbo_hex showcase (fetches QmlTui.TVision)" ON)
if(QMLTUI_BUILD_TURBO_HEX)
    add_subdirectory(turbo_hex)
endif()
if(QMLTUI_ENABLE_PRIVATE_QT)
    add_subdirectory(twozero48)
endif()

option(QMLTUI_BUILD_MARKDOWN_VIEWER "Build the markdown_viewer showcase (uses QmlTui.Markdown)" ON)
if(QMLTUI_BUILD_MARKDOWN_VIEWER)
    add_subdirectory(markdown_viewer)
endif()

if(QMLTUI_ENABLE_PTYVIEW AND TARGET QmlTui::Image)
    add_subdirectory(file_manager)
endif()
