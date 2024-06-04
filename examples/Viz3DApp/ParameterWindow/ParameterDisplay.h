#pragma once

// Plugin
#include "../Parameters.h"

// GLM
#include <glm/glm.hpp>

// Stdlib
#include <array>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct ParameterType
{
    enum Type
    {
        SLIDER,
        COLOR,
        TOGGLE,
        BUTTON,
        COMBO_BOX,
    };

    struct SliderRange
    {
        float minimum = 0;
        float maximum = 1;
        float interval = 0.1;
    };

    struct ComboBox
    {
        std::vector<std::string> options;
    };

    Type type{ ParameterType::Type::SLIDER };
    std::variant<void*, SliderRange, ComboBox> value{ nullptr };
};

static const std::vector<std::pair<std::string, std::vector<std::pair<std::string, ParameterType>>>> PARAMETER_ORDERED_DISPLAY_NAMES = { {
  { "spectrogram",
    {
      { "min_desired_frequency", { ParameterType::SLIDER, ParameterType::SliderRange{ 100, 24000, 100 } } },
      { "max_desired_frequency", { ParameterType::SLIDER, ParameterType::SliderRange{ 100, 24000, 100 } } },
      { "min_db", { ParameterType::SLIDER, ParameterType::SliderRange{ -80, -1, 1 } } },
      { "max_db", { ParameterType::SLIDER, ParameterType::SliderRange{ -80, -1, 1 } } },
      { "attack_seconds", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 2, 0.001f } } },
      { "release_seconds", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 2, 0.001f } } },
    } },
  { "visual",
    {
      { "visual", { ParameterType::Type::COMBO_BOX, ParameterType::ComboBox{ { "1", "2", "3" } } } },
    } },
  { "visual 1",
    {
      { "width", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 5, 0.25 } } },
      { "length", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 5, 0.25 } } },
      { "height", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 5, 0.25 } } },
      { "global_scale", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 2.5, 0.1 } } },
      { "y_displacement", { ParameterType::SLIDER, ParameterType::SliderRange{ -2.5, 2.5, 0.1 } } },
      { "num_lines", { ParameterType::SLIDER, ParameterType::SliderRange{ 1, 100, 1 } } },
      { "line_thickness", { ParameterType::SLIDER, ParameterType::SliderRange{ 0.001, 0.1, 0.001 } } },
      { "line_color_1", { ParameterType::COLOR } },
      { "line_color_2", { ParameterType::COLOR } },
      { "background_color", { ParameterType::COLOR } },
      { "gradient_position", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 1, 0.05 } } },
      { "gradient_intensity", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 1, 0.05 } } },
    } },
  { "visual 2",
    {
      { "2_line_color_1", { ParameterType::COLOR } },
      { "2_line_color_2", { ParameterType::COLOR } },
      { "2_background_color", { ParameterType::COLOR } },
      { "2_gradient_position", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 1, 0.05 } } },
      { "2_gradient_intensity", { ParameterType::SLIDER, ParameterType::SliderRange{ 0, 1, 0.05 } } },
    } },
  { "visual 3",
    {
      { "3_background_color", { ParameterType::COLOR } },
      { "3_x_amount", { ParameterType::SLIDER, ParameterType::SliderRange{ 1, 50, 1 } } },
      { "3_z_amount", { ParameterType::SLIDER, ParameterType::SliderRange{ 1, 50, 1 } } },
    } },
} };

float
getSliderValue(const Parameters& parameters, const std::string& name);

glm::vec3
getColorValue(const Parameters& parameters, const std::string& name);

bool
getToggleValue(const Parameters& parameters, const std::string& name);

unsigned int
getComboBoxValue(const Parameters& parameters, const std::string& name);

void
setSliderValue(Parameters& parameters, const std::string& name, float value);

void
setColorValue(Parameters& parameters, const std::string& name, const glm::vec3& value);

void
setToggleValue(Parameters& parameters, const std::string& name, bool value);

void
setButtonValue(Parameters& parameters, const std::string& name);

void
setComboBoxValue(Parameters& parameters, const std::string& name, unsigned int value);