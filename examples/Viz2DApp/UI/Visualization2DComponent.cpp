#include "Visualization2DComponent.h"

// Plugin
#include "../PluginProcessor.h"
#include "../Utility.h"

// Spectrex
#include <Spectrex/Components/Spectrogram.hpp>

// JUCE
#include <juce_gui_basics/juce_gui_basics.h>

// Label margin (factor)
constexpr float LABEL_MARGIN = 0.035f;

// Color
const auto BorderColor = juce::Colour::fromString("#FF1B0F1E");
const auto ComponentBorderColor = juce::Colour::fromString("#FF361E3C");
const auto LabelColor = juce::Colour::fromString("#7Fffffff");

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
    auto b = getLocalBounds();

    // Border color
    g.fillAll(BorderColor);

    // Calculate larger of margin in either width or height for consistency
    const auto marginPx = juce::jmax(LABEL_MARGIN * b.getWidth(), LABEL_MARGIN * b.getHeight());

    // Component borders
    const auto componentBorderWidth = 1.0f;
    g.setColour(ComponentBorderColor);
    g.drawRect(m_spectrogramComponent->getBounds().expanded(componentBorderWidth), componentBorderWidth);
    g.drawRect(m_waveformComponent->getBounds().expanded(componentBorderWidth), componentBorderWidth);

    // Labels
    g.setColour(LabelColor);
    {
        const bool frequencyLabelsVisible = true;
        const bool timeLabelsVisible = true;

        const auto spectrogram = m_spectrogramComponent->getSpectrexComponent();
        const auto waveform = m_waveformComponent->getSpectrexComponent();
        jassert(spectrogram != nullptr);
        jassert(waveform != nullptr);

        // Frequency Hz Labels
        if (frequencyLabelsVisible) {
            const auto minFreq = std::round(spectrogram->getMinFrequency());
            const auto maxFreq = std::round(spectrogram->getMaxFrequency());

            // Calculate all edges
            const auto gramLeft = m_spectrogramComponent->getX();
            const auto gramRight = m_spectrogramComponent->getRight();
            const auto gramTop = m_spectrogramComponent->getY();
            const auto gramBottom = m_spectrogramComponent->getBottom();

            // Margin
            const float actualMarginSize = marginPx;
            jassert(actualMarginSize > 0);
            const float minY = gramTop;
            const float maxY = gramBottom;
            const float minX = gramRight + actualMarginSize * 0.15f;

            // Add extra padding
            const float paddedMarginSize = actualMarginSize * 0.85f;

            // Init last bounds to out of range to not interfere at the
            // beginning
            juce::Rectangle<float> lastTextBounds = { 0.0, maxY + 100.0f, 0.0f, 0.0f };

            // Require minimum size
            if (maxY - minY > 10) {
                for (const auto& freq : m_spectrogramComponent->k_FreqsToMap) {

                    if (freq < minFreq || freq > maxFreq) {
                        continue;
                    }

                    auto viewBox = spectrogram->getViewBox();

                    // Normalized bounds: y-axis in normal mode
                    const auto minNorm = std::get<2>(viewBox);
                    const auto maxNorm = std::get<3>(viewBox);

                    const auto normFreq = freqToNorm(freq, minFreq, maxFreq);

                    if (normFreq < minNorm || normFreq > maxNorm) {
                        continue;
                    }

                    juce::String freqText;
                    if (freq < 1000.0f) {
                        freqText = juce::String((int)freq);
                    } else {
                        freqText = juce::String((int)(freq / 1000.0f)) + "k";
                    }

                    const auto yPos = juce::jmap(normFreq, minNorm, maxNorm, maxY, minY);
                    juce::Rectangle<float> textBounds{ minX, yPos, paddedMarginSize, 0.0f };
                    textBounds = textBounds.withSizeKeepingCentre(paddedMarginSize, paddedMarginSize);

                    if (textBounds.getY() < minY) {
                        textBounds.setY(minY);
                    } else if (textBounds.getBottom() > maxY) {
                        textBounds.setBottom(maxY);
                    }

                    // If this text would overlap the last, skip to the next
                    // one
                    if (textBounds.getBottom() > lastTextBounds.getY() + paddedMarginSize * 0.5f)
                        continue;

                    lastTextBounds = textBounds;
                    g.drawFittedText(freqText, textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
                }
            }
        }

        // Time quantity labels
        if (timeLabelsVisible) {
            // Calculate all edges
            const auto gramLeft = m_spectrogramComponent->getX();
            const auto gramRight = m_spectrogramComponent->getRight();
            const auto gramTop = m_spectrogramComponent->getY();
            const auto gramBottom = m_spectrogramComponent->getBottom();
            const auto waveLeft = m_waveformComponent->getX();
            const auto waveRight = m_waveformComponent->getRight();
            const auto waveTop = m_waveformComponent->getY();
            const auto waveBottom = m_waveformComponent->getBottom();

            // Get most recently drawn ppq
            auto lastPpq = m_spectrogramComponent->getPpqLastDrawn();
            const bool isSynced = m_processor.getParameter<bool>(spectrex::ProcessorParameters::Key::PlayHeadSynced);
            const auto numerator = m_processor.getParameter<int>(spectrex::ProcessorParameters::Key::TimeSignatureNumerator);
            const auto timeFactor = m_processor.getParameter<float>(spectrex::ProcessorParameters::Key::TimeFactor);
            const auto numBars = isSynced ? m_processor.getTimeQuantity() : m_processor.getTimeQuantity() * 1000.0f;

            const auto currentBar = isSynced ? lastPpq / numerator : 0.0f;

            auto minBar = isSynced ? 1.0f + currentBar - numBars : 0.0f;
            const bool isOverlap = m_processor.getParameter<bool>(spectrex::ProcessorParameters::Key::Override);
            if (isOverlap && isSynced) {
                minBar = std::floor(currentBar / numBars) * numBars + 1.0f;
            }
            auto maxBar = minBar + numBars;
            int increment = isSynced ? 1 : timeFactor * 1000.0f;

            auto viewBox = m_spectrogramComponent->getViewBox();

            // Normalized bounds: y-axis in rotated mode, x-axis otherwise
            auto aNorm = std::get<0>(viewBox);
            auto bNorm = std::get<1>(viewBox);

            // Pixel bounds: top->bottom in rotated mode, left->right otherwise
            auto aBound = (float)gramLeft;
            auto bBound = (float)gramRight;

            // Require minimum size
            if (abs(bBound - aBound) > 10) {
                // Calculate margin size (margin between spectrum and goniometer)
                const float actualMarginSize = waveTop - gramBottom;
                jassert(actualMarginSize > 0);

                float textWidth = 0.0f;
                float textHeight = actualMarginSize;

                const auto lowest = std::min(aBound, bBound);
                const auto highest = std::max(aBound, bBound);

                for (int i = (int)minBar; i <= (int)maxBar; i += increment) {

                    // Normalized bar position
                    auto barNorm = barToNormVal(i, minBar, maxBar);

                    // If sync mode is disabled, time is shown, so let the labels
                    // run from the other way around
                    if (!isSynced)
                        barNorm = 1.0f - barNorm;

                    if (barNorm < aNorm || barNorm > bNorm || aNorm == bNorm) {
                        continue;
                    }

                    juce::String barText{ i };
                    juce::Rectangle<float> textBounds;

                    const auto barPos = juce::jmap(barNorm, aNorm, bNorm, aBound, bBound);

                    // Width of the text is bounded simply by the bar text
                    // Height of the text is bounded by the margin size (see
                    // above)
                    textWidth = g.getCurrentFont().getStringWidth(barText);

                    textBounds = { barPos, (float)gramBottom, 0.0f, textHeight };
                    textBounds = textBounds.withSizeKeepingCentre(textWidth * 1.1f, textHeight);

                    // Clamp bounds
                    if (textBounds.getRight() > highest) {
                        textBounds.setX(highest - textBounds.getWidth());
                    } else if (textBounds.getX() < lowest) {
                        textBounds.setX(lowest);
                    }

                    // Skip overlapping texts
                    juce::Rectangle<float> lastTxtBounds{ -100.0f, -100.0f, 0.0f, 0.0f };
                    if (lastTxtBounds.intersects(textBounds))
                        continue;
                    lastTxtBounds = textBounds;

                    g.drawFittedText(barText, textBounds.toNearestInt(), juce::Justification::centred, 1);
                }
            }
        }
    }
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

Visualization2DComponent::Visualization2DComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor, Parameters& parameters)
  : m_pluginProcessor(processor)
  , m_processor(processor.getSpectrexMiniProcessor().getProcessor())
    , m_parameters(parameters)
{

    // Instantiate components
    m_spectrogramComponent = std::make_unique<VisualizationComponent>(context, processor, parameters, VisualizationComponent::Type::Spectrogram);
    m_waveformComponent = std::make_unique<VisualizationComponent>(context, processor, parameters, VisualizationComponent::Type::Waveform);

    // Add as children
    addAndMakeVisible(*m_spectrogramComponent);
    addAndMakeVisible(*m_waveformComponent);

    // Add Mouse Listener
    m_spectrogramComponent->addMouseListener(this, true);
    m_waveformComponent->addMouseListener(this, true);
}

Visualization2DComponent::~Visualization2DComponent() {}
