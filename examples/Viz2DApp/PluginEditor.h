#pragma once

#define PARAMETER_WINDOW

// Plugin
#include "UI/Visualization2DComponent.h"

// Parameters
#include "Parameters.h"
#ifdef PARAMETER_WINDOW
#include "ParameterWindow/ParameterWindow.h"
#endif

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

// Spectrex
#include <Spectrex/MiniProcessor.hpp>

// Stdlib
#include <memory>

class PluginAudioProcessor;

class PluginEditor
  : public juce::AudioProcessorEditor
  , public juce::Timer
{
  public:
    PluginEditor(PluginAudioProcessor& p);
    ~PluginEditor();

    // juce::Component
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

    // juce::Timer
    virtual void timerCallback() override;

  private:
    /// Called at the beginning of a GL frame before any drawing has been
    /// done, used as a single synchronization point to gather any data from the
    /// processors that can be used consistently throughout the entire frame.
    void beginGLDrawFrame();

  private:
    PluginAudioProcessor& m_processor;

    utility::WindowOpenGLContext m_openGLContext;

    Parameters m_parameters;

    std::unique_ptr<Visualization2DComponent> m_viz2DComponent;

#ifdef PARAMETER_WINDOW
    std::unique_ptr<ParameterWindow> m_parameterWindow;
#endif

    spectrex::MiniProcessor m_spectrexMiniProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
