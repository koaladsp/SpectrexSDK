#version 410 core

out vec4 FragColor;

in GeometryData
{
    flat int InstanceID;
}
In;

uniform int uXAmount;
uniform int uZAmount;

void main() {
    vec3 color;

    vec3 c1 = vec3(178, 93, 166) / 255.f;
    vec3 c2 = vec3(102, 136, 195) / 255.f;
    vec3 c3 = vec3(72, 165, 106) / 255.f;
    vec3 c4 = vec3(234, 175, 65) / 255.f;
    vec3 c5 = vec3(206, 74, 74) / 255.f;

    int zNr = In.InstanceID / uXAmount;
    int colorNr = zNr % 5;

    if (colorNr == 0) {
        color = c1;
    } else if (colorNr == 1) {
        color = c2;
    } else if (colorNr == 2) {
        color = c3;
    } else if (colorNr == 3) {
        color = c4;
    } else if (colorNr == 4) {
        color = c5;
    }

    FragColor = vec4(color, 1.0);
}
