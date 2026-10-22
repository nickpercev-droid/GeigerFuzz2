cmake_minimum_required(VERSION 3.22)
project(GeigerFuzz VERSION 1.0.0)

# JUCE is fetched during configure. Internet access is required on the build machine
# unless JUCE is already vendored and this FetchContent block is adapted.
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GeigerFuzz
    COMPANY_NAME "NightSignal Audio"
    BUNDLE_ID "audio.nightsignal.geigerfuzz"
    PLUGIN_MANUFACTURER_CODE NgSa
    PLUGIN_CODE Gfuz
    FORMATS VST3 Standalone
    PRODUCT_NAME "Geiger Fuzz"
    DESCRIPTION "Digital wavetable fuzz and bit reduction effect"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(GeigerFuzz)

target_sources(GeigerFuzz PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_features(GeigerFuzz PRIVATE cxx_std_17)
target_compile_definitions(GeigerFuzz PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(GeigerFuzz PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
