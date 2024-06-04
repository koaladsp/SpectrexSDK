#pragma once

#define PARAMETER_WINDOW

// Parameters
#include "Parameters.h"
#ifdef PARAMETER_WINDOW
#include "ParameterWindow/ParameterWindow.h"
#endif

// Renderer
#include "Renderer.h"

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_opengl/juce_opengl.h>

// Spectrex
#include <Spectrex/MiniProcessor.hpp>

// Stdlib
#include <memory>

class PluginAudioProcessor;

class PluginEditor
  : public juce::AudioProcessorEditor
  , private juce::OpenGLRenderer
{
  public:
    PluginEditor(PluginAudioProcessor& p);
    ~PluginEditor();

  private:
    /// Called at the beginning of a GL frame before any drawing has been
    /// done, used as a single synchronization point to gather any data from the
    /// processors that can be used consistently throughout the entire frame.
    void beginGLDrawFrame();

    /* OpenGL */
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

  private:
    PluginAudioProcessor& m_processor;

    juce::OpenGLContext m_openGLContext;

    // 3D Visualizers
    std::unique_ptr<Renderer> m_renderer;
    Parameters m_parameters;

#ifdef PARAMETER_WINDOW
    std::unique_ptr<ParameterWindow> m_parameterWindow;
#endif

    spectrex::MiniProcessor m_spectrexMiniProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};