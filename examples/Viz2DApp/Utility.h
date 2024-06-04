#pragma once

// GLM
#include <glm/glm.hpp>

// GSL
#include <gsl/span>

// Stdlib
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <type_traits>
#include <vector>

#define MILLI2MICRO(x) (x * 1000)

#if defined(WIN32)
#define OPTNONE
#elif defined(__APPLE__)
#define OPTNONE __attribute__((optnone))
#endif

using signed_size_t = std::make_signed_t<std::size_t>;

inline auto
atan2f_approx(float y, float x) noexcept -> float
{
    constexpr auto PI = 3.14159265358979323846f;

    const auto absy = std::fabsf(y) + 1e-10f;
    const auto r = (x - std::copysign(absy, x)) / (absy + std::fabs(x));
    const auto angle = (PI / 2.0f - std::copysign(PI / 4.0f, x)) + (0.1963f * r * r - 0.9817f) * r;
    return std::copysign(angle, y);
}

inline auto
cosf_approx(float x) noexcept -> float
{
    constexpr auto PI = 3.14159265358979323846f;

    constexpr auto tp = 1.0f / (2.0f * PI);
    x *= tp;
    x -= 0.25f + std::floor(x + 0.25f);
    x *= 16.0f * (std::abs(x) - 0.5f);
    x += 0.225f * x * (std::abs(x) - 1.0f);
    return x;
}

inline auto
exp2f_approx(float p) noexcept -> float
{
    p = std::max(p, -126.0f);
    const float z = p - int(p) + (p < 0.0f ? 1.0f : 0.0f);
    union
    {
        uint32_t i;
        float f;
    } v = { (uint32_t)((1 << 23) * (p + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z)) };
    return v.f;
}

// NOTE: Doesn't handle 0 case well
inline auto
log2f_approx(float x) noexcept -> float
{
    float y, f;
    int e;
    f = frexpf(fabsf(x), &e);
    y = 1.23149591368684f;
    y *= f;
    y += -4.11852516267426f;
    y *= f;
    y += 6.02197014179219f;
    y *= f;
    y += -3.13396450166353f;
    y += e;
    return y;
}

// NOTE: Doesn't handle 0 case well
#define log10f_approx(x) (log2f_approx(x) * 0.3010299956639812f)

/* MovingAverageTimer */

class MovingAverageTimer final
{
  public:
    void start() noexcept { m_start = std::chrono::high_resolution_clock::now(); }

    auto stop() noexcept -> double
    {
        const auto end = std::chrono::high_resolution_clock::now();

        const auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count() / 1.0e6;

        m_movingAverageInMs = m_alpha * time + (1.0 - m_alpha) * m_movingAverageInMs;

        return m_movingAverageInMs;
    }

    auto getTimeInMs() const noexcept -> double { return m_movingAverageInMs; }

    MovingAverageTimer(double alpha = 0.1) noexcept
      : m_alpha(alpha)
    {
    }

  private:
    const double m_alpha;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

    double m_movingAverageInMs = 0.0;
};

/* Rect */

class Rect
{
  public:
    void constrainTo(const Rect& other) noexcept
    {
        if (getLeft() < other.getLeft()) {
            setLeft(other.getLeft());
        } else if (getRight() > other.getRight()) {
            setRight(other.getRight());
        }

        if (getTop() > other.getTop()) {
            setTop(other.getTop());
        }
        if (getBottom() < other.getBottom()) {
            setBottom(other.getBottom());
        }
    }

    auto getLeft() const noexcept -> float { return m_center.x - m_size.x / 2.0f; }

    auto getRight() const noexcept -> float { return m_center.x + m_size.x / 2.0f; }

    auto getTop() const noexcept -> float { return m_center.y + m_size.y / 2.0f; }

    auto getBottom() const noexcept -> float { return m_center.y - m_size.y / 2.0f; }

    auto getCenter() const noexcept -> glm::vec2 { return m_center; }

    void setLeft(float left) noexcept { m_center.x = left + m_size.x / 2.0f; }

    void setRight(float right) noexcept { m_center.x = right - m_size.x / 2.0f; }

    void setTop(float top) noexcept { m_center.y = top - m_size.y / 2.0f; }

    void setBottom(float bottom) noexcept { m_center.y = bottom + m_size.y / 2.0f; }

    Rect(glm::vec2 center, glm::vec2 size) noexcept
      : m_center(center)
      , m_size(size)
    {
    }

  private:
    glm::vec2 m_center;

    glm::vec2 m_size;
};

/* NonCopyable */

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

template<typename T>
auto
signum(T val) noexcept -> int32_t
{
    return (T(0) < val) - (val < T(0));
}

template<typename T>
auto
approximatelyEqual(T a, T b) noexcept -> bool
{
    return std::abs(a - b) < std::numeric_limits<T>::epsilon();
}

/// @brief Given a frequency, returns a normalized value between 0 and 1
/// @param freq The frequency to normalize
/// @param minFreq The minimum frequency to normalize between, this frequency
/// will return a value of 0.0. Entering a value less than 1.0 will be replaced
/// with 1.0
/// @param maxFreq The maximum frequency to map to, this frequency would be
/// returned as 1.0
/// @return A value between 0 and 1 representing how far freq is between min
/// and max in log scale.
inline auto
freqToNorm(float freq, float minFreq = 20.0f, float maxFreq = 20000.0f) -> float
{
    minFreq = std::clamp(minFreq, 1.0f, maxFreq);
    return log2f(freq / minFreq) / log2f(maxFreq / minFreq);
}

/// @brief Returns a frequency from a normalized value
/// @param normFreq The normalized value between 0 and 1 (inclusive) to map
/// to a frequency
/// @param minFreq The minimum frequency in the range to map to, values less
/// than 1.0 will be map to 1.0
/// @param maxFreq The maximum frequency to map to
/// @return A frequency in hz between min and max that represents the normalized
/// value passed in.
inline auto
normToFreq(float normFreq, float minFreq = 20.0f, float maxFreq = 20000.0f) -> float
{
    minFreq = std::clamp(minFreq, 1.0f, maxFreq);
    return minFreq * powf(2.0, log2f(maxFreq / minFreq) * normFreq);
}

/// @brief Returns a properly formatted string with 2 values after the decimal.
/// If the frequency is greater than 1000 it will return a string like 1.xx khz
/// and if it is less than 1000 it will return a string like 324.xxhz.
/// @param freq The frequency to generate a string for.
/// @return A string representation of the frequency with 2 decimal precision
/// and proper suffix ("hz","khz").
inline auto
freqToString(float freq) -> std::string
{
    const auto suffix = freq >= 1000.0f ? std::string("khz") : std::string("hz");
    bool isKhz = false;
    if (freq >= 1000.0f) {
        freq *= 0.001f;
        isKhz = true;
    }

    std::stringstream stream;
    stream << std::fixed << std::setprecision(isKhz ? 2 : 0) << freq;
    const std::string freqStr = stream.str();

    return freqStr + suffix;
}

inline auto
/// @brief Returns a formatted string from a floating point value.
/// @param val The value to convert to a string.
/// @param precision The number of values after the decimal place to include.
/// @return The formatted string.
floatToString(float val, int precision) -> std::string
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << val;
    std::string str = stream.str();

    return str;
}

/// @brief Vector with information about all possible MIDI notes
static const std::vector<std::string> k_keyNames{
    "C-2", "C#-2", "D-2", "D#-2", "E-2", "F-2", "F#-2", "G-2", "G#-2", "A-2", "A#-2", "B-2", "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1",
    "G-1", "G#-1", "A-1", "A#-1", "B-1", "C0",  "C#0",  "D0",  "D#0",  "E0",  "F0",   "F#0", "G0",  "G#0",  "A0",  "A#0",  "B0",  "C1",  "C#1",
    "D1",  "D#1",  "E1",  "F1",   "F#1", "G1",  "G#1",  "A1",  "A#1",  "B1",  "C2",   "C#2", "D2",  "D#2",  "E2",  "F2",   "F#2", "G2",  "G#2",
    "A2",  "A#2",  "B2",  "C3",   "C#3", "D3",  "D#3",  "E3",  "F3",   "F#3", "G3",   "G#3", "A3",  "A#3",  "B3",  "C4",   "C#4", "D4",  "D#4",
    "E4",  "F4",   "F#4", "G4",   "G#4", "A4",  "A#4",  "B4",  "C5",   "C#5", "D5",   "D#5", "E5",  "F5",   "F#5", "G5",   "G#5", "A5",  "A#5",
    "B5",  "C6",   "C#6", "D6",   "D#6", "E6",  "F6",   "F#6", "G6",   "G#6", "A6",   "A#6", "B6",  "C7",   "C#7", "D7",   "D#7", "E7",  "F7",
    "F#7", "G7",   "G#7", "A7",   "A#7", "B7",  "C8",   "C#8", "D8",   "D#8", "E8",   "F8",  "F#8", "G8"
};

static inline auto
/// @brief Function used to initialize k_noteValyes. Shouldn't be called, you
/// should use k_noteValues instead.
/// @return A vector of all not frequencies in hz that correspond to the strings
/// in k_keyNames.
generateKeyValues() -> const std::vector<float>
{
    std::vector<float> keyVals;

    for (int midiVal = 0; midiVal < (int)k_keyNames.size(); ++midiVal) {
        auto val = 440.0f * std::pow(2.0f, (midiVal - 69) / 12.0f);
        keyVals.push_back(val);
    }

    return keyVals;
}

/// @brief Vector of note frequencies that correspond to the note names in
/// k_keyNames
static const std::vector<float> k_noteValues = generateKeyValues();

/// @brief A function that takes in a hertz value and outputs midi note
/// information.
/// @param hz The frequency of the given note.
/// @return A string representing the nearest midi note value to the given
/// frequency and the difference between the frequency and that note in
/// cents.
static inline auto
hzToNoteString(float hz) -> std::string
{
    std::string_view keyName = k_keyNames[0];
    float refFrequency = k_noteValues[0];

    // Search for notes within MIDI range
    if (hz >= k_noteValues.back()) {
        keyName = k_keyNames.back();
        refFrequency = k_noteValues.back();
    } else if (hz >= k_noteValues[0]) {
        for (size_t i = 0; i < k_noteValues.size() - 1; ++i) {
            const float lowerFrequency = k_noteValues[i];
            const float upperFrequency = k_noteValues[i + 1];
            if (hz >= lowerFrequency && hz <= upperFrequency) {
                // Found matching node, find closest note
                if (std::abs(hz - lowerFrequency) > std::abs(hz - upperFrequency)) {
                    keyName = k_keyNames[i + 1];
                    refFrequency = upperFrequency;
                } else {
                    keyName = k_keyNames[i];
                    refFrequency = lowerFrequency;
                }
                break;
            }
        }
    }

    const float cents = std::round(1200.0f * log2f(hz / refFrequency));
    std::stringstream stream;
    stream << keyName << (cents < 1.0f ? " " : " +") << floatToString(cents, 0);
    return stream.str();
}

/// @brief Constructs a span from iterators, here because some compilers are
/// pieces of shit and don't implement every aspect of the standard.
/// @tparam It Iterator type.
/// @param begin Begin iterator.
/// @param end End iterator.
/// @return Created span.
template<typename It>
constexpr auto
make_span(It begin, It end)
{
    return gsl::span<std::remove_pointer_t<typename It::pointer>>(&(*begin), std::distance(begin, end));
}

/// @brief Remaps a value from a source range to a target range.
/// @tparam T
/// @param sourceValue
/// @param sourceRangeMin
/// @param sourceRangeMax
/// @param targetRangeMin
/// @param targetRangeMax
/// @return
template<typename T>
inline auto
jmap(T sourceValue, T sourceRangeMin, T sourceRangeMax, T targetRangeMin, T targetRangeMax) noexcept -> T
{
    assert(sourceRangeMax != sourceRangeMin); // Mapping range with length zero
    return targetRangeMin + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
}

/// @brief A helper to calculate a normalized x value given a bar and total bars
/// being shown in a component. This assumes the first bar is on the right side
/// of the component and the last bar is at the end of the component.
/// @param bar The bar line to calculate a normalized position for. Bars always
/// start at 1, there is no bar 0 so if we show 32 bars that means we show from
/// bar 1 to bar 33. This number should be between 1 and maxBars.
/// @param maxBars How many total bars are being shown. If showing 32 bars, put
/// 32, not 33.
/// @return A value from 0.0 to 1.0 representing the normalized x position of
/// this bar in the components width.
inline auto
barToNormVal(float bar, float minBars, float maxBars) noexcept -> float
{
    float normBar = jmap(bar, minBars, maxBars, 0.0f, 1.0f);
    return normBar;
}

template<typename T>
inline auto
normalize(T val, T min, T max) noexcept -> T
{
    return (val - min) / (max - min);
}

template<typename T>
inline auto
denormalize(T val, T min, T max) noexcept -> T
{
    return (val * (max - min) + min);
}

inline auto
amplitudeToDb(float amplitude) noexcept -> float
{
    return (amplitude > 0) ? 20.0f * log10f_approx(amplitude) : std::numeric_limits<float>::infinity();
}

inline auto
dbToAmplitude(float db) noexcept -> float
{
    return powf(10.0f, db / 20.0f);
}

inline auto
lerp(float a, float b, float t) noexcept -> float
{
    return a + t * (b - a);
}

inline auto
smoothstep(float a, float b, float t) noexcept -> float
{
    t = std::clamp((t - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/// @brief Returns current microseconds since epoch.
/// @return Microseconds since epoch.
inline auto
getCurrentMicros() noexcept -> long long
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline auto
dbRescale(float x, float scale) noexcept -> float
{
    assert(x >= 0.0f && x <= 1.0f);              // Invalid X value
    assert(scale >= -100.0f && scale <= 100.0f); // Invalid dB scale

    if (scale < 0.0f) {
        return powf(x, powf(0.5f, -scale / 100.0f));
    } else {
        return powf(x, powf(2.0f, scale / 100.0f));
    }
}

// @brief Rounds up to the closest power-of-two.
inline auto
ceilPower2(uint32_t x) noexcept -> uint32_t
{
    if (x <= 1) {
        return 1;
    }

    uint32_t power = 2;
    x--;
    while (x >>= 1) {
        power <<= 1;
    }

    return power;
}

template<typename T>
inline auto
split(std::string string, const std::string& delimiter) -> std::vector<T>
{
    std::vector<T> ret;

    const auto add = [&ret](const auto& token) {
        std::stringstream ss{ token };

        T t;
        ss >> t;

        ret.push_back(t);
    };

    size_t pos = 0;
    std::string token;

    while ((pos = string.find(delimiter)) != std::string::npos) {
        token = string.substr(0, pos);

        add(token);

        string.erase(0, pos + delimiter.length());
    }

    add(string);

    return ret;
}
