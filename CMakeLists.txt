cmake_minimum_required(VERSION 3.24)

project(KickBassAssistant VERSION 0.1.0 LANGUAGES C CXX)

if (NOT APPLE)
    message(FATAL_ERROR "KickBassAssistant is currently configured for macOS only.")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_XCODE_GENERATE_SCHEME ON)

include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(juce)

juce_add_plugin(KickBassAssistant
    COMPANY_NAME "Balor Club"
    BUNDLE_ID "club.balor.kickbassassistant"
    PLUGIN_MANUFACTURER_CODE BlrC
    PLUGIN_CODE KBA1
    PRODUCT_NAME "KickBass Assistant"
    FORMATS VST3 Standalone
    COPY_PLUGIN_AFTER_BUILD TRUE
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES Fx Dynamics
)

juce_generate_juce_header(KickBassAssistant)

target_sources(KickBassAssistant
    PRIVATE
        src/PluginEditor.cpp
        src/PluginEditor.h
        src/PluginProcessor.cpp
        src/PluginProcessor.h
)

target_compile_definitions(KickBassAssistant
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(KickBassAssistant
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
