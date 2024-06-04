#pragma once

// GLM
#include <glm/glm.hpp>

struct Parameters
{
    /* spectrogram */
    float min_desired_frequency = 200.f;
    float max_desired_frequency = 8000.f;

    float min_db = -48.f;
    float max_db = -6.f;

    float attack_seconds = 0.2f;
    float release_seconds = 0.2f;
    /* ----- */

    unsigned int visual = 0;

    /* visual 1 */
    // lines
    float width = 2.5f;
    float length = 3.f;
    float height = 1.5f;
    float global_scale = 1.f;
    float y_displacement = -0.2f;
    float num_lines = 40.f;
    float line_thickness = 0.025f;

    // camera
    float cam_angle = 20.f;
    float cam_zoom = -2.5f;

    // colors
    glm::vec3 color_1 = { 28.f / 255.f, 71.f / 255.f, 119.f / 255.f };
    glm::vec3 color_2 = { 130.f / 255.f, 183.f / 255.f, 33.f / 255.f };
    glm::vec3 background_color = { 18.f / 255.f, 19.f / 255.f, 20.f / 255.f };

    float gradient_position = 0.2f;
    float gradient_intensity = 0.5f;
    /* ----- */

    /* visual 2 */

    struct Visual2
    {
        float width = 2.5f;
        float length = 20.f;
        float height = 2.f;
        float num_lines = 100.f;
        float line_thickness = 0.025f;

        float y_displacement = -0.2f;

        glm::vec3 color_1 = { 125.f / 255.f, 29.f / 255.f, 169.f / 255.f };
        glm::vec3 color_2 = { 59.f / 255.f, 179.f / 255.f, 187.f / 255.f };
        glm::vec3 background_color = { 7.f / 255.f, 7.f / 255.f, 7.f / 255.f };

        float gradient_position = 0.2f;
        float gradient_intensity = 0.5f;
    } visual_2;

    /* ----- */

    /* visual 3 */

    struct Visual3
    {
        float x_amount = 25;
        float z_amount = 25;

        float base_height = 0.05f;
        float height = 1.f;

        glm::vec3 background_color = { 224.f / 255.f, 221.f / 255.f, 207.f / 255.f };
    } visual_3;

    /* ----- */

    // debug
    bool disable_msaa = false;
};