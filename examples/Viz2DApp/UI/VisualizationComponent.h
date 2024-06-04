#pragma once

// Plugin
#include "../Parameters.h"
#include "../WindowOpenGLContext.h"

// Spectrex
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

class Component;
class PluginAudioProcessor;

class VisualizationComponent final
  : public juce::Component
  , public juce::OpenGLRenderer
  , public juce::KeyListener
  , public juce::Timer
  , public juce::Button::Listener
  , public Parameters::Listener
{
  public:
    enum class Type
    {
        Waveform,
        Spectrogram,
    };

    // Constants
    const juce::Array<int> k_FreqsToMap{
        30, 40, 50, 60, 80, 100, 200, 300, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000, 20000
    };
    const float k_barStrokeWidth = 2.0f;      // px
    const float k_beatStrokeWidth = 1.0f;     // px
    const float k_dbMarkerStrokeWidth = 0.5f; // px

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
    void timerCallback() override;

    /// @brief Get the ppq of this component that was most recently drawn.
    /// @return ppq.
    auto getPpqLastDrawn() const noexcept -> float;

    /// @brief Called when any key is pressed.
    /// @param key Key pressed.
    /// @param originatingComponent The component that received the key event.
    /// @return True when consumed, otherwise false.
    auto keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) -> bool override;

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

    VisualizationComponent(utility::WindowOpenGLContext& context, PluginAudioProcessor& processor, Parameters& parameters, Type type);
    virtual ~VisualizationComponent();

  private:
    /// @brief Call when recreating components to synchronize state
    void initialUpdate();

    /// @brief Called when a parameter is updated.
    void parameterChanged(const Parameters& parameters, const std::string& name) noexcept override;

    /// @brief Recalculate clipping boundaries, essential to get properly DPI
    /// scaled render target.
    void updateClippingBounds();

  private:
    // Component type
    const Type m_type;

    juce::Rectangle<int> m_clippingBounds;

    utility::WindowOpenGLContext& m_openGLContext;
    PluginAudioProcessor& m_pluginProcessor;
    spectrex::KProcessor& m_processor;
    Parameters& m_parameters;

    std::unique_ptr<spectrex::KComponent> m_component;

    bool m_shouldDrawMouseTargetLines;

    juce::uint32 m_lastClipUpdate = 0;
};
