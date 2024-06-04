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
    std::variant<void*, SliderRange, ComboBox, bool> value{ nullptr };
};

static const std::vector<std::pair<std::string, std::vector<std::pair<std::string, ParameterType>>>> PARAMETER_ORDERED_DISPLAY_NAMES = { {
  { "spectrogram",
    {
      { "pause", { ParameterType::TOGGLE, false } },
      { "min_frequency", { ParameterType::SLIDER, ParameterType::SliderRange{ 100, 24000, 100 } } },
      { "max_frequency", { ParameterType::SLIDER, ParameterType::SliderRange{ 100, 24000, 100 } } },
      { "min_db", { ParameterType::SLIDER, ParameterType::SliderRange{ -80, -1, 1 } } },
      { "max_db", { ParameterType::SLIDER, ParameterType::SliderRange{ -80, -1, 1 } } },
      { "window", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "None", "Hann", "Blackman" } } } },
      { "stft_overlap", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "1/2", "3/4", "7/8" } } } },
      { "time_multiplier", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "1", "2", "4", "8", "16" } } } },

#ifdef ENHANCED_FEATURES
      // @enhanced
      { "mix", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "stereo", "left", "right", "mid", "side" } } } },
      { "ft_size", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "256", "512", "1024", "2048", "4096", "8192" } } } }
#else
      { "mix", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "mid" } } } },
      { "ft_size", { ParameterType::COMBO_BOX, ParameterType::ComboBox{ { "256", "512" } } } }
#endif
    } }
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