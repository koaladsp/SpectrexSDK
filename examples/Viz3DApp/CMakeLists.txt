# Shader compilation
add_subdirectory(shaders)

# Product
juce_add_plugin(Viz3DApp
        COMPANY_NAME "KoalaDSP"
        COPY_PLUGIN_AFTER_BUILD TRUE
        FORMATS Standalone VST3
        PRODUCT_NAME "Viz3DApp"
        PLUGIN_NAME "Spectrex 3D Example App"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE

        MICROPHONE_PERMISSION_ENABLED   TRUE
        MICROPHONE_PERMISSION_TEXT      "This app needs permissions to use your microphone."
        HARDENED_RUNTIME_OPTIONS        com.apple.security.device.audio-input
)
target_compile_definitions(Viz3DApp
        PUBLIC
        # JUCE
        JUCE_VST3_CAN_REPLACE_VST2=0
)
set_target_properties(Viz3DApp PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist")
    
# Sources
target_sources(Viz3DApp
        PRIVATE
        # Headers
        Renderer.h
        Rendering.h
        PluginEditor.h
        PluginProcessor.h
        Parameters.h
        Utility.h
        ParameterWindow/ParameterWindow.h
        ParameterWindow/ParameterDisplay.h

        # Sources
        Renderer.cpp
        Rendering.cpp
        PluginEditor.cpp
        PluginProcessor.cpp
        ParameterWindow/ParameterWindow.cpp
        ParameterWindow/ParameterDisplay.cpp
)

# Link dependencies
target_link_libraries(Viz3DApp
        PRIVATE
        # Spectrex dependencies
        Spectrex
        Viz3DShaders
        glm
        GSL
        glad
        $<$<CXX_COMPILER_ID:MSVC>:ipp>

        # JUCE
        $<BUILD_INTERFACE:juce::juce_audio_utils>
        $<BUILD_INTERFACE:juce::juce_audio_devices>
        $<BUILD_INTERFACE:juce::juce_opengl>
)
