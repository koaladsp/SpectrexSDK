#pragma once

// Plugin
#include "../Parameters.h"
#include "../WindowOpenGLContext.h"
#include "VisualizationComponent.h"

// Spectrex
#include <Spectrex/Components/Component.hpp>
#include <Spectrex/Processing/Processor.hpp>
#include <Spectrex/Utility/Utility.hpp>

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Stdlib
#include <memory>
#include <thread>

class Processor;
class PluginAudioProcessor;

class Visualization2DComponent : public juce::Component
{
  public:
    void resized() override;
    void paint(juce::Graphics& g) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

    Visualization2DComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor, Parameters& parameters);
    virtual ~Visualization2DComponent() override;

  private:
    PluginAudioProcessor& m_pluginProcessor;
    spectrex::KProcessor& m_processor;
    Parameters& m_parameters;

    std::unique_ptr<VisualizationComponent> m_waveformComponent;
    std::unique_ptr<VisualizationComponent> m_spectrogramComponent;
};
