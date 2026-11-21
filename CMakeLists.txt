cmake_minimum_required(VERSION 3.22)
project(DGPlugins VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------- DG-MasterBus
juce_add_plugin(DGMasterBus
    COMPANY_NAME "DGC"
    PLUGIN_MANUFACTURER_CODE Dgcl
    PLUGIN_CODE Dgmb
    FORMATS VST3
    PRODUCT_NAME "DG-MasterBus"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(DGMasterBus PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    DspUtils.h
    PassiveSummer.h
    Saturation.h
    Enhance.h
    StereoTools.h
    SSLCompressor.h)

target_compile_definitions(DGMasterBus PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DGMasterBus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ------------------------------------------------------------------- DG-Sender
juce_add_plugin(DGSender
    COMPANY_NAME "DGC"
    PLUGIN_MANUFACTURER_CODE Dgcl
    PLUGIN_CODE Dgsn
    FORMATS VST3
    PRODUCT_NAME "DG-Sender"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(DGSender PRIVATE
    SenderProcessor.cpp
    SenderProcessor.h
    DspUtils.h
    StereoTools.h
    Saturation.h)

target_compile_definitions(DGSender PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DGSender
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
