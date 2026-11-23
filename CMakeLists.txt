cmake_minimum_required(VERSION 3.22)
project(GeigerCounterPro VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GeigerCounterPro
    COMPANY_NAME "Independent Audio Lab"
    BUNDLE_ID "audio.independentaudiolab.geigercounterpro"
    PLUGIN_MANUFACTURER_CODE InAL
    PLUGIN_CODE GcP1
    FORMATS VST3
    PRODUCT_NAME "Geiger Counter Pro (Behavioral)"
    DESCRIPTION "12-bit wavetable distortion inspired by the WMD Geiger Counter Pro signal path"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(GeigerCounterPro)
target_sources(GeigerCounterPro PRIVATE
    PluginProcessor.cpp PluginProcessor.h
    PluginEditor.cpp PluginEditor.h)
target_compile_features(GeigerCounterPro PRIVATE cxx_std_17)
target_compile_definitions(GeigerCounterPro PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(GeigerCounterPro PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
