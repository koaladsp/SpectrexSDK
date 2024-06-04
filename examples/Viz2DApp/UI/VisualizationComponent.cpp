#include "VisualizationComponent.h"

// Plugin
#include "../PluginProcessor.h"
#include "../Utility.h"
#include "Cursor.h"

// Spectrex
#include <Spectrex/Components/Spectrogram.hpp>
#include <Spectrex/Components/Waveform.hpp>
#include <Spectrex/Utility/Exception.hpp>

// Stdlib
#include <algorithm>
#include <limits>

// Colors
const auto WaveformDbMarker = juce::Colours::red;
const auto BorderColor = juce::Colours::white;
const auto MouseTarget = juce::Colour::fromString("#FF854C91");
const auto FrequencyTickColor = juce::Colour::fromString("#3Fffffff");
const auto BarTickColor = juce::Colour::fromString("#3Fffffff");
const auto MeterWhite = juce::Colour::fromString("#FFf0f0f0");
const auto MeterYellow = juce::Colour::fromString("#FFfff199");
const auto MeterRed = juce::Colour::fromString("#FFec394f");
const auto MeterHold = juce::Colour::fromString("#FF5a5636");

// Color ramps
// clang-format off
const std::vector<float> WhiteAlphaRamp = {
    // Start (0.0f)
    1.0f, 1.0f, 1.0f, 0.500000f,
    1.0f, 1.0f, 1.0f, 0.503906f,
    1.0f, 1.0f, 1.0f, 0.507812f,
    1.0f, 1.0f, 1.0f, 0.511719f,
    1.0f, 1.0f, 1.0f, 0.515625f,
    1.0f, 1.0f, 1.0f, 0.519531f,
    1.0f, 1.0f, 1.0f, 0.523438f,
    1.0f, 1.0f, 1.0f, 0.527344f,
    1.0f, 1.0f, 1.0f, 0.531250f,
    1.0f, 1.0f, 1.0f, 0.535156f,
    1.0f, 1.0f, 1.0f, 0.539062f,
    1.0f, 1.0f, 1.0f, 0.542969f,
    1.0f, 1.0f, 1.0f, 0.546875f,
    1.0f, 1.0f, 1.0f, 0.550781f,
    1.0f, 1.0f, 1.0f, 0.554688f,
    1.0f, 1.0f, 1.0f, 0.558594f,
    1.0f, 1.0f, 1.0f, 0.562500f,
    1.0f, 1.0f, 1.0f, 0.566406f,
    1.0f, 1.0f, 1.0f, 0.570312f,
    1.0f, 1.0f, 1.0f, 0.574219f,
    1.0f, 1.0f, 1.0f, 0.578125f,
    1.0f, 1.0f, 1.0f, 0.582031f,
    1.0f, 1.0f, 1.0f, 0.585938f,
    1.0f, 1.0f, 1.0f, 0.589844f,
    1.0f, 1.0f, 1.0f, 0.593750f,
    1.0f, 1.0f, 1.0f, 0.597656f,
    1.0f, 1.0f, 1.0f, 0.601562f,
    1.0f, 1.0f, 1.0f, 0.605469f,
    1.0f, 1.0f, 1.0f, 0.609375f,
    1.0f, 1.0f, 1.0f, 0.613281f,
    1.0f, 1.0f, 1.0f, 0.617188f,
    1.0f, 1.0f, 1.0f, 0.621094f,
    1.0f, 1.0f, 1.0f, 0.625000f,
    1.0f, 1.0f, 1.0f, 0.628906f,
    1.0f, 1.0f, 1.0f, 0.632812f,
    1.0f, 1.0f, 1.0f, 0.636719f,
    1.0f, 1.0f, 1.0f, 0.640625f,
    1.0f, 1.0f, 1.0f, 0.644531f,
    1.0f, 1.0f, 1.0f, 0.648438f,
    1.0f, 1.0f, 1.0f, 0.652344f,
    1.0f, 1.0f, 1.0f, 0.656250f,
    1.0f, 1.0f, 1.0f, 0.660156f,
    1.0f, 1.0f, 1.0f, 0.664062f,
    1.0f, 1.0f, 1.0f, 0.667969f,
    1.0f, 1.0f, 1.0f, 0.671875f,
    1.0f, 1.0f, 1.0f, 0.675781f,
    1.0f, 1.0f, 1.0f, 0.679688f,
    1.0f, 1.0f, 1.0f, 0.683594f,
    1.0f, 1.0f, 1.0f, 0.687500f,
    1.0f, 1.0f, 1.0f, 0.691406f,
    1.0f, 1.0f, 1.0f, 0.695312f,
    1.0f, 1.0f, 1.0f, 0.699219f,
    1.0f, 1.0f, 1.0f, 0.703125f,
    1.0f, 1.0f, 1.0f, 0.707031f,
    1.0f, 1.0f, 1.0f, 0.710938f,
    1.0f, 1.0f, 1.0f, 0.714844f,
    1.0f, 1.0f, 1.0f, 0.718750f,
    1.0f, 1.0f, 1.0f, 0.722656f,
    1.0f, 1.0f, 1.0f, 0.726562f,
    1.0f, 1.0f, 1.0f, 0.730469f,
    1.0f, 1.0f, 1.0f, 0.734375f,
    1.0f, 1.0f, 1.0f, 0.738281f,
    1.0f, 1.0f, 1.0f, 0.742188f,
    1.0f, 1.0f, 1.0f, 0.746094f,
    1.0f, 1.0f, 1.0f, 0.750000f,
    1.0f, 1.0f, 1.0f, 0.753906f,
    1.0f, 1.0f, 1.0f, 0.757812f,
    1.0f, 1.0f, 1.0f, 0.761719f,
    1.0f, 1.0f, 1.0f, 0.765625f,
    1.0f, 1.0f, 1.0f, 0.769531f,
    1.0f, 1.0f, 1.0f, 0.773438f,
    1.0f, 1.0f, 1.0f, 0.777344f,
    1.0f, 1.0f, 1.0f, 0.781250f,
    1.0f, 1.0f, 1.0f, 0.785156f,
    1.0f, 1.0f, 1.0f, 0.789062f,
    1.0f, 1.0f, 1.0f, 0.792969f,
    1.0f, 1.0f, 1.0f, 0.796875f,
    1.0f, 1.0f, 1.0f, 0.800781f,
    1.0f, 1.0f, 1.0f, 0.804688f,
    1.0f, 1.0f, 1.0f, 0.808594f,
    1.0f, 1.0f, 1.0f, 0.812500f,
    1.0f, 1.0f, 1.0f, 0.816406f,
    1.0f, 1.0f, 1.0f, 0.820312f,
    1.0f, 1.0f, 1.0f, 0.824219f,
    1.0f, 1.0f, 1.0f, 0.828125f,
    1.0f, 1.0f, 1.0f, 0.832031f,
    1.0f, 1.0f, 1.0f, 0.835938f,
    1.0f, 1.0f, 1.0f, 0.839844f,
    1.0f, 1.0f, 1.0f, 0.843750f,
    1.0f, 1.0f, 1.0f, 0.847656f,
    1.0f, 1.0f, 1.0f, 0.851562f,
    1.0f, 1.0f, 1.0f, 0.855469f,
    1.0f, 1.0f, 1.0f, 0.859375f,
    1.0f, 1.0f, 1.0f, 0.863281f,
    1.0f, 1.0f, 1.0f, 0.867188f,
    1.0f, 1.0f, 1.0f, 0.871094f,
    1.0f, 1.0f, 1.0f, 0.875000f,
    1.0f, 1.0f, 1.0f, 0.878906f,
    1.0f, 1.0f, 1.0f, 0.882812f,
    1.0f, 1.0f, 1.0f, 0.886719f,
    1.0f, 1.0f, 1.0f, 0.890625f,
    1.0f, 1.0f, 1.0f, 0.894531f,
    1.0f, 1.0f, 1.0f, 0.898438f,
    1.0f, 1.0f, 1.0f, 0.902344f,
    1.0f, 1.0f, 1.0f, 0.906250f,
    1.0f, 1.0f, 1.0f, 0.910156f,
    1.0f, 1.0f, 1.0f, 0.914062f,
    1.0f, 1.0f, 1.0f, 0.917969f,
    1.0f, 1.0f, 1.0f, 0.921875f,
    1.0f, 1.0f, 1.0f, 0.925781f,
    1.0f, 1.0f, 1.0f, 0.929688f,
    1.0f, 1.0f, 1.0f, 0.933594f,
    1.0f, 1.0f, 1.0f, 0.937500f,
    1.0f, 1.0f, 1.0f, 0.941406f,
    1.0f, 1.0f, 1.0f, 0.945312f,
    1.0f, 1.0f, 1.0f, 0.949219f,
    1.0f, 1.0f, 1.0f, 0.953125f,
    1.0f, 1.0f, 1.0f, 0.957031f,
    1.0f, 1.0f, 1.0f, 0.960938f,
    1.0f, 1.0f, 1.0f, 0.964844f,
    1.0f, 1.0f, 1.0f, 0.968750f,
    1.0f, 1.0f, 1.0f, 0.972656f,
    1.0f, 1.0f, 1.0f, 0.976562f,
    1.0f, 1.0f, 1.0f, 0.980469f,
    1.0f, 1.0f, 1.0f, 0.984375f,
    1.0f, 1.0f, 1.0f, 0.988281f,
    1.0f, 1.0f, 1.0f, 0.992188f,
    1.0f, 1.0f, 1.0f, 0.996094f,
    1.0f, 1.0f, 1.0f, 1.000000f
    // End (1.0f)
};
// clang-format on

void
VisualizationComponent::paint(juce::Graphics& g)
{
    // Skip paint when component is not ready yet
    if (!m_component) {
        return;
    }

    if (getLocalBounds().getHeight() == 0 || getLocalBounds().getWidth() == 0) {
        return;
    }

    const bool isRotated = false; // @param

    if (m_type == Type::Spectrogram) {
        // Spectrogram: frequency ticks

        auto b = getLocalBounds();

        const auto maxFrq = std::round(m_component->getMaxFrequency());
        const auto minFrq = std::round(m_component->getMinFrequency());

        juce::Path linePath;
        juce::PathStrokeType stroke{ 1.0f };

        auto viewBox = getViewBox();

        // Normalized bounds: y-axis in rotated mode, x-axis otherwise
        const auto minNorm = isRotated ? std::get<0>(viewBox) : std::get<2>(viewBox);
        const auto maxNorm = isRotated ? std::get<1>(viewBox) : std::get<3>(viewBox);

        // Pixel bounds: 0->width in rotated mode, height->0 otherwise
        const auto minBound = isRotated ? 0.0f : (float)getHeight();
        const auto maxBound = isRotated ? (float)getWidth() : 0.0f;

        // Draw frequency ticks when not in bar mode
        for (const auto freq : k_FreqsToMap) {

            const auto normVal = freqToNorm(freq, minFrq, maxFrq);

            if (normVal < minNorm || normVal > maxNorm) {
                continue;
            }

            if (!juce::isPositiveAndNotGreaterThan(normVal, 1.0f)) {
                continue;
            }

            const auto lineStart = juce::jmap(normVal, minNorm, maxNorm, minBound, maxBound);

            g.setColour(FrequencyTickColor);
            linePath.clear();
            linePath.startNewSubPath(isRotated ? lineStart : b.getX(), isRotated ? 0.0f : lineStart);
            linePath.lineTo(isRotated ? lineStart : b.getRight(), isRotated ? b.getHeight() : lineStart);
            g.strokePath(linePath, stroke);
        }
    }

    if (m_type == Type::Spectrogram || m_type == Type::Waveform) {
        const auto b = getLocalBounds();

        // Spectrogram: Bar Strokes
        {
            // Get most recently drawn ppq
            auto lastPpq = getPpqLastDrawn();
            const bool isOverlap = false;                       // @param
            const bool isSynced = false;                        // @param
            const auto timeFactor = 1.0f;                       // @param
            const auto numBars = m_processor.getTimeQuantity(); // @param
            const auto numerator = 1;                           // @param
            const auto timeUnit = m_processor.getTimeUnit();    // @param

            const auto viewBox = getViewBox();

            const auto minXNorm = std::get<0>(viewBox);
            const auto maxXNorm = std::get<1>(viewBox);
            const auto minYNorm = std::get<2>(viewBox);
            const auto maxYNorm = std::get<3>(viewBox);

            const auto currentBar = isSynced ? lastPpq / numerator : 0.0f;

            auto minBar = isSynced ? 1.0f + currentBar - numBars : 0.0f;

            if (isOverlap && isSynced) {
                minBar = std::floor(currentBar / numBars) * numBars + 1.0f;
            }

            const auto maxBar = minBar + numBars;

            int increment = isSynced ? 1 : timeFactor * 1000.0f;

            // If we shown less than a full bar on the screen, we skip this
            // drawing.
            juce::Path barPath;
            juce::Path beatPath;
            if ((int)numBars != 0) {
                for (int i = std::floor(minBar); i <= (int)numBars + std::floor(minBar) + 1; i += increment) {
                    const auto barNorm = barToNormVal(i, minBar, maxBar);

                    const auto barPos = juce::jmap(barNorm,
                                                   isRotated ? minYNorm : minXNorm,
                                                   isRotated ? maxYNorm : maxXNorm,
                                                   isRotated ? (float)b.getHeight() : 0.0f,
                                                   isRotated ? 0.0f : (float)b.getRight());

                    const juce::PathStrokeType barStroke{
                        k_barStrokeWidth,
                    };

                    g.setColour(BarTickColor);

                    // Bar Strokes
                    if (barNorm >= (isRotated ? minYNorm : minXNorm) && barNorm <= (isRotated ? maxYNorm : maxXNorm)) {
                        barPath.clear();
                        barPath.startNewSubPath(isRotated ? 0.0f : barPos, isRotated ? barPos : 0.0f);
                        barPath.lineTo(isRotated ? (float)getRight() : barPos, isRotated ? barPos : (float)getBottom());
                        g.strokePath(barPath, barStroke);
                    }

                    // Beat Strokes
                    if (timeUnit == spectrex::KProcessor::TimeUnit::Bars && numBars <= 8) {
                        const auto timeSigNum = 1; // @param
                        auto scaledNumBars = numBars * (maxXNorm - minXNorm);
                        const auto beatWidth = (barToNormVal(1.0f, minBar, maxBar) - barToNormVal(2.0, minBar, maxBar)) *
                                               (isRotated ? b.getHeight() : b.getWidth()) / timeSigNum;
                        float beatPos = barPos - beatWidth;
                        const juce::PathStrokeType beatStroke{ k_beatStrokeWidth,
                                                               juce::PathStrokeType::JointStyle::beveled,
                                                               juce::PathStrokeType::EndCapStyle::butt };

                        for (int beat = 1; beat < timeSigNum; ++beat) {
                            if (beatPos < 0.0f || beatPos > (isRotated ? b.getHeight() : b.getWidth())) {
                                beatPos -= beatWidth;
                                continue;
                            }

                            beatPath.clear();

                            beatPath.startNewSubPath(isRotated ? 0.0f : beatPos, isRotated ? beatPos : 0.0f);
                            beatPath.lineTo(isRotated ? b.getRight() : beatPos, isRotated ? beatPos : b.getBottom());
                            g.strokePath(beatPath, beatStroke);
                            beatPos -= beatWidth;
                        }
                    }
                }
            }
        }
    }

    if (m_type == Type::Waveform || m_type == Type::Spectrogram) {
        // Mouse Location
        if (m_shouldDrawMouseTargetLines) {

            auto mousePos = getMouseXYRelative();

            const juce::PathStrokeType mouseStroke{ k_beatStrokeWidth,
                                                    juce::PathStrokeType::JointStyle::beveled,
                                                    juce::PathStrokeType::EndCapStyle::butt };

            juce::Path horizPath;
            horizPath.startNewSubPath(0, mousePos.getY());
            horizPath.lineTo(getWidth(), mousePos.getY());

            // Splitting vertical path across the horizontal one to not overlap
            // the fill where they cross.
            juce::Path vertPathTop;
            vertPathTop.startNewSubPath(mousePos.getX(), 0);
            vertPathTop.lineTo(mousePos.getX(), mousePos.getY() - k_beatStrokeWidth / 2.0);

            juce::Path vertPathBot;
            vertPathBot.startNewSubPath(mousePos.getX(), mousePos.getY() + k_beatStrokeWidth);
            vertPathBot.lineTo(mousePos.getX(), getHeight());

            g.setColour(MouseTarget);
            g.strokePath(horizPath, mouseStroke);
            g.strokePath(vertPathTop, mouseStroke);
            g.strokePath(vertPathBot, mouseStroke);

            // Show info text next to cursor
            const auto infoText = getMouseTargetText();
            const auto infoTextOffsetX = 20; // px
            const auto infoTextOffsetY = 20; // px
            g.drawSingleLineText(infoText, mousePos.getX() + infoTextOffsetX, mousePos.getY() + infoTextOffsetY);
        }
    }

    if (m_type == Type::Waveform) {
        const auto headroom = m_processor.getWaveformHeadroom();
        if (headroom > std::numeric_limits<float>::epsilon()) {
            // Determine whether waveform is drawn as mono or stereo
            const auto stereoWaveform = true; // @param

            // Headroom [0, 100%] scales the amplitude back by at most half.
            //
            // For single waveform rendering:
            //
            // Considering the waveform is mirrored from the middle,
            // half amplitude corresponds to 0.25 of the waveform height.
            //
            // For multiple waveform rendering (stereo) the above behaviour is
            // duplicated across the vertical bounds.
            //
            const auto headroomScale = 1.0f - std::clamp((headroom * 0.0025f), 0.0f, 0.25f);

            const auto b = getLocalBounds().toFloat();

            const juce::PathStrokeType stroke{ k_dbMarkerStrokeWidth };

            g.setColour(WaveformDbMarker);

            if (isRotated) {
                const auto numWaveforms = stereoWaveform ? 2 : 1;
                const auto waveformWidth = getWidth() / numWaveforms;
                const auto x = waveformWidth * headroomScale;

                juce::Path path;
                for (int i = 0; i < numWaveforms; ++i) {
                    const auto xBegin = waveformWidth * i;
                    const auto xEnd = waveformWidth * (i + 1);

                    path.clear();
                    path.startNewSubPath(xBegin + x, 0);
                    path.lineTo(xBegin + x, b.getHeight());
                    g.strokePath(path, stroke);

                    path.clear();
                    path.startNewSubPath(xEnd - x, 0);
                    path.lineTo(xEnd - x, b.getHeight());
                    g.strokePath(path, stroke);
                }
            } else {
                const auto numWaveforms = stereoWaveform ? 2 : 1;
                const auto waveformHeight = getHeight() / numWaveforms;
                const auto y = waveformHeight * headroomScale;

                juce::Path path;
                for (int i = 0; i < numWaveforms; ++i) {
                    const auto yBegin = waveformHeight * i;
                    const auto yEnd = waveformHeight * (i + 1);

                    path.clear();
                    path.startNewSubPath(0, yBegin + y);
                    path.lineTo(b.getWidth(), yBegin + y);
                    g.strokePath(path, stroke);

                    path.clear();
                    path.startNewSubPath(0, yEnd - y);
                    path.lineTo(b.getWidth(), yEnd - y);
                    g.strokePath(path, stroke);
                }
            }
        }
    }
}

void
VisualizationComponent::updateClippingBounds()
{
    // Recalculate clipping boundaries, essential to get properly DPI scaled
    // render target
    m_clippingBounds = m_openGLContext.updateViewportSize(this);
}

void
VisualizationComponent::resized()
{
    updateClippingBounds();
}

void
VisualizationComponent::moved()
{
    resized();
}

void
VisualizationComponent::parentHierarchyChanged()
{
    resized();
}

void
VisualizationComponent::newOpenGLContextCreated()
{
    try {
        // Instantiate the desired component and defaults
        switch (m_type) {
            case Type::Spectrogram: {
                m_component = std::make_unique<spectrex::KSpectrogramComponent>(m_processor, m_openGLContext.getVisualizationContext());
                // Sets a default color ramp for the spectrogram (heatmap)
                ((spectrex::KSpectrogramComponent*)(m_component.get()))->setColorRamp({ 0.0f, 0.0f, 0.0f, 0.0f, //
                                                                                        0.0f, 0.0f, 0.0f, 1.0f, //
                                                                                        0.0f, 0.0f, 1.0f, 1.0f, //
                                                                                        0.0f, 1.0f, 1.0f, 1.0f, //
                                                                                        0.0f, 1.0f, 0.0f, 1.0f, //
                                                                                        1.0f, 1.0f, 0.0f, 1.0f, //
                                                                                        1.0f, 0.0f, 0.0f, 1.0f });
            } break;
            case Type::Waveform: {
                m_component = std::make_unique<spectrex::KWaveformComponent>(m_processor, m_openGLContext.getVisualizationContext());
                // Sets a default color ramp for the waveform (single color)
                // A color ramp isn't really required for a waveform, but you can of course add one to intensify the waveform amplitudes
                ((spectrex::KWaveformComponent*)(m_component.get()))->setColorRamp(WhiteAlphaRamp);
            } break;
            default:
                jassertfalse; // Unknown component type
        }

        initialUpdate();

    } catch (const spectrex::Exception& e) {
        DBG("Exception during initialization: " << e.getReason());
        jassertfalse;

        m_component = nullptr;
    }
}

void
VisualizationComponent::renderOpenGL()
{
    // THREAD: GL
    // NOTE: Do not perform any juce:: UI calls (e.g. juce::Component) here,
    // it leads to undefined behaviour and its scaling is inconsistent with
    // GL and will cause issues.

    // Get properly GL scaled (DPI corrected) clipping bounds
    auto clippingBounds = m_clippingBounds;

    const auto width = clippingBounds.getWidth();
    const auto height = clippingBounds.getHeight();

    // Do not draw or allocate anything if either dimension is zero
    if (width == 0 || height == 0) {
        return;
    }

    // Apply scissor test to clear area
    juce::OpenGLHelpers::enableScissorTest(clippingBounds);
    juce::OpenGLHelpers::clear(juce::Colours::black);

    if (m_component == nullptr) {
        return;
    }

    m_component->setWidth(width);
    m_component->setHeight(height);
    m_component->setX(clippingBounds.getX());
    m_component->setY(clippingBounds.getY());

    m_component->draw();
}

void
VisualizationComponent::openGLContextClosing()
{
}

void
VisualizationComponent::mouseMove(const juce::MouseEvent& event)
{
    if (m_component != nullptr) {
        auto position = event.getPosition() * m_openGLContext.getViewportScale();

        m_component->onMouseMove(position.x, position.y);
    }
}

void
VisualizationComponent::mouseDown(const juce::MouseEvent& event)
{
    if (m_component != nullptr) {
        setMouseCursor(juce::MouseCursor::NormalCursor);

        auto position = event.getPosition() * m_openGLContext.getViewportScale();

        if (event.getNumberOfClicks() == 1) {
            m_component->onMouseStart(position.x, position.y);
        } else if (event.getNumberOfClicks() == 2) {
            m_component->onMouseDoubleClick(position.x, position.y);
        }
    }
}

void
VisualizationComponent::mouseUp(const juce::MouseEvent& event)
{
    if (m_component != nullptr) {
        setMouseCursor(juce::MouseCursor::NormalCursor);

        auto position = event.getPosition() * m_openGLContext.getViewportScale();

        m_component->onMouseEnd(position.x, position.y);
    }
}

void
VisualizationComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (m_component != nullptr) {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);

        auto position = event.getPosition() * m_openGLContext.getViewportScale();

        m_component->onMouseDrag(position.x, position.y);

        if (event.mods.isShiftDown()) {
            m_component->onMouseShiftDrag(position.x, position.y);
        }
    }
}

void
VisualizationComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (m_component != nullptr) {
        const auto position = event.getPosition() * m_openGLContext.getViewportScale();

        m_component->onMouseWheel(position.x, position.y, wheel.deltaY);
    }
}

void
VisualizationComponent::mouseMagnify(const juce::MouseEvent& event, float scaleFactor)
{
}

void
VisualizationComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (m_component != nullptr) {

        const auto position = event.getPosition() * m_openGLContext.getViewportScale();

        if (event.mods.isShiftDown()) {
            m_component->onMouseShiftDoubleClick(position.x, position.y);
        }
    }
}

void
VisualizationComponent::buttonClicked(juce::Button* button)
{
}

void
VisualizationComponent::buttonStateChanged(juce::Button* button)
{
}

auto
VisualizationComponent::getPpqLastDrawn() const noexcept -> float
{
    if (!m_component) {
        return 0.0f;
    }

    return m_component->getPositionLastDrawn();
}

auto
VisualizationComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) -> bool
{
    (void)key;
    (void)originatingComponent;

    return false;
}

void
VisualizationComponent::timerCallback()
{
    const auto& cursor = Cursor::get();

    // Detect display scale or DPI changes using timer because there seems to be no
    // reliable JUCE callback for this
    {
        const auto currentTime = juce::Time::getApproximateMillisecondCounter();
        if (m_lastClipUpdate - currentTime >= 500) {

            updateClippingBounds();
            m_lastClipUpdate = currentTime;
        }
    }
}

void
VisualizationComponent::setShouldDrawMouseTarget(bool shouldDrawTarget) noexcept
{
    m_shouldDrawMouseTargetLines = shouldDrawTarget;
}

auto
VisualizationComponent::getViewBox() const noexcept -> std::tuple<float, float, float, float>
{
    if (!m_component) {
        return {};
    }

    return m_component->getViewBox();
}

auto
VisualizationComponent::getMouseTargetText() const noexcept -> const juce::String
{
    if (!m_component || !isMouseOver(true))
        return juce::String();

    const auto mousePos = getMouseXYRelative();
    const auto normX = mousePos.getX() / (float)getWidth();
    const auto normY = mousePos.getY() / (float)getHeight();

    return m_component->getInfoTextForNormalizedPosition(normX, normY);
}

auto
VisualizationComponent::getType() const noexcept -> const Type
{
    return m_type;
}

VisualizationComponent::VisualizationComponent(utility::WindowOpenGLContext& context,
                                               PluginAudioProcessor& processor,
                                               Parameters& parameters,
                                               Type type)
  : m_type(type)
  , m_openGLContext(context)
  , m_pluginProcessor(processor)
  , m_processor(processor.getSpectrexMiniProcessor().getProcessor())
  , m_parameters(parameters)
  , m_shouldDrawMouseTargetLines(false)
{
    setOpaque(true);

    // Add renderer
    m_openGLContext.addRenderingTarget(this);

    // Add key listener
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    setVisible(true);

    m_parameters.addListener(this);

    // 60 Hz timer
    startTimer(1000 / 60);
}

VisualizationComponent::~VisualizationComponent()
{
    m_parameters.removeListener(this);
    m_openGLContext.removeRenderingTarget(this);
}

void
VisualizationComponent::initialUpdate()
{
    // Sync all parameters
    m_parameters.all();
}

void
VisualizationComponent::parameterChanged(const Parameters& parameters, const std::string& name) noexcept
{
    // We don't create components until the context is created so we need to check
    if (!m_component) {
        return;
    }

    // Handle parameters
    if (name == "pause") {
        m_processor.setFrozen(parameters.pause);
    } else if (name == "min_frequency") {
        m_component->setMinFrequency(parameters.min_frequency);
    } else if (name == "max_frequency") {
        m_component->setMaxFrequency(parameters.max_frequency);
    } else if (name == "min_db") {
        m_component->setMinDb(parameters.min_db);
    } else if (name == "max_db") {
        m_component->setMaxDb(parameters.max_db);
    } else if (name == "window") {
        m_processor.setParameter<spectrex::Window>(spectrex::ProcessorParameters::Key::Window, parameters.window);
    } else if (name == "stft_overlay") {
        m_processor.setParameter<float>(spectrex::ProcessorParameters::Key::StftOverlap, parameters.stft_overlap);
    } else if (name == "time_multiplier") {
        m_processor.setParameter<float>(spectrex::ProcessorParameters::Key::TimeMultiplier, parameters.time_multiplier);
    } else if (name == "mix") {
        m_processor.setParameter<spectrex::MixMode>(spectrex::ProcessorParameters::Key::MixMode, parameters.mix);
    } else if (name == "ft_size") {
        m_processor.setParameter<spectrex::FtSize>(spectrex::ProcessorParameters::Key::FtSize, parameters.ft_size);
    }
}
