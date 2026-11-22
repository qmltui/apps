# Qt Multimedia is optional: with it the example plays media files and streams;
# without it only the procedural demo source is available.
find_package(Qt6 OPTIONAL_COMPONENTS Multimedia QUIET)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Each executable owns the QML module (as table_view does) so Qt writes the
# build tree's QML import path into a qt.conf beside it and `import QmlTui.Controls`
# resolves when the binary is launched from Qt Creator or a shell. A shared
# static module library would not get that treatment, and since no C++ symbol
# of QmlTui::Controls is referenced the linker drops the library that carries
# the module's qmldir.
set(audio_player_sources
    src/AudioEngine.h
    src/AudioEngine.cpp
    src/SpectrumAnalyzer.h
    src/SpectrumAnalyzer.cpp
    src/DemoSynth.h
    src/DemoSynth.cpp
    src/Playlist.h
    src/Playlist.cpp
    src/WaveformTrace.h
    src/WaveformTrace.cpp
    src/ScopeArt.h
    src/ScopeArt.cpp
)
set(audio_player_qml_files
    qml/Main.qml
    qml/Theme.qml
    qml/MainDeck.qml
    qml/PlaylistDeck.qml
    qml/SpectrumDeck.qml
    qml/VisualizerDeck.qml
    qml/AddTrackDialog.qml
    qml/KeyMapBar.qml
    qml/KeyHint.qml
    qml/VuMeter.qml
)

# Adds an executable that compiles the module under its own URI.
function(audio_player_add_executable target uri main_source)
    # An optional fourth argument gives the module its own output directory, so
    # a second executable can compile the same URI without colliding on disk.
    set(module_output_directory)
    if(ARGC GREATER 3)
        set(module_output_directory OUTPUT_DIRECTORY "${ARGV3}")
    endif()
    qt_add_executable(${target} ${main_source})
    qt_add_qml_module(${target}
        URI ${uri}
        ${module_output_directory}
        RESOURCE_PREFIX "/qt/qml"
        SOURCES ${audio_player_sources}
        QML_FILES ${audio_player_qml_files}
        DEPENDENCIES
            TARGET QmlTui::Core
            TARGET QmlTui::Controls
    )
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE Qt6::Core Qt6::Qml QmlTui::Core QmlTui::Controls)
    if(TARGET Qt6::Multimedia)
        target_link_libraries(${target} PRIVATE Qt6::Multimedia)
        target_compile_definitions(${target} PRIVATE AUDIO_PLAYER_HAVE_MULTIMEDIA=1)
    else()
        target_compile_definitions(${target} PRIVATE AUDIO_PLAYER_HAVE_MULTIMEDIA=0)
    endif()
endfunction()

if(TARGET Qt6::Multimedia)
    message(STATUS "audio_player: Qt Multimedia found, file playback enabled")
else()
    message(STATUS "audio_player: Qt Multimedia not found, demo source only")
endif()

audio_player_add_executable(qmltui_audio_player AudioPlayer src/main.cpp)

if(COMMAND qmltui_wasm_app)
    qmltui_wasm_app(qmltui_audio_player)
endif()

if(BUILD_TESTING AND TARGET QmlTui::Runner)
    target_link_libraries(qmltui_audio_player PRIVATE QmlTui::Runner)

    qmltui_add_scenario(
        NAME audio_player_demo_smoke
        TARGET qmltui_audio_player
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_player.js"
        COLUMNS 120
        ROWS 36
        ARTIFACT "audio_player/initial.txt"
        ARTIFACT_CONTENT "AUDIO PLAYER"
        LABELS AgentSmoke
    )

    qmltui_add_scenario(
        NAME audio_player_demo
        TARGET qmltui_audio_player
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/demo.js"
        COLUMNS 120
        ROWS 36
        CLOCK REAL
        RECORD audio_player.cast
        LABELS Demo
    )
endif()

if(BUILD_TESTING)
    qt_add_executable(test_audio_player_dsp tests/audio_player_dsp.cpp ${audio_player_sources})
    target_include_directories(test_audio_player_dsp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(test_audio_player_dsp PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        QmlTui::Core
    )
    if(TARGET Qt6::Multimedia)
        target_link_libraries(test_audio_player_dsp PRIVATE Qt6::Multimedia)
        target_compile_definitions(test_audio_player_dsp PRIVATE AUDIO_PLAYER_HAVE_MULTIMEDIA=1)
    else()
        target_compile_definitions(test_audio_player_dsp PRIVATE AUDIO_PLAYER_HAVE_MULTIMEDIA=0)
    endif()
    add_test(NAME audio_player_dsp COMMAND test_audio_player_dsp)
    set_tests_properties(audio_player_dsp PROPERTIES LABELS AgentSmoke)
endif()
