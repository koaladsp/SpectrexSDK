#include "ParameterDisplay.h"

// Plugin
#include "../Utility.h"

float
getSliderValue(const Parameters& parameters, const std::string& name)
{
    if (name == "min_frequency") {
        return parameters.min_frequency;
    } else if (name == "max_frequency") {
        return parameters.max_frequency;
    } else if (name == "min_db") {
        return parameters.min_db;
    } else if (name == "max_db") {
        return parameters.max_db;
    }

    return 0;
}

glm::vec3
getColorValue(const Parameters& parameters, const std::string& name)
{
    return {};
}

bool
getToggleValue(const Parameters& parameters, const std::string& name)
{
    if (name == "pause") {
        return parameters.pause;
    }

    return false;
}

unsigned int
getComboBoxValue(const Parameters& parameters, const std::string& name)
{
    if (name == "window") {
        switch (parameters.window) {
            case spectrex::Window::WindowNone:
                return 0;
            case spectrex::Window::WindowHann:
                return 1;
            case spectrex::Window::WindowBlackman:
                return 2;
            default:
                break;
        }
    } else if (name == "stft_overlap") {
        if (approximatelyEqual(parameters.stft_overlap, 1.0f / 2.0f)) {
            return 0;
        } else if (approximatelyEqual(parameters.stft_overlap, 3.0f / 4.0f)) {
            return 1;
        } else if (approximatelyEqual(parameters.stft_overlap, 7.0f / 8.0f)) {
            return 2;
        }
    } else if (name == "time_multiplier") {
        if (approximatelyEqual(parameters.time_multiplier, 1.0f)) {
            return 0;
        } else if (approximatelyEqual(parameters.time_multiplier, 2.0f)) {
            return 1;
        } else if (approximatelyEqual(parameters.time_multiplier, 4.0f)) {
            return 2;
        } else if (approximatelyEqual(parameters.time_multiplier, 8.0f)) {
            return 3;
        } else if (approximatelyEqual(parameters.time_multiplier, 16.0f)) {
            return 4;
        }
    } else if (name == "mix") {
#ifdef ENHANCED_FEATURES
        // @enhanced
        switch (parameters.mix) {
            case spectrex::MixMode::Stereo:
                return 0;
            case spectrex::MixMode::Left:
                return 1;
            case spectrex::MixMode::Right:
                return 2;
            case spectrex::MixMode::Mid:
                return 3;
            case spectrex::MixMode::Side:
                return 4;
            default:
                break;
        }
#else
        return 0;
#endif
    } else if (name == "ft_size") {
#ifdef ENHANCED_FEATURES
        // @enhanced
        switch (parameters.ft_size) {
            case spectrex::FtSize::Size256:
                return 0;
            case spectrex::FtSize::Size512:
                return 1;
            case spectrex::FtSize::Size1024:
                return 2;
            case spectrex::FtSize::Size2048:
                return 3;
            case spectrex::FtSize::Size4096:
                return 4;
            case spectrex::FtSize::Size8192:
                return 5;
            default:
                break;
        }
#else
        switch (parameters.ft_size) {
            case spectrex::FtSize::Size256:
                return 0;
            case spectrex::FtSize::Size512:
                return 1;
            default:
                break;
        }
#endif
    }

    return 0;
}

void
setSliderValue(Parameters& parameters, const std::string& name, float value)
{
    if (name == "min_frequency") {
        parameters.min_frequency = value;
    } else if (name == "max_frequency") {
        parameters.max_frequency = value;
    } else if (name == "min_db") {
        parameters.min_db = value;
    } else if (name == "max_db") {
        parameters.max_db = value;
    }
    parameters.onParameterChanged(name);
}

void
setColorValue(Parameters& parameters, const std::string& name, const glm::vec3& value)
{
    parameters.onParameterChanged(name);
}

void
setToggleValue(Parameters& parameters, const std::string& name, bool value)
{
    if (name == "pause") {
        parameters.pause = value;
    }
    parameters.onParameterChanged(name);
}

void
setButtonValue(Parameters& parameters, const std::string& name)
{
    parameters.onParameterChanged(name);
}

void
setComboBoxValue(Parameters& parameters, const std::string& name, unsigned int value)
{
    if (name == "window") {
        switch (value) {
            case 0:
                parameters.window = spectrex::Window::WindowNone;
                break;
            case 1:
                parameters.window = spectrex::Window::WindowHann;
                break;
            case 2:
                parameters.window = spectrex::Window::WindowBlackman;
                break;
            default:
                break;
        }
    } else if (name == "stft_overlap") {
        switch (value) {
            case 0: // 1/2
                parameters.stft_overlap = 1.0f / 2.0f;
                break;
            case 1: // 3/4
                parameters.stft_overlap = 3.0f / 4.0f;
                break;
            case 2: // 7/8
                parameters.stft_overlap = 7.0f / 8.0f;
                break;
            default:
                break;
        }
    } else if (name == "time_multiplier") {
        switch (value) {
            case 0: // 1
                parameters.time_multiplier = 1;
                break;
            case 1: // 2
                parameters.time_multiplier = 2;
                break;
            case 2: // 4
                parameters.time_multiplier = 4;
                break;
            case 3: // 8
                parameters.time_multiplier = 8;
                break;
            case 4: // 16
                parameters.time_multiplier = 16;
                break;
            default:
                break;
        }
    } else if (name == "mix") {
#ifdef ENHANCED_FEATURES
        // @enhanced
        switch (value) {
            case 0: // Stereo
                parameters.mix = spectrex::MixMode::Stereo;
                break;
            case 1: // Left
                parameters.mix = spectrex::MixMode::Left;
                break;
            case 2: // Right
                parameters.mix = spectrex::MixMode::Right;
                break;
            case 3: // Mid
                parameters.mix = spectrex::MixMode::Mid;
                break;
            case 4: // Side
                parameters.mix = spectrex::MixMode::Side;
                break;
            default:
                break;
        }
#else
        switch (value) {
            case 0: // Mid
                parameters.mix = spectrex::MixMode::Mid;
                break;
            default:
                break;
        }
#endif
    } else if (name == "ft_size") {
#ifdef ENHANCED_FEATURES
        // @enhanced
        switch (value) {
            case 0: // 256
                parameters.ft_size = spectrex::FtSize::Size256;
                break;
            case 1: // 512
                parameters.ft_size = spectrex::FtSize::Size512;
                break;
            default:
                break;
        }
#else
        switch (value) {
            case 0: // 256
                parameters.ft_size = spectrex::FtSize::Size256;
                break;
            case 1: // 512
                parameters.ft_size = spectrex::FtSize::Size512;
                break;
            case 2: // 1024
                parameters.ft_size = spectrex::FtSize::Size1024;
                break;
            case 3: // 2048
                parameters.ft_size = spectrex::FtSize::Size2048;
                break;
            case 4: // 4096
                parameters.ft_size = spectrex::FtSize::Size4096;
                break;
            case 5: // 8192
                parameters.ft_size = spectrex::FtSize::Size8192;
                break;
            default:
                break;
        }
#endif
    }
    parameters.onParameterChanged(name);
}
