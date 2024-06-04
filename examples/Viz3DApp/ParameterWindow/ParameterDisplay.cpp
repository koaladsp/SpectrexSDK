#include "ParameterDisplay.h"

float
getSliderValue(const Parameters& parameters, const std::string& name)
{
    if (name == "min_desired_frequency") {
        return parameters.min_desired_frequency;
    } else if (name == "max_desired_frequency") {
        return parameters.max_desired_frequency;
    } else if (name == "min_db") {
        return parameters.min_db;
    } else if (name == "max_db") {
        return parameters.max_db;
    } else if (name == "attack_seconds") {
        return parameters.attack_seconds;
    } else if (name == "release_seconds") {
        return parameters.release_seconds;
    } else if (name == "width") {
        return parameters.width;
    } else if (name == "length") {
        return parameters.length;
    } else if (name == "height") {
        return parameters.height;
    } else if (name == "global_scale") {
        return parameters.global_scale;
    } else if (name == "y_displacement") {
        return parameters.y_displacement;
    } else if (name == "num_lines") {
        return parameters.num_lines;
    } else if (name == "line_thickness") {
        return parameters.line_thickness;
    } else if (name == "gradient_position") {
        return parameters.gradient_position;
    } else if (name == "gradient_intensity") {
        return parameters.gradient_intensity;
    } else if (name == "2_gradient_position") {
        return parameters.visual_2.gradient_position;
    } else if (name == "2_gradient_intensity") {
        return parameters.visual_2.gradient_intensity;
    } else if (name == "3_x_amount") {
        return parameters.visual_3.x_amount;
    } else if (name == "3_z_amount") {
        return parameters.visual_3.z_amount;
    }

    return 0;
}

glm::vec3
getColorValue(const Parameters& parameters, const std::string& name)
{
    if (name == "line_color_1") {
        return parameters.color_1;
    } else if (name == "line_color_2") {
        return parameters.color_2;
    } else if (name == "background_color") {
        return parameters.background_color;
    } else if (name == "2_line_color_1") {
        return parameters.visual_2.color_1;
    } else if (name == "2_line_color_2") {
        return parameters.visual_2.color_2;
    } else if (name == "2_background_color") {
        return parameters.visual_2.background_color;
    } else if (name == "3_background_color") {
        return parameters.visual_3.background_color;
    }

    return {};
}

bool
getToggleValue(const Parameters& parameters, const std::string& name)
{
    if (name == "disable_msaa") {
        return parameters.disable_msaa;
    }

    return false;
}

unsigned int
getComboBoxValue(const Parameters& parameters, const std::string& name)
{
    if (name == "visual") {
        return parameters.visual;
    }

    return 0;
}

void
setSliderValue(Parameters& parameters, const std::string& name, float value)
{
    if (name == "min_desired_frequency") {
        parameters.min_desired_frequency = value;
    } else if (name == "max_desired_frequency") {
        parameters.max_desired_frequency = value;
    } else if (name == "min_db") {
        parameters.min_db = value;
    } else if (name == "max_db") {
        parameters.max_db = value;
    } else if (name == "attack_seconds") {
        parameters.attack_seconds = value;
    } else if (name == "release_seconds") {
        parameters.release_seconds = value;
    } else if (name == "width") {
        parameters.width = value;
    } else if (name == "length") {
        parameters.length = value;
    } else if (name == "height") {
        parameters.height = value;
    } else if (name == "global_scale") {
        parameters.global_scale = value;
    } else if (name == "y_displacement") {
        parameters.y_displacement = value;
    } else if (name == "num_lines") {
        parameters.num_lines = value;
    } else if (name == "line_thickness") {
        parameters.line_thickness = value;
    } else if (name == "gradient_position") {
        parameters.gradient_position = value;
    } else if (name == "gradient_intensity") {
        parameters.gradient_intensity = value;
    } else if (name == "2_gradient_position") {
        parameters.visual_2.gradient_position = value;
    } else if (name == "2_gradient_intensity") {
        parameters.visual_2.gradient_intensity = value;
    } else if (name == "3_x_amount") {
        parameters.visual_3.x_amount = value;
    } else if (name == "3_z_amount") {
        parameters.visual_3.z_amount = value;
    }
}

void
setColorValue(Parameters& parameters, const std::string& name, const glm::vec3& value)
{
    if (name == "line_color_1") {
        parameters.color_1 = value;
    } else if (name == "line_color_2") {
        parameters.color_2 = value;
    } else if (name == "background_color") {
        parameters.background_color = value;
    } else if (name == "2_line_color_1") {
        parameters.visual_2.color_1 = value;
    } else if (name == "2_line_color_2") {
        parameters.visual_2.color_2 = value;
    } else if (name == "2_background_color") {
        parameters.visual_2.background_color = value;
    } else if (name == "3_background_color") {
        parameters.visual_3.background_color = value;
    }
}

void
setToggleValue(Parameters& parameters, const std::string& name, bool value)
{
    if (name == "disable_msaa") {
        parameters.disable_msaa = value;
    }
}

void
setButtonValue(Parameters& parameters, const std::string& name)
{
}

void
setComboBoxValue(Parameters& parameters, const std::string& name, unsigned int value)
{
    if (name == "visual") {
        parameters.visual = value;
    }
}