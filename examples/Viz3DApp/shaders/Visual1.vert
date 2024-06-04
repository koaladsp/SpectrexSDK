#version 410

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

out VertexData
{
    vec2 TexCoord;
    flat int InstanceID;
    float SpectrogramValue;
}
Out;

// Spectrogram texture
uniform sampler2D uSpectrogram;

// Number of visible rows in the spectrogram (may be less, e.g. half of the actual texture size)
uniform int uSpectrogramRows;

// Row index of the latest row in the spectrogram
uniform int uSpectrogramLatestRow;

// Selected frequency range
uniform float uMinDesiredFrequency;
uniform float uMaxDesiredFrequency;

// Selected dB range
uniform float uMinDb;
uniform float uMaxDb;

// Frequency limits
uniform float uMinFrequency;
uniform float uMaxFrequency;

// Spectrum value height in terms of line vertical coordinates
uniform float uLineSpectrumHeight;
uniform float uLineThickness;

uniform mat4 uViewProjection;

uniform float uWidth;
uniform float uLength;
uniform float uGlobalScale;
uniform float uYDisplacement;
uniform int uNumInstances;

mat4
translate(vec3 d)
{
    // clang-format off
    return mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    d, 1.0
    );
    // clang-format on
}

mat4
scale(vec3 c)
{
    // clang-format off
    return mat4(
    c.x, 0.0, 0.0, 0.0,
    0.0, c.y, 0.0, 0.0,
    0.0, 0.0, c.z, 0.0,
    0.0, 0.0, 0.0, 1.0
    );
    // clang-format on
}

// 1 / log(10)
#define I_LOG10 0.43429448190325182765

float log_y(float y)
{
    float minf = I_LOG10 * log(uMinDesiredFrequency / uMaxFrequency);
    float maxf = I_LOG10 * log(uMaxDesiredFrequency / uMaxFrequency);

    return pow(10.0f, mix(minf, maxf, y));
}

float db(float mag)
{
    return 20.0f * I_LOG10 * log(mag);
}

// Cheap window tapering function [0, 1]
float taper(float t)
{
    float c = 10.0f;
    float x = (t < 0.5f) ? t : 1.0f - t; // Mirror halfway through
    float y = clamp(1.0f - (x * c), 0.0f, 1.0f);
    return clamp(1.0f - (y * y), 0.0f, 1.0f);
}

void
main()
{
    Out.TexCoord = aTexCoord;
    Out.InstanceID = gl_InstanceID;

    vec3 Position = aPosition;

    // Scale line thickness properly
    Position.y *= uLineThickness;

    mat4 scaledModel =
    scale(vec3(uGlobalScale) * vec3(uWidth, 1, 1))
    * translate(vec3(0, uYDisplacement, 0))
    * translate(vec3(0, 0, uLength * ((float(gl_InstanceID) / float(uNumInstances - 1)) - 0.5)));

    vec4 scaledPosition = scaledModel * vec4(Position, 1.0);

    // spectrex
    {
        // Total spectrogram texture data size
        int height = textureSize(uSpectrogram, 0).y;
        int bins = textureSize(uSpectrogram, 0).x;

        // Initial bin magnitude value
        float magnitude = 0.0f;

        // Spectrogram bin coordinate
        // Bins values are logarithmic in a linear array, so scale y logarithmically to correspond with a log frequency spectrum from low to high
        float bin = log_y(aTexCoord.x);
        bin = clamp(bin, 0.0f, 1.0f);

        // Number of spectrogram rows to accumulate per instance
        int numRowsPerInstance = int(float(uSpectrogramRows) / float(uNumInstances));

        // Row index of the exact row in the window that we want to map to this primitive
        // Note that the entire window that we're showing runs from [uSpectrogramLatestRow - uSpectrogramRows, uSpectrogramLatestRow]
        int r = uSpectrogramLatestRow - uSpectrogramRows + (gl_InstanceID * numRowsPerInstance);

        // The spectrogram resolution is likely higher than the number of primitives that we are plotting
        // Sum and average our bin(s) of interest from all necessary rows inside the spectrogram "in between" the primitives
        // This is in fact a LOD operation to make sure no information is lost when mapping to our primitives
        for(int dr = 0; dr < numRowsPerInstance; ++dr) {
            int sr = r + dr;

            // Make sure the row coordinate wraps around and stays within bounds
            if (sr < 0) { sr += height; }
            else if (sr >= height) { sr -= height; }

            // Sample bin from texture
            magnitude += texture(uSpectrogram, vec2(bin * float(bins), float(sr)) / vec2(bins, height)).r;
        }
        magnitude *= 1.0f / numRowsPerInstance;

        // Convert bin magnitude to dB
        float db = db(magnitude);
        float dbClamped = clamp(db, uMinDb, uMaxDb);
        float value = (uMaxDb != uMinDb) ? (dbClamped - uMinDb) / (uMaxDb - uMinDb) : 0.0f;

        // Make sure the line begins and ends at 0, this obviously messes up the accuracy of the spectrum values, but this is visual effect anyway...
        // We do this by windowing the value using a steep tapering window
        value *= taper(aTexCoord.x);
        Out.SpectrogramValue = value;

        // Displace Y vertex accordingly
        scaledPosition.y += value * uLineSpectrumHeight * uGlobalScale;
    }

    gl_Position = uViewProjection * scaledPosition;
}