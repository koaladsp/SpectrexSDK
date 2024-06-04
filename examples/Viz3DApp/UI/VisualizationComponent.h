#pragma once

#include "../WindowOpenGLContext.h"

// spectrex
#include <Spectrex/Components/Component.hpp>
#include <Spectrex/Processing/Processor.hpp>
#include <Spectrex/Utility/Utility.hpp>

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>

// Stdlib
#include <array>
#include <chrono>
#include <vector>

/* Forward declarations */

class Component;

class PluginAudioProcessor;

/* MeterHoldState */

struct MeterHoldState
{
    float Time = 0.0f;

    float Value = 0.0f;
};

/* VisualizationComponent */

class VisualizationComponent final
  : public juce::Component
  , public juce::OpenGLRenderer
  , public juce::AudioProcessorValueTreeState::Listener
  , public juce::KeyListener
  , public juce::Timer
  , public juce::Button::Listener
  , public juce::ValueTree::Listener
{
  public:
    enum class Type
    {
        Waveform,
        Spectrogram,
        Spectrum,
        Goniometer,
        Meters,
        CorrelationMeter
    };

  public:
    void paint(juce::Graphics& g) override;

    void resized() override;

    void moved() override;

    void parentHierarchyChanged() override;

    void newOpenGLContextCreated() override;

    void renderOpenGL() override;

    void openGLContextClosing() override;

    void mouseMove(const juce::MouseEvent& event) override;

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent& event) override;

    void mouseDrag(const juce::MouseEvent& event) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void mouseMagnify(const juce::MouseEvent& event, float scaleFactor) override;

    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void buttonClicked(juce::Button* button) override;

    void buttonStateChanged(juce::Button* button) override;

    /// @brief Get the ppq of this component that was most recently drawn.
    /// @return ppq.
    auto getPpqLastDrawn() const noexcept -> float;

    /// @brief Called when any key is pressed.
    /// @param key Key pressed.
    /// @param originatingComponent The component that received the key event.
    /// @return True when consumed, otherwise false.
    auto keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) -> bool override;

    void timerCallback() override;

    /// @brief Sets whether or not the component should draw lines to show
    /// relative mouse position inside the visualization component. This is used
    /// to disable drawing the mouse target lines when the user is mousing over
    /// an unrelated component like the controls panel.
    /// @param shouldDrawTarget Whether or not to draw the mouse target lines.
    void setShouldDrawMouseTarget(bool shouldDrawTarget) noexcept;

    /// @brief Returns the the viewbox boundaries (taking into account zooming
    /// and panning) within [0, 1] range, where 0 is the actual left/bottom of
    /// the component and 1 is the actual right/top of the underlying component.
    /// @return Normalized viewbox boundaries, order: left, right, bottom, top.
    auto getViewBox() const noexcept -> std::tuple<float, float, float, float>;

    /// @brief Gets a description about the data at the current mouse position
    /// inside this component.
    /// @return A String describing the data under the mouse cursor in this
    /// component. If the mouse is not currently inside this component it
    /// returns nothing.
    auto getMouseTargetText() const noexcept -> const juce::String;

    /// @brief Returns the type of component this contains.
    /// @return The type of this component.
    auto getType() const noexcept -> const Type;

    /// @brief Returns the underlying spectrex component.
    /// @return Spectrex component.
    spectrex::KComponent* getSpectrexComponent() noexcept { return m_component.get(); }

    VisualizationComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor, Type type);

    ~VisualizationComponent();

    inline static const juce::Identifier k_refShiftId = "ref_shift_id";

  private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;

    /// @brief Draws a meter (as separate "LEDs") with value \a v (in range [0,
    /// 1]), at location (\a x, \a y) with dimensions (\a width, \a height).
    /// @param g Graphics object for drawing.
    /// @param v Value of meter.
    /// @param holdValue Meter hold value.
    /// @param x X-position of meter.
    /// @param y Y-position of meter.
    /// @param width Width of meter.
    /// @param height Height of meter.
    /// @param flip Whether or not to flip vertically.
    void drawMeter(juce::Graphics& g, float v, float holdValue, float x, float y, float width, float height, bool flip = false) noexcept;

    /// @brief Attaches this to our state and calls our callback for each
    /// parameter we add.
    /// @param id the id of the parameter to listen to.
    void attachAndUpdateParam(const juce::String& id);

    /// @brief Call when recreating components to synchronize state
    void initialUpdate();

    /// @brief Called when our state is updated so we can synchronize state for
    /// our internal components.
    /// @param parameterID the id the callback is for
    /// @param newValue the value of parameter \a parameterID that's been
    /// updated.
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    /// @brief Recalculate clipping boundaries, essential to get properly DPI
    /// scaled render target.
    void updateClippingBounds();

  private:
    /* Constants */

    const float k_barStrokeWidth = 2.0f;

    const float k_beatStrokeWidth = 1.0f;

    const float k_dbMarkerStrokeWidth = 0.5f;

    const Type m_type;

    juce::Rectangle<int> m_clippingBounds;

    /* spectrex */

    utility::WindowOpenGLContext& m_openGLContext;

    PluginAudioProcessor& m_pluginProcessor;

    spectrex::KProcessor& m_processor;

    //    juce::AudioProcessorValueTreeState& m_state;

    std::unique_ptr<spectrex::KComponent> m_component;

    // MovingAverageTimer m_timer;

    /* Timing */

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastPaintTime;

    double m_lastPpq;

    /* Data */

    std::array<std::vector<spectrex::MeteringValue>, 3> m_meteringData;

    std::array<MeterHoldState, 2> m_peakHoldState;

    bool m_shouldDrawMouseTargetLines;

    juce::uint32 m_lastClipUpdate = 0;
};
