#pragma once

// Plugin
#include "ParameterDisplay.h"

// JUCE
#include "juce_gui_basics/juce_gui_basics.h"
#include <juce_opengl/juce_opengl.h>

/* Forward Declarations */

class ParameterWindow;
class ColorParameter;
class SliderParameter;

class SliderParameter : public juce::Slider
{
  public:
    SliderParameter(std::string name, const ParameterType::SliderRange& range);

    void setParameterValue(Parameters& parameters) noexcept;

    void syncParameterValue(const Parameters& parameters) noexcept;

  private:
    std::string m_name;
};

/* ColorPicker */

class ColorPicker final : public juce::ColourSelector
{
  public:
    ColorPicker(ColorParameter& parent, int flags, std::string name) noexcept;

    void setParameterValue(Parameters& parameters) noexcept;

  private:
    ColorParameter& m_parent;
    std::string m_name;
};

/* ColorParameter */

class ColorParameter : public juce::ShapeButton
{
  public:
    ColorParameter(ParameterWindow* window, std::string name) noexcept;

    void syncParameterValue(const Parameters& parameters) noexcept;

    void clicked() override;

  private:
    ParameterWindow* m_window;

    std::string m_name;

    juce::Colour m_currentColor;
};

class ToggleParameter : public juce::TextButton
{
  public:
    ToggleParameter(std::string name);

    void setParameterValue(Parameters& parameters) noexcept;

    void syncParameterValue(const Parameters& parameters) noexcept;

    void clicked() override;

  private:
    std::string m_name;
};

class ComboBoxParameter : public juce::ComboBox
{
  public:
    ComboBoxParameter(std::string name, const ParameterType::ComboBox& comboBox);

    void setParameterValue(Parameters& parameters) noexcept;

    void syncParameterValue(const Parameters& parameters) noexcept;

  private:
    std::string m_name;
};

class ButtonParameter : public juce::TextButton
{
  public:
    ButtonParameter(std::string name);

    void setParameterValue(Parameters& parameters) noexcept;

  private:
    std::string m_name;
};

/* ParameterWindow */

class ParameterWindow final
  : public juce::DocumentWindow
  , public juce::Slider::Listener
  , public juce::Button::Listener
  , public juce::ComboBox::Listener
  , public juce::ChangeListener
  , public juce::KeyListener
{
  private:
    class ParameterWindowComponent final : public juce::Component
    {
      public:
        ParameterWindowComponent(ParameterWindow* window, const Parameters& parameters);

        void syncAll() noexcept;

        void paint(juce::Graphics& graphics) override;

      private:
        const Parameters& m_parameters;

        /* Parameter Components */

        std::vector<std::unique_ptr<juce::Label>> m_sectionLabels;
        std::vector<std::unique_ptr<juce::Label>> m_parameterLabels;

        std::vector<std::unique_ptr<SliderParameter>> m_sliderParameters;
        std::vector<std::unique_ptr<ColorParameter>> m_colorParameters;
        std::vector<std::unique_ptr<ToggleParameter>> m_toggleParameters;
        std::vector<std::unique_ptr<ButtonParameter>> m_buttonParameters;
        std::vector<std::unique_ptr<ComboBoxParameter>> m_comboBoxParameters;
    };

  public:
    ParameterWindow(Parameters& parameters);

    void sliderValueChanged(juce::Slider* slider) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void buttonClicked(juce::Button* source) override;

    void comboBoxChanged(juce::ComboBox* source) override;

    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

  private:
    Parameters& m_parameters;

    ParameterWindowComponent* m_parameterWindowComponent;
    juce::Viewport* m_viewportComponent;
};
