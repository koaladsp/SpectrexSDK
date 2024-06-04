#include "ParameterWindow.h"

// Stdlib
#include <fstream>
#include <utility>

SliderParameter::SliderParameter(std::string name, const ParameterType::SliderRange& range)
  : m_name(std::move(name))
{
    setRange(range.minimum, range.maximum, range.interval);
    setVelocityBasedMode(true);
}

void
SliderParameter::setParameterValue(Parameters& parameters) noexcept
{
    setSliderValue(parameters, m_name, (float)getValue());
}

void
SliderParameter::syncParameterValue(const Parameters& parameters) noexcept
{
    setValue(getSliderValue(parameters, m_name));
}

ColorPicker::ColorPicker(ColorParameter& parent, int flags, std::string name) noexcept
  : juce::ColourSelector(flags)
  , m_parent(parent)
  , m_name(std::move(name))
{
}

void
ColorPicker::setParameterValue(Parameters& parameters) noexcept
{
    const auto currentColor = getCurrentColour();

    glm::vec3 color;
    color.x = currentColor.getFloatRed();
    color.y = currentColor.getFloatGreen();
    color.z = currentColor.getFloatBlue();

    setColorValue(parameters, m_name, color);

    m_parent.syncParameterValue(parameters);
    m_parent.repaint();
}

ColorParameter::ColorParameter(ParameterWindow* window, std::string name) noexcept
  : juce::ShapeButton(name, juce::Colours::white, juce::Colours::white, juce::Colours::white)
  , m_window(window)
  , m_name(std::move(name))
  , m_currentColor(juce::Colour::fromFloatRGBA(0, 0, 0, 1.0f))
{
    auto j_path = juce::Path();
    j_path.addRectangle(0, 0, 1.f, 1.f);
    setShape(j_path, true, false, false);
    setOutline(juce::Colours::grey, 2);
}

void
ColorParameter::syncParameterValue(const Parameters& parameters) noexcept
{
    glm::vec3 color = getColorValue(parameters, m_name);
    m_currentColor = juce::Colour::fromFloatRGBA(color.x, color.y, color.z, 1.0f);
    setColours(m_currentColor, m_currentColor, m_currentColor);
}

void
ColorParameter::clicked()
{
    auto colourSelector = std::make_unique<ColorPicker>(*this,
                                                        juce::ColourSelector::showColourAtTop | juce::ColourSelector::editableColour |
                                                          juce::ColourSelector::showSliders | juce::ColourSelector::showColourspace,
                                                        m_name);

    colourSelector->setName(m_name);
    colourSelector->setCurrentColour(m_currentColor);
    colourSelector->addChangeListener(m_window);
    colourSelector->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
    colourSelector->setSize(300, 400);

    juce::CallOutBox::launchAsynchronously(std::move(colourSelector), getScreenBounds(), nullptr);
}

ToggleParameter::ToggleParameter(std::string name)
  : juce::TextButton("")
  , m_name(std::move(name))
{
    setClickingTogglesState(true);

    setToggleState(false, juce::dontSendNotification);
    clicked();
}

void
ToggleParameter::setParameterValue(Parameters& parameters) noexcept
{
    setToggleValue(parameters, m_name, getToggleState());
}

void
ToggleParameter::syncParameterValue(const Parameters& parameters) noexcept
{
    setToggleState(getToggleValue(parameters, m_name), juce::dontSendNotification);
    clicked();
}

void
ToggleParameter::clicked()
{
    setButtonText(getToggleState() ? "On" : "Off");
}

ComboBoxParameter::ComboBoxParameter(std::string name, const ParameterType::ComboBox& comboBox)
  : m_name(std::move(name))
{
    for (int i = 0; i < comboBox.options.size(); i++) {
        addItem(comboBox.options[i], i + 1);
    }
    setSelectedId(1, juce::dontSendNotification);
}

void
ComboBoxParameter::setParameterValue(Parameters& parameters) noexcept
{
    setComboBoxValue(parameters, m_name, getSelectedId() - 1);
}

void
ComboBoxParameter::syncParameterValue(const Parameters& parameters) noexcept
{
    setSelectedId((int)getComboBoxValue(parameters, m_name) + 1, juce::dontSendNotification);
}

ButtonParameter::ButtonParameter(std::string name)
  : juce::TextButton("")
  , m_name(std::move(name))
{
}

void
ButtonParameter::setParameterValue(Parameters& parameters) noexcept
{
    setButtonValue(parameters, m_name);
}

juce::Rectangle<int>
addToBottom(juce::Rectangle<int>& in, int amount)
{
    int in_height = in.getHeight();
    in.expand(0, amount);
    juce::Rectangle<int> out = in.removeFromTop(amount);

    out.setPosition(in.getPosition().x, in.getPosition().y + in_height);
    return out;
}

ParameterWindow::ParameterWindowComponent::ParameterWindowComponent(ParameterWindow* window, const Parameters& parameters)
  : m_parameters(parameters)
{
    setAlwaysOnTop(true);
    auto area = juce::Rectangle<int>(500 - 10, 0);

    for (const auto& section : PARAMETER_ORDERED_DISPLAY_NAMES) {
        // section label
        m_sectionLabels.push_back(std::make_unique<juce::Label>());

        m_sectionLabels.back()->setText(section.first, juce::dontSendNotification);
        m_sectionLabels.back()->setJustificationType(juce::Justification::centredLeft);

        m_sectionLabels.back()->setBounds(addToBottom(area, 30));
        addAndMakeVisible(*m_sectionLabels.back());

        area.removeFromLeft(25);

        for (const auto& parameter : section.second) {

            auto parameter_area = addToBottom(area, 20);

            // parameter label
            m_parameterLabels.push_back(std::make_unique<juce::Label>());

            m_parameterLabels.back()->setText(parameter.first, juce::dontSendNotification);
            m_parameterLabels.back()->setJustificationType(juce::Justification::centredLeft);

            m_parameterLabels.back()->setBounds(parameter_area.removeFromLeft(150));
            addAndMakeVisible(*m_parameterLabels.back());

            if (parameter.second.type == ParameterType::SLIDER) {
                auto slider_range = std::get_if<ParameterType::SliderRange>(&parameter.second.value);
                if (slider_range) {
                    m_sliderParameters.push_back(std::make_unique<SliderParameter>(parameter.first, *slider_range));
                }

                m_sliderParameters.back()->addListener(window);
                m_sliderParameters.back()->syncParameterValue(m_parameters);

                m_sliderParameters.back()->setBounds(parameter_area);
                addAndMakeVisible(*m_sliderParameters.back());

            } else if (parameter.second.type == ParameterType::COLOR) {
                m_colorParameters.push_back(std::make_unique<ColorParameter>(window, parameter.first));

                m_colorParameters.back()->setButtonText(parameter.first);
                m_colorParameters.back()->syncParameterValue(m_parameters);

                m_colorParameters.back()->setBounds(parameter_area.removeFromLeft(100));
                addAndMakeVisible(*m_colorParameters.back());
            } else if (parameter.second.type == ParameterType::TOGGLE) {
                m_toggleParameters.push_back(std::make_unique<ToggleParameter>(parameter.first));

                m_toggleParameters.back()->addListener(window);
                m_toggleParameters.back()->syncParameterValue(m_parameters);

                m_toggleParameters.back()->setBounds(parameter_area.removeFromLeft(100));
                addAndMakeVisible(*m_toggleParameters.back());
            } else if (parameter.second.type == ParameterType::BUTTON) {
                m_buttonParameters.push_back(std::make_unique<ButtonParameter>(parameter.first));

                m_buttonParameters.back()->addListener(window);

                m_buttonParameters.back()->setBounds(parameter_area.removeFromLeft(100));
                addAndMakeVisible(*m_buttonParameters.back());
            } else if (parameter.second.type == ParameterType::COMBO_BOX) {
                auto combo_box = std::get_if<ParameterType::ComboBox>(&parameter.second.value);
                if (combo_box) {
                    m_comboBoxParameters.push_back(std::make_unique<ComboBoxParameter>(parameter.first, *combo_box));
                }

                m_comboBoxParameters.back()->addListener(window);
                m_comboBoxParameters.back()->syncParameterValue(m_parameters);

                m_comboBoxParameters.back()->setBounds(parameter_area.removeFromLeft(200));
                addAndMakeVisible(*m_comboBoxParameters.back());
            }

            addToBottom(area, 5);
        }

        area.expand(25, 0);
        area.removeFromRight(25);
    }

    setBounds(area);
}

void
ParameterWindow::ParameterWindowComponent::syncAll() noexcept
{
    for (const auto& sliderParameter : m_sliderParameters) {
        sliderParameter->syncParameterValue(m_parameters);
    }

    for (const auto& colorParameter : m_colorParameters) {
        colorParameter->syncParameterValue(m_parameters);
    }

    for (const auto& toggleParameter : m_toggleParameters) {
        toggleParameter->syncParameterValue(m_parameters);
    }

    for (const auto& comboBoxParameter : m_comboBoxParameters) {
        comboBoxParameter->syncParameterValue(m_parameters);
    }
}

void
ParameterWindow::ParameterWindowComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colours::darkgrey);
}

ParameterWindow::ParameterWindow(Parameters& parameters)
  : juce::DocumentWindow("Parameters", juce::Colours::darkgrey, 0)
  , m_parameters(parameters)
{
    setSize(500, 600);

    setUsingNativeTitleBar(true);

    m_parameterWindowComponent = new ParameterWindowComponent{ this, m_parameters };

    m_viewportComponent = new juce::Viewport();

    m_viewportComponent->setViewedComponent(m_parameterWindowComponent, true);
    m_viewportComponent->setBounds(getBounds());

    setContentOwned(m_viewportComponent, true);

    setCentreRelative(0.75f, 0.5f);
    setAlwaysOnTop(true);

    setVisible(true);
    setResizable(false, false);

    addKeyListener(this);
    setWantsKeyboardFocus(true);

    toFront(false);
}

void
ParameterWindow::sliderValueChanged(juce::Slider* slider)
{
    if (auto parameterSlider = dynamic_cast<SliderParameter*>(slider)) {
        parameterSlider->setParameterValue(m_parameters);
    } else {
        jassertfalse; // Should only use SliderParameter
    }
}

void
ParameterWindow::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (auto* colorSelector = dynamic_cast<ColorPicker*>(source)) {
        colorSelector->setParameterValue(m_parameters);
    } else {
        jassertfalse; // Should only use ColorParameter
    }
}

void
ParameterWindow::buttonClicked(juce::Button* source)
{
    if (auto* toggleParameter = dynamic_cast<ToggleParameter*>(source)) {
        toggleParameter->setParameterValue(m_parameters);
    } else if (auto* buttonParameter = dynamic_cast<ButtonParameter*>(source)) {
        buttonParameter->setParameterValue(m_parameters);
    } else {
        jassertfalse; // Should only use ToggleParameter
    }
}

void
ParameterWindow::comboBoxChanged(juce::ComboBox* source)
{
    if (auto* comboBoxParameter = dynamic_cast<ComboBoxParameter*>(source)) {
        comboBoxParameter->setParameterValue(m_parameters);
    } else {
        jassertfalse; // Should only use ColorParameter
    }
}

bool
ParameterWindow::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    return false;
}