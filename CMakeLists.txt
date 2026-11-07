cmake_minimum_required(VERSION 3.22)
project(BassRig VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Baixa o JUCE automaticamente (precisa de internet na primeira configuracao)
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

option(BASSRIG_COPY_AFTER_BUILD "Copia o plugin para a pasta VST3 do sistema apos compilar" ON)

juce_add_plugin(BassRig
    COMPANY_NAME "BassRig"
    PLUGIN_MANUFACTURER_CODE Bsrg
    PLUGIN_CODE Brig
    FORMATS VST3 Standalone
    PRODUCT_NAME "BassRig"
    VST3_CATEGORIES Fx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${BASSRIG_COPY_AFTER_BUILD})

target_sources(BassRig PRIVATE
    Source/DSP.h
    Source/Illustrations.h
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp)

target_compile_definitions(BassRig PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Strings em UTF-8 (acentos) no MSVC
target_compile_options(BassRig PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(BassRig
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
