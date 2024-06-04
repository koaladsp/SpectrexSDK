#version 410

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

out VertexData
{
    int InstanceID;
    float ShapeWidth;
    float ShapeLength;
    float SpectrogramValue;
}
Out;

uniform int uXAmount;
uniform int uZAmount;

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

void main() {
    Out.InstanceID = gl_InstanceID;

    // positions in the grid
    int xNr = gl_InstanceID % uXAmount;
    int zNr = gl_InstanceID / uXAmount;

    // [0, 1) adds spacing to the outer edges, keeps all cubes in a 1x1 plane
    Out.ShapeWidth = 1.f / float(uXAmount);
    Out.ShapeLength = 1.f / float(uZAmount);

    float xPos = 0.5f - (Out.ShapeWidth) * float(xNr);
    float zPos = -0.5f + (Out.ShapeLength) * float(zNr);

    xPos -= Out.ShapeWidth / 2.f;
    zPos += Out.ShapeLength / 2.f;

    // spectrex
    {
        // Total spectrogram texture data size
        int height = textureSize(uSpectrogram, 0).y;
        int bins = textureSize(uSpectrogram, 0).x;

        // Initial bin magnitude value
        float magnitude = 0.0f;

        // Number of spectrogram rows to accumulate per instance
        int numRowsPerInstance = int(float(uSpectrogramRows) / float(uZAmount));

        // Number of bins per shape
        int numBinsPerShape = int(float(bins) / float(uXAmount));

        // Row index of the exact row in the window that we want to map to this primitive
        // Note that the entire window that we're showing runs from [uSpectrogramLatestRow - uSpectrogramRows, uSpectrogramLatestRow]
        int r = uSpectrogramLatestRow - uSpectrogramRows + (zNr * numRowsPerInstance);

        // The spectrogram resolution is likely higher than the number of primitives that we are plotting
        // Sum and average our bin(s) of interest from all necessary rows inside the spectrogram "in between" the primitives
        // This is in fact a LOD operation to make sure no information is lost when mapping to our primitives
        for(int dr = 0; dr < numRowsPerInstance; ++dr) {
            int sr = r + dr;

            // Make sure the row coordinate wraps around and stays within bounds
            if (sr < 0) { sr += height; }
            else if (sr >= height) { sr -= height; }

            // Determine the normalized linear start point of this block
            float blockStart = 1.0f - (float(xNr) / float(uXAmount));

            // Determine the normalized linear end point of this block (start point of the next block)
            float blockEnd = 1.0f - (float(xNr + 1) / float(uXAmount));

            // Determine the corresponding logarithmic start and end point
            float blockStartLog = clamp(log_y(blockStart), 0.0f, 1.0f);
            float blockEndLog = clamp(log_y(blockEnd), 0.0f, 1.0f);

            // Determine the corresponding start and end bin ids
            int binStart = int(blockStartLog * float(bins));
            int binEnd = int(blockEndLog * float(bins));
            int numBins = max(1, binEnd - binStart);

            // Average all bins that belong to this block
            float sum = 0.0f;
            for(int b = 0; b < numBins; ++b) {
                sum += texture(uSpectrogram, vec2(float(binStart + b), float(sr)) / vec2(bins, height)).r;
            }
            magnitude += sum / float(numBins);
        }
        magnitude *= 1.0f / numRowsPerInstance;

        // Convert bin magnitude to dB
        float db = db(magnitude);
        float dbClamped = clamp(db, uMinDb, uMaxDb);
        float value = (uMaxDb != uMinDb) ? (dbClamped - uMinDb) / (uMaxDb - uMinDb) : 0.0f;

        Out.SpectrogramValue = value;
    }

    gl_Position = vec4(aPosition + vec3(xPos, 0, zPos), 1.0);
}
