# Qt Multimedia is optional: with it the example plays media files and streams;
# without it only the procedural demo source is available.
find_package(Qt6 OPTIONAL_COMPONENTS Multimedia QUIET)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Each executable owns the QML module (as table_view does) so Qt writes the
# build tree's QML import path into a qt.conf beside it and `import QmlTui.Controls`
# resolves when the binary is launched from Qt Creator or a shell. A shared
# static module library would not get that treatment, and since no C++ symbol
# of qmltui::controls is referenced the linker drops the library that carries
# the module's qmldir.
set(audio_player_sources
    AudioEngine.h
    AudioEngine.cpp
    SpectrumAnalyzer.h
    SpectrumAnalyzer.cpp
    DemoSynth.h
    DemoSynth.cpp
    Playlist.h
    Playlist.cpp
    WaveformTrace.h
    WaveformTrace.cpp
    ScopeArt.h
    ScopeArt.cpp
)
set(audio_player_qml_files
    Main.qml
    Theme.qml
    MainDeck.qml
    PlaylistDeck.qml
    VisualizerDeck.qml
    AddTrackDialog.qml
    KeyMapBar.qml
    KeyHint.qml
    VuMeter.qml
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
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        SOURCES ${audio_player_sources}
        QML_FILES ${audio_player_qml_files}
        DEPENDENCIES
            TARGET qmltui::core
            TARGET qmltui::controls
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE Qt6::Core Qt6::Qml qmltui::core qmltui::controls)
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

audio_player_add_executable(qmltui_audio_player AudioPlayer main.cpp)

if(QMLTUI_INSTALL_EXAMPLES)
    install(TARGETS qmltui_audio_player RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(TARGET qmltui::runner)
    target_link_libraries(qmltui_audio_player PRIVATE qmltui::runner)

    if(QMLTUI_BUILD_TESTS)
        qmltui_add_scenario_test(
            NAME audio_player_demo_smoke
            TARGET qmltui_audio_player
            SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_audio_player.js"
            COLUMNS 120
            ROWS 36
            ARTIFACT "audio_player/initial.txt"
            ARTIFACT_CONTENT "AUDIO PLAYER"
            LABELS AgentSmoke
        )

        qmltui_add_scenario_test(
            NAME audio_player_demo
            TARGET qmltui_audio_player
            SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/test_demo.js"
            COLUMNS 120
            ROWS 36
            RECORD audio_player.cast
            LABELS Demo
        )
    endif()
endif()

if(QMLTUI_BUILD_TESTS)
    qt_add_executable(test_audio_player_dsp test_audio_player_dsp.cpp ${audio_player_sources})
    target_include_directories(test_audio_player_dsp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test_audio_player_dsp PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Test
        qmltui::core
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
