#include "Visualizers2D.h"

#include "../PluginProcessor.h"
#include "../Utility.h"

// Spectrex
#include <Spectrex/Components/Spectrogram.hpp>

// JUCE
#include <juce_gui_basics/juce_gui_basics.h>

// Label margin (factor)
constexpr float LABEL_MARGIN = 0.01f;

void
Visualization2DComponent::resized()
{
    auto b = getLocalBounds();

    // Calculate larger of margin in either width or height for consistency
    const auto marginPx = juce::jmax(LABEL_MARGIN * b.getWidth(), LABEL_MARGIN * b.getHeight());

    // Frame bounds
    b = b.withTrimmedTop(marginPx).withTrimmedBottom(marginPx).withTrimmedLeft(marginPx).withTrimmedRight(marginPx);

    // Spectrogram
    const auto spectrogramBounds = b.withHeight(b.getHeight() * 0.5f - marginPx);
    m_spectrogramComponent->setBounds(spectrogramBounds);

    // Waveform
    const auto waveformBounds = b.withTop(b.getHeight() * 0.5f + marginPx).withHeight(b.getHeight() * 0.5f);
    m_waveformComponent->setBounds(waveformBounds);
}

void
Visualization2DComponent::paint(juce::Graphics& g)
{
    // Border color
    g.fillAll(juce::Colours::darkslategrey);
}

void
Visualization2DComponent::mouseMove(const juce::MouseEvent& event)
{
    Component::mouseMove(event);

    m_waveformComponent->setShouldDrawMouseTarget(true);
    m_spectrogramComponent->setShouldDrawMouseTarget(true);
}

void
Visualization2DComponent::mouseExit(const juce::MouseEvent& event)
{
    Component::mouseExit(event);

    m_waveformComponent->setShouldDrawMouseTarget(false);
    m_spectrogramComponent->setShouldDrawMouseTarget(false);
}

Visualization2DComponent::Visualization2DComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor)
  : m_pluginProcessor(processor)
  , m_processor(processor.getSpectrexMiniProcessor().getProcessor())
{

    // Instantiate components
    m_spectrogramComponent = std::make_unique<VisualizationComponent>(context, processor, VisualizationComponent::Type::Spectrogram);
    m_waveformComponent = std::make_unique<VisualizationComponent>(context, processor, VisualizationComponent::Type::Waveform);

    // Add as children
    addAndMakeVisible(*m_spectrogramComponent);
    addAndMakeVisible(*m_waveformComponent);

    // Add Mouse Listener
    m_spectrogramComponent->addMouseListener(this, true);
    m_waveformComponent->addMouseListener(this, true);
}

Visualization2DComponent::~Visualization2DComponent() {}
