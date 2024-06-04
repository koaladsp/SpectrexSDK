# Product
juce_add_plugin(Viz2DApp
        COMPANY_NAME "KoalaDSP"
        COPY_PLUGIN_AFTER_BUILD TRUE
        FORMATS Standalone VST3
        PRODUCT_NAME "Viz2DApp"
        PLUGIN_NAME "Spectrex 2D Example App"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE

        MICROPHONE_PERMISSION_ENABLED   TRUE
        MICROPHONE_PERMISSION_TEXT      "This app needs permissions to use your microphone."
        HARDENED_RUNTIME_OPTIONS        com.apple.security.device.audio-input
)
target_compile_definitions(Viz2DApp
        PUBLIC
        # JUCE
        JUCE_VST3_CAN_REPLACE_VST2=0
)
set_target_properties(Viz2DApp PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist")

# Sources
target_sources(Viz2DApp
        PRIVATE
        # Headers
        PluginEditor.h
        PluginProcessor.h
        Parameters.h
        Utility.h
        WindowOpenGLContext.h
        ParameterWindow/ParameterWindow.h
        ParameterWindow/ParameterDisplay.h
        UI/Cursor.h
        UI/Visualization2DComponent.h
        UI/VisualizationComponent.h

        # Sources
        PluginEditor.cpp
        PluginProcessor.cpp
        WindowOpenGLContext.cpp
        ParameterWindow/ParameterWindow.cpp
        ParameterWindow/ParameterDisplay.cpp
        UI/Cursor.cpp
        UI/Visualization2DComponent.cpp
        UI/VisualizationComponent.cpp
)

# Link dependencies
target_link_libraries(Viz2DApp
        PRIVATE
        # Spectrex dependencies
        Spectrex
        glm
        GSL
        glad
        $<$<CXX_COMPILER_ID:MSVC>:ipp>

        # JUCE
        $<BUILD_INTERFACE:juce::juce_audio_utils>
        $<BUILD_INTERFACE:juce::juce_audio_devices>
        $<BUILD_INTERFACE:juce::juce_opengl>
)
