#pragma once

// GSL
#include <gsl/span>

// Stdlib
#include <cstdint>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>

namespace spectrex {

/// Channel (selection) type.
using Channel = size_t;

/// Indices the left channel of a stereo channel setup.
constexpr Channel Left = 0;
/// Indices the right channel of a stereo channel setup.
constexpr Channel Right = 1;
/// Indices the downmixed mono channel of a stereo channel setup.
constexpr Channel Mix = 2;

/// Data view type of an audio channel.
using AudioChannelView = gsl::span<const float>;

/// Value contained within the spectrum data.
struct SpectrumValue
{
    /// Spectrum value at frequency (magnitude).
    float Value = 0.0f;

    /// Highlight value at frequency.
    float Highlight = 0.0f;

    /// Historic value at frequency.
    float History = 0.0f;

    /// Hold value.
    float Hold = 0.0f;

    /// Adds another spectrum value in-place.
    auto operator+=(const SpectrumValue& other) -> SpectrumValue&;
};

auto
operator/(const SpectrumValue& value, const float v) -> SpectrumValue;

auto
operator<(const SpectrumValue& a, const SpectrumValue& b) -> bool;

/// A representation of the value(s) contained within a single visible unit of the waveform data at a particular time. This "bin" mechanism is
/// necessary because waveforms can be zoomed in and out, and any particular visualized "line" of data may actually contain a lot of values underneath
/// it. In order to plot waveforms properly, we need to know the characteristics of these values, such as minimum and maximum amplitude.
struct WaveformBin
{
    /// Minimum amplitude of any contained values in this bin.
    float Min = 0.0f;

    /// Maximum amplitude of any contained values in this bin.
    float Max = 0.0f;

    /// An estimation of the frequency of the signal in this bin.
    float Frequency = 0.0f;
};

/// Struct with the current state information for a spectrogram visualization.
struct SpectrogramInfo
{
    /// Texture height (the Y axis maps the rows).
    size_t Height = 0;

    /// Texture width (the X axis maps the bins).
    size_t Width = 0;

    /// Number of actual visible rows inside the spectrogram.
    /// This may be less than the texture height in some modes.
    size_t Rows = 0;

    /// Minimum frequency in the spectrogram.
    float MinFrequency = 0.0f;

    /// Maximum frequency in the spectrogram.
    float MaxFrequency = 0.0f;

    /// Time offset of the spectrogram.
    float TimeOffset = 0.0f;

    /// Total number of rows written.
    /// This can be used to deduce the position of the newest row along the spectrogram.
    size_t RowsWritten = 0;

    /// Normalized position [0, 1] of the last written data in the spectrogram.
    float Position = 0.0f;

    /// Construct an empty instance.
    SpectrogramInfo() {}

    /// Constructs a new instance.
    SpectrogramInfo(size_t width,
                    size_t height,
                    size_t rows,
                    float minFrequency,
                    float maxFrequency,
                    float timeOffset,
                    size_t rowsWritten,
                    float position) noexcept
      : Width(width)
      , Height(height)
      , Rows(rows)
      , MinFrequency(minFrequency)
      , MaxFrequency(maxFrequency)
      , TimeOffset(timeOffset)
      , RowsWritten(rowsWritten)
      , Position(position)
    {
    }
};

/// Struct with the current state information for a waveform visualization.
struct WaveformInfo
{
    /// Total height of the waveform
    size_t Height = 0;

    /// Current write pointer of the waveform
    size_t WritePointer = 0;

    /// Construct an empty instance.
    WaveformInfo() noexcept {}

    /// Construct a new instance.
    WaveformInfo(size_t height, size_t writePointer) noexcept
      : Height(height)
      , WritePointer(writePointer)
    {
    }
};

/// Describes a block of memory that can be synchronized. Memory starts at \a Pointer, pointing to the \a RowIndex'th row and is Width x Height
/// elements long.
/// @tparam T Type of data.
template<typename T>
struct SyncInfo
{
    /// Row index.
    size_t RowIndex = 0;

    /// Pointer into data.
    T* Pointer = nullptr;

    /// Width of this data block that should be synchronized.
    size_t Width = 0;

    /// Height of this data block that should be synchronized.
    size_t Height = 0;

    /// Marks a clear condition, requiring all buffers to be cleared.
    bool Clear = false;

    /// Returns whether the SyncInfo object is valid
    bool isValid() const noexcept { return Pointer != nullptr && Width > 0 && Height > 0; }

    /// Construct an empty instance.
    SyncInfo()
      : RowIndex(0)
      , Pointer(nullptr)
      , Width(0)
      , Height(0)
      , Clear(false)
    {
    }

    /// Construct an empty instance indicating invalid data or a clear condition
    SyncInfo(bool Clear)
      : RowIndex(0)
      , Pointer(nullptr)
      , Width(0)
      , Height(0)
      , Clear(Clear)
    {
    }

    /// Construct a new instance.
    /// @param index Index into data.
    /// @param pointer Pointer into data.
    /// @param width Width of block.
    /// @param height Height of block.
    SyncInfo(size_t index, T* pointer, size_t width, size_t height)
      : RowIndex(index)
      , Pointer(pointer)
      , Width(width)
      , Height(height)
      , Clear(false)
    {
    }
};

} // namespace spectrex
