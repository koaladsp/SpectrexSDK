// Plugin
#include "VisualizationComponent.h"
#include "Cursor.h"

#include "../PluginProcessor.h"
#include "../Utility.h"

/*
#include "VISION4X/Constants.h"
#include "VISION4X/PluginProcessor.h"

#include "VISION4X_Core/Exception.h"

#include "VISION4X_Core/Components/CorrelationMeter.h"
#include "VISION4X_Core/Components/Goniometer.h"
#include "VISION4X_Core/Components/Metering.h"
#include "VISION4X_Core/Components/Spectrogram.h"
#include "VISION4X_Core/Components/Spectrum.h"
#include "VISION4X_Core/Components/Waveform.h"

#include "VISION4X_Core/Rendering/Cursor.h"

#include "VISION4X_Core/Utility/ColorRamps.h"
#include "VISION4X_Core/Utility/Utility.h"

#include "Assets.h"
*/

// Spectrex
#include <Spectrex/Components/CorrelationMeter.hpp>
#include <Spectrex/Components/Goniometer.hpp>
#include <Spectrex/Components/Metering.hpp>
#include <Spectrex/Components/Spectrogram.hpp>
#include <Spectrex/Components/Spectrum.hpp>
#include <Spectrex/Components/Waveform.hpp>
#include <Spectrex/Utility/Exception.hpp>

// Stdlib
#include <algorithm>
#include <limits>

// Constants
const auto BorderColor = juce::Colours::white;
const auto MouseTarget = juce::Colours::white;
const auto WaveformDbMarker = juce::Colours::red;
const auto LineColor = juce::Colours::white;
const float DbLabelIncrement = 10.0f;
const juce::Array<int> MeterDbTextVals{ -80, -60, -40, -20, 0 };
const juce::Array<int> FreqsToMap{ 30, 40, 50, 60, 80, 100, 200, 300, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000, 20000 };

const juce::Colour MeterWhite = juce::Colour::fromString("#FFf0f0f0");
const juce::Colour MeterYellow = juce::Colour::fromString("#FFfff199");
const juce::Colour MeterRed = juce::Colour::fromString("#FFec394f");
const juce::Colour MeterHold = juce::Colour::fromString("#FF5a5636");

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
        auto lineColor = BorderColor;

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
        for (const auto freq : FreqsToMap) {

            const auto normVal = freqToNorm(freq, minFrq, maxFrq);

            if (normVal < minNorm || normVal > maxNorm) {
                continue;
            }

            if (!juce::isPositiveAndNotGreaterThan(normVal, 1.0f)) {
                continue;
            }

            const auto lineStart = juce::jmap(normVal, minNorm, maxNorm, minBound, maxBound);

            // const auto lineY = b.getHeight() * (1.0f - normY);
            g.setColour(BorderColor.withAlpha(0.3f));
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

                    g.setColour(BorderColor.withAlpha(0.7f));

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

    if (m_type == Type::Waveform || m_type == Type::Spectrogram || m_type == Type::Spectrum) {
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
        }
    }

    // DB Marker Lines - Spectrum
    if (m_type == Type::Spectrum) {
        const auto b = getLocalBounds().toFloat();

        const auto minDb = m_component->getMinDb();

        const auto maxDb = m_component->getMaxDb();

        const auto dbRangeStart = -80.0f; // @parameter
        const auto dbRangeEnd = 0.0f;     // @parameter
        const auto dbScale = 0.0f;        // @parameter

        const juce::PathStrokeType stroke{ k_beatStrokeWidth };

        g.setColour(BorderColor.withAlpha(0.4f));

        juce::Path path;

        for (float val = dbRangeEnd; val >= dbRangeStart; val -= DbLabelIncrement) {

            if (val < minDb || val > maxDb) {
                continue;
            }

            const auto dbNorm = juce::jmap(val, minDb, maxDb, isRotated ? 1.0f : 0.0f, isRotated ? 0.0f : 1.0f);
            const auto dbNormScaled = dbRescale(dbNorm, dbScale);
            const auto position = juce::jmap(dbNormScaled, 0.0f, isRotated ? b.getHeight() : b.getWidth());

            path.clear();

            path.startNewSubPath(isRotated ? 0.0f : position, isRotated ? position : b.getY());
            path.lineTo(isRotated ? b.getWidth() : position, isRotated ? position : b.getBottom());

            g.strokePath(path, stroke);
        }

        // Spectrum: Bar division lines (lines in between the bars)
        {
            const auto viewBox = m_component->getViewBox();

            // Normalized bounds: x-axis in rotated mode, y-axis otherwise
            const auto minNorm = isRotated ? std::get<0>(viewBox) : std::get<2>(viewBox);
            const auto maxNorm = isRotated ? std::get<1>(viewBox) : std::get<3>(viewBox);

            // Pixel bounds: 0->width in rotated mode, height->0 otherwise
            const auto minBound = isRotated ? 0.0f : (float)getHeight();
            const auto maxBound = isRotated ? (float)getWidth() : 0.0f;

            const auto numBins = m_processor.getAnalyzerNumBins() * (1.0f / (isRotated ? Cursor::get().getZoom().x : Cursor::get().getZoom().y));
            juce::Path path;

            for (int i = 0; i < numBins; ++i) {
                // Normalized position
                const auto normPos = i * (1.0f / numBins);

                // Don't show lines out of view
                if (normPos < minNorm || normPos > maxNorm) {
                    continue;
                }

                // Compute target-position in current window
                const auto pos = juce::jmap(normPos, minNorm, maxNorm, minBound, maxBound);

                // Draw line: x-axis in rotated mode, y-axis otherwise
                path.clear();

                path.startNewSubPath(isRotated ? pos : 0.0f, isRotated ? 0.0f : pos);
                path.lineTo(isRotated ? pos : b.getRight(), isRotated ? b.getHeight() : pos);

                g.strokePath(path, stroke);
            }
        }
    }

    // DB Marker Lines - Meter
    /*
    if (m_type == Type::Meters) {
        const auto b = getLocalBounds().toFloat();

        // Get mix mode
        const auto mixMode =
          m_processor.getParameter<MixMode>(ProcessorParameters::Key::MixMode);

        if (m_processor.isValid()) {
            const auto sync =
              [&](Channel channel,
                  SyncInfo<MeteringValue> first,
                  std::optional<SyncInfo<MeteringValue>> second_) {
                  auto info = m_processor.getMeteringInfo(channel);

                  // Ensure valid data
                  if (info.Height == 0) {
                      return;
                  }

                  // Ensure size of waveform data
                  m_meteringData[channel].resize(info.Height);

                  // Ensure valid data is present
                  if (!first.isValid()) {
                      return;
                  }
                  // ModelCopy, so second part should always be
                  // null, and only require a single copy
                  ASSERT(!second_, "Model should be ModelCopy");
                  std::copy_n(first.Pointer,
                              first.Height,
                              m_meteringData[channel].data() + first.RowIndex);
                  (void)second_;
              };

            // Synchronize data
            using namespace std::placeholders;
            switch (mixMode) {
                case MixMode::Stereo: {
                    m_processor.syncMetering(Left,
                                             std::bind(sync, Left, _1, _2));
                    m_processor.syncMetering(Right,
                                             std::bind(sync, Right, _1, _2));
                } break;
                case MixMode::Left:
                case MixMode::Right:
                case MixMode::Mid:
                case MixMode::Side: {
                    m_processor.syncMetering(Mix, std::bind(sync, Mix, _1, _2));
                } break;
            }

            const auto width = b.getWidth() / 2;
            const auto height = b.getHeight() / 2;

            auto meterWidth = width;
            auto meterHeight = height;
            if (mixMode == MixMode::Stereo) {
                meterWidth = isRotated ? width : width / 2;
                meterHeight = isRotated ? height / 2 : height;
            }

            // Update time
            const auto now = std::chrono::high_resolution_clock::now();
            const auto delta =
              std::chrono::duration_cast<std::chrono::nanoseconds>(
                now - m_lastPaintTime)
                .count() /
              1.0e6f;

            m_lastPaintTime = now;

            // Compute time constant
            const auto meterFalloff = m_processor.getMeterFalloff();

            const auto falloffTimeConstant =
              (meterFalloff > 0) ? 1.0f - expf(-4.605f / (meterFalloff / delta))
                                 : 0.0f;

            // Update hold state values
            const auto updateMeterHoldState = [=](auto& meterHoldState,
                                                  const auto peak) {
                if (peak < meterHoldState.Value) {
                    // Update hold state time
                    meterHoldState.Time += delta;

                    // Hold time is always 1000 ms
                    if (meterHoldState.Time > 1000.0f) {
                        meterHoldState.Value =
                          lerp(meterHoldState.Value, peak, falloffTimeConstant);
                    }
                } else {
                    meterHoldState.Value = peak;
                    meterHoldState.Time = 0.0f;
                }
            };

            switch (mixMode) {
                case MixMode::Stereo: {
                    // Ensure valid data or stop rendering
                    if (m_meteringData[Left].empty()) {
                        return;
                    }
                    if (m_meteringData[Right].empty()) {
                        return;
                    }

                    // Update states
                    updateMeterHoldState(m_peakHoldState[0],
                                         m_meteringData[Left][0].Peak);
                    updateMeterHoldState(m_peakHoldState[1],
                                         m_meteringData[Right][0].Peak);

                    if (isRotated) {
                        // Peak
                        drawMeter(g,
                                  m_meteringData[Left][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 3,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Left][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 1,
                                  meterHeight * 3,
                                  meterWidth,
                                  meterHeight,
                                  true);
                        drawMeter(g,
                                  m_meteringData[Right][0].Peak,
                                  m_peakHoldState[1].Value,
                                  meterWidth * 0,
                                  meterHeight * 2,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Right][0].Peak,
                                  m_peakHoldState[1].Value,
                                  meterWidth * 1,
                                  meterHeight * 2,
                                  meterWidth,
                                  meterHeight,
                                  true);

                        // RMS
                        drawMeter(g,
                                  m_meteringData[Left][0].Rms,
                                  -1.0f,
                                  meterWidth * 0,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Left][0].Rms,
                                  -1.0f,
                                  meterWidth * 1,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);
                        drawMeter(g,
                                  m_meteringData[Right][0].Rms,
                                  -1.0f,
                                  meterWidth * 0,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Right][0].Rms,
                                  -1.0f,
                                  meterWidth * 1,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight,
                                  true);
                    } else {
                        // Peak
                        drawMeter(g,
                                  m_meteringData[Left][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Left][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);
                        drawMeter(g,
                                  m_meteringData[Right][0].Peak,
                                  m_peakHoldState[1].Value,
                                  meterWidth * 1,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Right][0].Peak,
                                  m_peakHoldState[1].Value,
                                  meterWidth * 1,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);

                        // RMS
                        drawMeter(g,
                                  m_meteringData[Left][0].Rms,
                                  -1.0f,
                                  meterWidth * 2,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Left][0].Rms,
                                  -1.0f,
                                  meterWidth * 2,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);
                        drawMeter(g,
                                  m_meteringData[Right][0].Rms,
                                  -1.0f,
                                  meterWidth * 3,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Right][0].Rms,
                                  -1.0f,
                                  meterWidth * 3,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);
                    }

                } break;
                case MixMode::Left:
                case MixMode::Right:
                case MixMode::Mid:
                case MixMode::Side: {
                    // Ensure valid data or stop rendering
                    if (m_meteringData[Mix].empty()) {
                        return;
                    }

                    // Update states
                    updateMeterHoldState(m_peakHoldState[0],
                                         m_meteringData[Mix][0].Peak);

                    if (isRotated) {
                        // Peak
                        drawMeter(g,
                                  m_meteringData[Mix][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Mix][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 1,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);

                        // RMS
                        drawMeter(g,
                                  m_meteringData[Mix][0].Rms,
                                  -1.0f,
                                  meterWidth * 0,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Mix][0].Rms,
                                  -1.0f,
                                  meterWidth * 1,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight,
                                  true);
                    } else {
                        // Peak
                        drawMeter(g,
                                  m_meteringData[Mix][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Mix][0].Peak,
                                  m_peakHoldState[0].Value,
                                  meterWidth * 0,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);

                        // RMS
                        drawMeter(g,
                                  m_meteringData[Mix][0].Rms,
                                  -1.0f,
                                  meterWidth * 1,
                                  meterHeight * 0,
                                  meterWidth,
                                  meterHeight);
                        drawMeter(g,
                                  m_meteringData[Mix][0].Rms,
                                  -1.0f,
                                  meterWidth * 1,
                                  meterHeight * 1,
                                  meterWidth,
                                  meterHeight,
                                  true);
                    }

                } break;
            }
        }

        // Center line (vertically)
        {
            const float strokeWidth = 3.0f;
            const juce::PathStrokeType stroke{ strokeWidth };

            g.setColour(BorderColor);

            juce::Path path;
            path.startNewSubPath(isRotated ? 0.0f : b.getCentreX(),
                                 isRotated ? b.getCentreY() : 0.0f);
            path.lineTo(isRotated ? b.getWidth() : b.getCentreX(),
                        isRotated ? b.getCentreY() : b.getHeight());

            g.strokePath(path, stroke);

            if (mixMode == MixMode::Stereo) {
                int pathStartX =
                  isRotated ? 0.0f : b.getCentreX() / 2 - strokeWidth / 4;
                int pathStartY =
                  isRotated ? b.getCentreY() / 2 - strokeWidth / 4 : 0;
                int pathEndX = isRotated ? b.getRight()
                                         : b.getCentreX() / 2 - strokeWidth / 4;
                int pathEndY = isRotated ? b.getCentreY() / 2 - strokeWidth / 4
                                         : b.getHeight();

                path.startNewSubPath(pathStartX, pathStartY);
                path.lineTo(pathEndX, pathEndY);

                g.strokePath(path, stroke);

                pathStartX = isRotated ? 0.0f
                                       : b.getCentreX() + b.getCentreX() / 2 +
                                           strokeWidth / 4;
                pathStartY = isRotated ? b.getCentreY() + b.getCentreY() / 2 +
                                           strokeWidth / 4
                                       : 0;
                pathEndX = isRotated
                             ? b.getRight()
                             : b.getCentreX() + b.getCentreX() / 2 + 3.0f / 4;
                pathEndY = isRotated
                             ? b.getCentreY() + b.getCentreY() / 2 + 3.0f / 4
                             : b.getHeight();

                path.startNewSubPath(pathStartX, pathStartY);
                path.lineTo(pathEndX, pathEndY);

                g.strokePath(path, stroke);
            }
        }
    }
    */

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
        const auto lineColor = LineColor;

        // Instantiate the desired component and defaults
        switch (m_type) {
            case Type::Spectrum: {
                m_component = std::make_unique<spectrex::KSpectrumComponent>(m_processor, m_openGLContext.getVisualizationContext());
            } break;
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
                ((spectrex::KWaveformComponent*)(m_component.get()))->setColorRamp({ 0.94117647058f, 0.94117647058f, 0.94117647058f, 1.0f });
            } break;
            case Type::Goniometer: {
                m_component = std::make_unique<spectrex::KGoniometerComponent>(m_processor, m_openGLContext.getVisualizationContext());
            } break;
            case Type::Meters: {
                m_component = std::make_unique<spectrex::KMeteringComponent>(m_processor, m_openGLContext.getVisualizationContext());
            } break;
            case Type::CorrelationMeter: {
                m_component = std::make_unique<spectrex::KCorrelationMeterComponent>(m_processor, m_openGLContext.getVisualizationContext());
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
    // m_timer.start();

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

    // Reset scissor test
    // glDisable(GL_SCISSOR_TEST);

    // m_timer.stop();
}

void
VisualizationComponent::openGLContextClosing()
{
    // Destroy our own context
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

    // Detect display scale or DPI changes using timer because there is no
    // reliable JUCE callback for this
    {
        const auto currentTime = juce::Time::getApproximateMillisecondCounter();
        if (m_lastClipUpdate - currentTime >= 500) {

            updateClippingBounds();
            m_lastClipUpdate = currentTime;
        }
    }

    // Metering
    if (m_type == Type::Meters) {
        // Keep the spectrum and waveform's data positions in sync with the
        // viewbox
        {
            const Rect viewBox = Rect{ cursor.getPosition() + glm::vec2{ 0.5f, 0.5f }, cursor.getZoom() };

            // If the component is rotated, use the top, otherwise use the right
            const bool isRotated = false; // @param
            auto dataPosition = isRotated ? viewBox.getTop() : viewBox.getRight();

            m_processor.setAnalyzerDataPosition(dataPosition);
            m_processor.setMeterDataPosition(dataPosition);
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

VisualizationComponent::VisualizationComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor, Type type)
  : m_type(type)
  , m_openGLContext(context)
  , m_pluginProcessor(processor)
  , m_processor(processor.getSpectrexMiniProcessor().getProcessor())
  //  , m_state(processor.getParameterState())
  , m_shouldDrawMouseTargetLines(false)
{
    setOpaque(true);

    // Add renderer
    m_openGLContext.addRenderingTarget(this);
    // m_openGLContext.getContext().setContinuousRepainting(true);

    /// PARAM: Add parameters here (1/3)
    //
    // Sets up all parameter listeners
    //
    /* @param
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_minDb.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_maxDb.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_minFreq.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_maxFreq.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_timeFactor.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_msTimeFactor.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_timeMultiplier.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_ftSize.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_mix.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_colorRamp.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_overrideMode.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_syncMode.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerReference.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerFalloff.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_analyzerHold.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerFlatten.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerDbScale.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_mapBias.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerNumBins.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerHighlightFalloff.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_analyzerHighlightThreshold.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_barGraphHighlight.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_waveformHeadroom.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_meterFalloff.id);
    attachAndUpdateParam(
      m_pluginProcessor.getParameters().m_waveformHighlight.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_listenEnable.id);
    attachAndUpdateParam(m_pluginProcessor.getParameters().m_rotate.id);

    m_pluginProcessor.getParameterState().state.addListener(this);
    */

    // Add key listener
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    setVisible(true);

    startTimer(1000 / 60);
}

VisualizationComponent::~VisualizationComponent()
{
    /// PARAM: Add parameters here (2/3)
    //
    // Cleans up all parameter listeners
    //
    /* @param
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_minDb.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_maxDb.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_minFreq.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_maxFreq.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_timeFactor.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_msTimeFactor.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_timeMultiplier.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_ftSize.id, this);
    m_state.removeParameterListener(m_pluginProcessor.getParameters().m_mix.id,
                                    this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_colorRamp.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_overrideMode.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_syncMode.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerReference.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerFalloff.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerHold.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerFlatten.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerDbScale.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_mapBias.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_barGraphHighlight.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerNumBins.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerHighlightFalloff.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_analyzerHighlightThreshold.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_waveformHighlight.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_listenEnable.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_waveformHeadroom.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_meterFalloff.id, this);
    m_state.removeParameterListener(
      m_pluginProcessor.getParameters().m_rotate.id, this);

    m_pluginProcessor.getParameterState().state.removeListener(this);
    */

    m_openGLContext.removeRenderingTarget(this);
}

void
VisualizationComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    //
}

void
VisualizationComponent::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
{
    //
}

void
VisualizationComponent::drawMeter(juce::Graphics& g, float v, float holdValue, float x, float y, float width, float height, bool flip) noexcept
{
    // Rotate graphs
    const bool isRotated = false; // @param

    // Number of sub divisions based on height
    const auto numSubdivisions = 16 * std::ceil((isRotated ? width : height) / 123.0f);

    // Read min. and max. dB values
    const auto minDb = m_component->getMinDb();
    const auto maxDb = m_component->getMaxDb();

    // Convert to range
    const auto range = std::abs(minDb - maxDb);

    // Normalized value
    const auto vDb = juce::Decibels::gainToDecibels(v);
    const auto normV = juce::jmap(vDb, minDb, maxDb, 0.0f, 1.0f);

    // Compute steps in dB
    const auto dbStep = range / (numSubdivisions - 1);

    const auto padding = 2.0f; // In pixels

    const auto ledThickness = ((isRotated ? width : height) - numSubdivisions * padding) / numSubdivisions;

    const auto ledOffset = ledThickness + padding;

    const auto dbMarks = MeterDbTextVals;

    // Hold line
    if (std::floor(holdValue * numSubdivisions) > 0.0f) {
        g.setColour(MeterHold);

        const int xPos =
          isRotated ? flip ? (x + (int)(numSubdivisions * holdValue) * ledOffset) : width - (x + ((int)(numSubdivisions * holdValue) + 1) * ledOffset)
                    : x;
        const int yPos = isRotated ? y
                         : flip    ? (y + (int)(numSubdivisions * holdValue) * ledOffset)
                                   : height - (y + ((int)(numSubdivisions * holdValue) + 1) * ledOffset);
        const int ledWidth = isRotated ? ledThickness : width;
        const int ledHeight = isRotated ? height : ledThickness;

        g.fillRect(juce::Rectangle(xPos, yPos, ledWidth, ledHeight));
    }

    // Meter bars
    for (int i = 0; i < std::floor(numSubdivisions * normV); ++i) {
        const auto rangeMin = (int)(minDb + (i + 0) * dbStep);
        const auto rangeMax = (int)(minDb + (i + 1) * dbStep);

        if (rangeMin >= 0) {
            g.setColour(MeterRed);
        } else {
            const auto highlight = std::any_of(dbMarks.begin(), dbMarks.end(), [=](const auto& val) { return val >= rangeMin && val < rangeMax; });

            if (highlight) {
                g.setColour(MeterYellow);
            } else {
                g.setColour(MeterWhite);
            }
        }

        const int xPos = isRotated ? (flip ? (x + i * ledOffset) : width - (x + (i + 1) * ledOffset)) : x;
        const int yPos = isRotated ? y : (flip ? (y + i * ledOffset) : height - (y + (i + 1) * ledOffset));

        const int ledWidth = isRotated ? ledThickness : width;

        const int ledHeight = isRotated ? height : ledThickness;

        g.fillRect(juce::Rectangle(xPos, yPos, ledWidth, ledHeight));
    }
}

void
VisualizationComponent::attachAndUpdateParam(const juce::String& id)
{
    // m_state.addParameterListener(id, this);
    // parameterChanged(id, *m_state.getRawParameterValue(id));
}

void
VisualizationComponent::initialUpdate()
{
    /// PARAM: Add parameters here (3/3)
    //
    // Sets the initial (default) values for all parameters
    //
    /* @param
    parameterChanged(m_pluginProcessor.getParameters().m_minDb.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_minDb.id));
    parameterChanged(m_pluginProcessor.getParameters().m_maxDb.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_maxDb.id));
    parameterChanged(m_pluginProcessor.getParameters().m_minFreq.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_minFreq.id));
    parameterChanged(m_pluginProcessor.getParameters().m_maxFreq.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_maxFreq.id));
    parameterChanged(m_pluginProcessor.getParameters().m_timeFactor.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_timeFactor.id));
    parameterChanged(m_pluginProcessor.getParameters().m_msTimeFactor.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_msTimeFactor.id));
    parameterChanged(m_pluginProcessor.getParameters().m_timeMultiplier.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_timeMultiplier.id));
    parameterChanged(m_pluginProcessor.getParameters().m_ftSize.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_ftSize.id));
    parameterChanged(m_pluginProcessor.getParameters().m_mix.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_mix.id));
    parameterChanged(m_pluginProcessor.getParameters().m_colorRamp.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_colorRamp.id));
    parameterChanged(m_pluginProcessor.getParameters().m_overrideMode.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_overrideMode.id));
    parameterChanged(m_pluginProcessor.getParameters().m_syncMode.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_syncMode.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerReference.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerReference.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerFalloff.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerFalloff.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerHold.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerHold.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerFlatten.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerFlatten.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerDbScale.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerDbScale.id));
    parameterChanged(m_pluginProcessor.getParameters().m_barGraphHighlight.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_barGraphHighlight.id));
    parameterChanged(m_pluginProcessor.getParameters().m_mapBias.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_mapBias.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerNumBins.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerNumBins.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerHighlightFalloff.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerHighlightFalloff.id));
    parameterChanged(m_pluginProcessor.getParameters().m_analyzerHighlightThreshold.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_analyzerHighlightThreshold.id));
    parameterChanged(m_pluginProcessor.getParameters().m_waveformHighlight.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_waveformHighlight.id));
    parameterChanged(m_pluginProcessor.getParameters().m_listenEnable.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_listenEnable.id));
    parameterChanged(m_pluginProcessor.getParameters().m_waveformHeadroom.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_waveformHeadroom.id));
    parameterChanged(m_pluginProcessor.getParameters().m_meterFalloff.id,
                     *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_meterFalloff.id));
    parameterChanged(m_pluginProcessor.getParameters().m_rotate.id, *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_rotate.id));
    */
}

void
VisualizationComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    /* @param
    // We don't create components till the context is created so we need to
    // check
    if (!m_component->get()) {
        return;
    }

    // Since we're calling repaint we need to ensure this is on the message
    // thread
    if (!juce::MessageManager::existsAndIsCurrentThread()) {
        juce::Component::SafePointer<VisualizationComponent> safePtr{ this };
        auto callback = [=]() {
            if (safePtr.getComponent()) {
                parameterChanged(parameterID, newValue);
            }
        };
        juce::MessageManager::callAsync(callback);

        return;
    }

    std::string errorMessage = "";

    const auto setColorRamp = [](float value, auto* component) {
        switch ((int)round(value + 1.0f)) {
            case 1:
                component->setColorRamp(colorramps::Rocket);
                break;
            case 2:
                component->setColorRamp(colorramps::Mako);
                break;
            case 3:
                component->setColorRamp(colorramps::Viridis);
                break;
            case 4:
                component->setColorRamp(colorramps::Magma);
                break;
            case 5:
                component->setColorRamp(colorramps::Inferno);
                break;
            case 6:
                component->setColorRamp(colorramps::Cividis);
                break;
            case 7:
                component->setColorRamp(colorramps::Turbo);
                break;
            case 8:
                component->setColorRamp(colorramps::Fire);
                break;
            case 9:
                component->setColorRamp(colorramps::Heatmap);
                break;
            case 10:
                component->setColorRamp(colorramps::Grayscale);
                break;
        }
    };

    const bool isSynced = m_pluginProcessor.getParameterState().getRawParameterValue(m_pluginProcessor.getParameters().m_syncMode.id)->load() > 0.5f;

    if (parameterID == m_pluginProcessor.getParameters().m_minDb.id) {
        m_component->setMinDb(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_maxDb.id) {
        m_component->setMaxDb(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_minFreq.id) {
        m_component->setMinFrequency(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_maxFreq.id) {
        m_component->setMaxFrequency(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_timeFactor.id && isSynced) {
        float factor = 0.0f;

        switch ((int)round(newValue + 1.0f)) {
            case 1:
                factor = 1.0f / 32.0f;
                break;
            case 2:
                factor = 1.0f / 16.0f;
                break;
            case 3:
                factor = 1.0f / 8.0f;
                break;
            case 4:
                factor = 1.0f / 4.0f;
                break;
            case 5:
                factor = 1.0f / 2.0f;
                break;
            case 6:
                factor = 1.0f / 1.0f;
                break;
        }

        m_processor.setParameter<float>(ProcessorParameters::Key::TimeFactor, factor);

        // TODO: This setNumBars duplication of properties needs to be removed
        if (m_processor.getTimeQuantity() > 0) {
            m_component->setNumBars(m_processor.getTimeQuantity());
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_msTimeFactor.id && !isSynced) {
        m_processor.setParameter<float>(ProcessorParameters::Key::TimeFactor, newValue / 1000.0f);
    } else if (parameterID == m_pluginProcessor.getParameters().m_timeMultiplier.id) {

        m_processor.setParameter<float>(ProcessorParameters::Key::TimeMultiplier, (int)newValue);

        // TODO: This setNumBars duplication of properties needs to be removed
        if (m_processor.getTimeQuantity() > 0) {
            m_component->setNumBars(m_processor.getTimeQuantity());
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_ftSize.id) {
        FtSize ftSize;

        switch ((int)round(newValue + 1.0f)) {
            case 1:
                ftSize = FtSize::Size2048;
                break;
            case 2:
                ftSize = FtSize::Size4096;
                break;
            case 3:
                ftSize = FtSize::Size8192;
                break;
            default:
                jassertfalse;
        }

        m_processor.setParameter<FtSize>(ProcessorParameters::Key::FtSize, ftSize);
    } else if (parameterID == m_pluginProcessor.getParameters().m_mix.id) {
        switch ((int)round(newValue + 1.0f)) {
            case 1:
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::MixMode, MixMode::Stereo);
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::ListenMode, MixMode::Stereo);
                break;
            case 2:
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::MixMode, MixMode::Left);
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::ListenMode, MixMode::Left);
                break;
            case 3:
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::MixMode, MixMode::Right);
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::ListenMode, MixMode::Right);
                break;
            case 4:
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::MixMode, MixMode::Mid);
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::ListenMode, MixMode::Mid);
                break;
            case 5:
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::MixMode, MixMode::Side);
                m_processor.setParameter<MixMode>(ProcessorParameters::Key::ListenMode, MixMode::Side);
                break;
            default:
                jassertfalse;
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_overrideMode.id) {
        m_processor.setParameter<bool>(ProcessorParameters::Key::Override, newValue > 0.5f);
    } else if (parameterID == m_pluginProcessor.getParameters().m_syncMode.id) {
        // When in standalone mode, always disable syncing
        const auto value = juce::JUCEApplication::isStandaloneApp() ? false : newValue > 0.5f;

        m_processor.setParameter<bool>(ProcessorParameters::Key::PlayHeadSynced, value);

        if (value) {
            parameterChanged(m_pluginProcessor.getParameters().m_timeFactor.id,
                             m_pluginProcessor.getParameterState().getRawParameterValue(m_pluginProcessor.getParameters().m_timeFactor.id)->load());
        } else {
            parameterChanged(m_pluginProcessor.getParameters().m_msTimeFactor.id,
                             m_pluginProcessor.getParameterState().getRawParameterValue(m_pluginProcessor.getParameters().m_msTimeFactor.id)->load());
        }

    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerReference.id) {
        auto* spectrumComponent = dynamic_cast<SpectrumComponent*>(m_component->get());

        if (spectrumComponent != nullptr) {
            switch (static_cast<int>(round(newValue))) {
                case 1:
                    spectrumComponent->setEnableReference(false);
                    break;
                case 2: // Acoustic
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Acoustic_Max_bin, assets::Acoustic_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Acoustic_Average_bin,
                                                             assets::Acoustic_Average_binSize / sizeof(float));
                    break;
                case 3: // Ambient
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Ambient_Max_bin, assets::Ambient_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Ambient_Average_bin,
                                                             assets::Ambient_Average_binSize / sizeof(float));
                    break;
                case 4: // Classical
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Classical_Max_bin, assets::Classical_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Classical_Average_bin,
                                                             assets::Classical_Average_binSize / sizeof(float));
                    break;
                case 5: // DNB Diep
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::DNB_Diep_High_bin, assets::DNB_Diep_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::DNB_Diep_Low_bin, assets::DNB_Diep_Low_binSize / sizeof(float));
                    break;
                case 6: // DNB Hard
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::DNB_Hard_High_bin, assets::DNB_Hard_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::DNB_Hard_Low_bin, assets::DNB_Hard_Low_binSize / sizeof(float));
                    break;
                case 7: // DNB Jungle
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::DNB_Jungle_High_bin,
                                                             assets::DNB_Jungle_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::DNB_Jungle_Low_bin,
                                                             assets::DNB_Jungle_Low_binSize / sizeof(float));
                    break;
                case 8: // Hip-hop
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::HipHop_Max_bin, assets::HipHop_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::HipHop_Average_bin,
                                                             assets::HipHop_Average_binSize / sizeof(float));
                    break;
                case 9: // House
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::House_Max_bin, assets::House_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::House_Average_bin, assets::House_Average_binSize / sizeof(float));
                    break;
                case 10: // IDM
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::IDM_High_bin, assets::IDM_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::IDM_Low_bin, assets::IDM_Low_binSize / sizeof(float));
                    break;
                case 11: // Indie
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Indie_Max_bin, assets::Indie_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Indie_Average_bin, assets::Indie_Average_binSize / sizeof(float));
                    break;
                case 12: // Jazz
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Jazz_Max_bin, assets::Jazz_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Jazz_Average_bin, assets::Jazz_Average_binSize / sizeof(float));
                    break;
                case 13: // Modern Pop
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Modern_Pop_Max_bin,
                                                             assets::Modern_Pop_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Modern_Pop_Average_bin,
                                                             assets::Modern_Pop_Average_binSize / sizeof(float));
                    break;
                case 14: // Pink Noise
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Pink_Noise_High_bin,
                                                             assets::Pink_Noise_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Pink_Noise_Low_bin,
                                                             assets::Pink_Noise_Low_binSize / sizeof(float));
                    break;
                case 15: // Rock
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Rock_Max_bin, assets::Rock_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Rock_Average_bin, assets::Rock_Average_binSize / sizeof(float));
                    break;
                case 16: // Techno
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Techno_Max_bin, assets::Techno_Max_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Techno_Average_bin,
                                                             assets::Techno_Average_binSize / sizeof(float));
                    break;
                case 17: // Dubstep
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Dubstep_High_bin, assets::Dubstep_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Dubstep_Low_bin, assets::Dubstep_Low_binSize / sizeof(float));
                    break;
                case 18: // Trap
                    spectrumComponent->setEnableReference(true);
                    spectrumComponent->setReferenceUpperData((const float*)assets::Trap_High_bin, assets::Trap_High_binSize / sizeof(float));
                    spectrumComponent->setReferenceLowerData((const float*)assets::Trap_Low_bin, assets::Trap_Low_binSize / sizeof(float));
                    break;
            }
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerFalloff.id) {
        m_processor.setAnalyzerFalloff(newValue * 0.001f); // ms to s
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerHold.id) {
        m_processor.setAnalyzerHold(newValue * 0.001f); // ms to s
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerFlatten.id) {
        m_processor.setParameter(ProcessorParameters::Key::Flatten, (bool)newValue && !m_pluginProcessor.isLiteLicense());
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerDbScale.id) {
        m_processor.setAnalyzerDbScale(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_mapBias.id) {
        m_processor.setAnalyzerDbPerOctave(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerNumBins.id) {
        // TWEAK: Not obvious: if set to the maximum parameter value, assume 0
        // will will trigger lines instead of bars.
        if ((int)newValue == (int)m_pluginProcessor.getParameters().m_analyzerNumBins.range.end) {
            newValue = 0.0f;
        }
        m_processor.setAnalyzerNumBins(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerHighlightFalloff.id) {
        m_processor.setAnalyzerHighlightFalloff(newValue * 0.001f); // ms to s
    } else if (parameterID == m_pluginProcessor.getParameters().m_analyzerHighlightThreshold.id) {
        m_processor.setAnalyzerHighlightThreshold(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_colorRamp.id) {
        // Spectrogram
        {
            auto* spectrogramComponent = dynamic_cast<SpectrogramComponent*>(m_component->get());

            if (spectrogramComponent != nullptr) {
                setColorRamp(newValue, spectrogramComponent);
            }
        }

        const int waveformColorRamp = juce::roundFloatToInt(*m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_waveformHighlight.id));

        const int barGraphColorRamp = juce::roundFloatToInt(*m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_barGraphHighlight.id));

        // Waveform: When changing the color ramp parameter (spectrogram) and
        // the waveform color ramp is set to 2.0f: Update the waveform color
        // ramp to be equal to the spectrogram color ramp.
        {
            auto* waveformComponent = dynamic_cast<WaveformComponent*>(m_component->get());

            if (waveformColorRamp == 2 && waveformComponent != nullptr) {
                setColorRamp(newValue, waveformComponent);
            }
        }

        // Same for spectrum
        {
            auto* spectrumComponent = dynamic_cast<SpectrumComponent*>(m_component->get());

            if (barGraphColorRamp == 2 && spectrumComponent != nullptr) {
                setColorRamp(newValue, spectrumComponent);
            }
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_waveformHighlight.id) {
        // When this value is 0: No color map or highlighting, when this value
        // is 1: do highlighting but no color map, when this valye is 2: do both
        // highlighting and color map.

        auto* waveformComponent = dynamic_cast<WaveformComponent*>(m_component->get());

        const int selection = juce::roundFloatToInt(newValue);

        if (waveformComponent != nullptr) {
            switch (selection) {
                case 0:
                    waveformComponent->setColorRamp(colorramps::MeterWhite);
                    break;
                case 1:
                    waveformComponent->setColorRamp(colorramps::Grayscale);
                    break;
                case 2: {
                    const float colorRamp = *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_colorRamp.id);
                    setColorRamp(colorRamp, waveformComponent);
                    break;
                }
                default:
                    jassertfalse;
            }
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_listenEnable.id) {
        if (newValue > 0.5f) {
            // Enable listening
            m_processor.setParameter<bool>(ProcessorParameters::Key::Listen, true);
        } else {
            // Disable listening
            m_processor.setParameter<bool>(ProcessorParameters::Key::Listen, false);
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_barGraphHighlight.id) {
        // When this value is 0: No color map or highlighting, when this value
        // is 1: do highlighting but no color map, when this valye is 2: do both
        // highlighting and color map.

        auto* spectrumComponent = dynamic_cast<SpectrumComponent*>(m_component->get());

        const int selection = juce::roundFloatToInt(newValue);

        if (spectrumComponent != nullptr) {

            switch (selection) {
                case 0:
                    spectrumComponent->setColorRamp(colorramps::MeterWhite);
                    break;
                case 1:
                    spectrumComponent->setColorRamp(colorramps::Grayscale);
                    break;
                case 2: {
                    const float colorRamp = *m_state.getRawParameterValue(m_pluginProcessor.getParameters().m_colorRamp.id);
                    setColorRamp(colorRamp, spectrumComponent);
                    break;
                }
                default:
                    jassertfalse;
            }
        }
    } else if (parameterID == m_pluginProcessor.getParameters().m_waveformHeadroom.id) {
        m_processor.setWaveformHeadroom(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_meterFalloff.id) {
        m_processor.setMeterFalloff(newValue);
        m_processor.setPeakRelease(newValue);
    } else if (parameterID == m_pluginProcessor.getParameters().m_rotate.id) {
        m_processor.setParameter(ProcessorParameters::Key::Rotate, (bool)newValue && !m_pluginProcessor.isLiteLicense());
    }

    // Updates text labels
    // If I repaint the parent separately the texts and lines don't update
    // together and it looks wrong.
    // getParentComponent()->repaint();
    */
}
