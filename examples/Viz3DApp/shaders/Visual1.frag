#version 410 core

in VertexData
{
    vec2 TexCoord;
    flat int InstanceID;
    float SpectrogramValue;
}
In;

out vec4 FragColor;

uniform vec3 uLineColor1;
uniform vec3 uLineColor2;

uniform float uGradientPosition;
uniform float uGradientIntensity;

vec3 gradient(vec3 col1, vec3 col2, float x, float pos, float intensity) {
    return mix(col1, col2, smoothstep(pos - (0.5 * intensity), pos + (0.5 * intensity), x));
}

void
main()
{
    FragColor = vec4(gradient(uLineColor1, uLineColor2, In.SpectrogramValue, uGradientPosition, uGradientIntensity), 1.0);
}