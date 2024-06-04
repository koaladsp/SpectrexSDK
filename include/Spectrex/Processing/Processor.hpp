#pragma once

// Spectrex
#include <Spectrex/Processing/Data.hpp>
#include <Spectrex/Processing/Parameters.hpp>
#include <Spectrex/extra.h>

// Stdlib
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <typeinfo>
#include <unordered_map>

namespace spectrex {

/// Main processor class.
///
/// Specific audio analysis occurs in separate processors owned by the this main processor.
///
/// KProcessor is thread-safe and ensures that data is synchronized between an audio, UI and rendering thread.
class KProcessor
{
  public:
    /// Type of time unit.
    enum class TimeUnit
    {
        Bars,
        Time
    };

    /// Handler function type definition.
    template<typename T>
    using SyncHandler = std::function<void(SyncInfo<T>, std::optional<SyncInfo<T>>)>;

    /// Processor expects audio samples to be delivered in fixed blocks of this size. Various assumptions are made based on this block size.
    static constexpr uint32_t k_expectedBlockSize = 32;

  public:
    /// Synchronizes with the beginning of a rendering frame.
    ///
    /// This should be called at the beginning of a rendering frame, for example in a function such as beginGLDrawFrame(). It prepares any data caches
    /// for reuse during rendering to make sure no tearing or synchronization issues will appear while data is being processed.
    ///
    /// @thread consumer
    void beginFrame() noexcept;

    /// Processes audio data.
    ///
    /// Performs processing of the data audio data with numSamples number of samples and numChannels number of audio channels. Processing
    /// will make use of individual processors for different (specific) kinds of analysis on the audio data.
    ///
    /// Processing will not happen if the this Processor is in a frozen state, or whenever a parameter such as the sample rate has been changed
    /// without a successive call to Processor::prepare.
    ///
    /// This function should only be called from the audio thread.
    ///
    /// @param left Audio (left channel) input data.
    /// @param right Audio (right channel) input data.
    /// @param numChannels Number of audio channels in buffer.
    void process(AudioChannelView left, AudioChannelView right, uint32_t numChannels) noexcept;

    /// Prepare this Processor for processing.
    ///
    /// Should be called whenever one of the following parameters have changed:
    ///
    /// - Sample rate
    /// - Number of bars
    ///
    /// When one of these properties has changed, the Processor will be in an invalid state, until Processor::prepare is called. Whenever this
    /// Processor is in an invalid state, no processing will occur.
    ///
    /// Will return false whenever an error has occured and the Processor is in an invalid state.
    ///
    /// @param totalNumSamples Will be set to the total number of samples that will be displayed in the visualizations, depending on the current
    /// parameters.
    /// @return Will return false if an error occured, else true.
    /// @thread audio
    auto prepare(float& totalNumSamples) noexcept -> bool;

    /// Reset the visualization position.
    void resetPosition() noexcept;

    /// Cache all possible infos for scrolling visualizations (spectrogram and waveform) that need to be in sync. Optional use.
    ///
    /// For these visualizations, all sync* calls will use the cached variables
    /// gathered by this function. This is to ensure that during an entire
    /// graphics frame, these components see data from the same moment in time,
    /// to prevent misalignment.
    ///
    /// @thread consumer
    void cacheSyncWaveformSpectrogram() noexcept;

    /// Data synchronization function for waveform plot.
    ///
    /// This function is used to get data from spectrex into your app.
    ///
    /// @param channel Channel to synchronize data for.
    /// @param handler Handling function.
    /// @thread consumer
    void syncWaveform(Channel channel, SyncHandler<WaveformBin> handler) noexcept;

    /// Data synchronization function for spectrogram.
    ///
    /// This function is used to get data from spectrex into your app.
    ///
    /// @param handler Handling function.
    /// @thread consumer
    void syncSpectrogram(SyncHandler<float> handler) noexcept;

    /// Data synchronization function for spectrum analyzer.
    ///
    /// This function is used to get data from spectrex into your app.
    ///
    /// @param handler Handling function.
    /// @thread consumer
    void syncSpectrum(SyncHandler<SpectrumValue> handler) noexcept;

    /// Returns a flag indicating if the processor is in a valid state.
    auto isValid() const noexcept -> bool;

    /// Returns whether the position is at its initial condition, such as right after initialization.
    auto isInitialPosition() const noexcept -> bool;

    /// Returns the spectrogram info.
    auto getSpectrogramInfo() const noexcept -> SpectrogramInfo;

    /// Returns the waveform info.
    auto getWaveformInfo(Channel channel) const noexcept -> WaveformInfo;

    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @tparam T Return type.
    /// @param key Key.
    template<typename T>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> T;
    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @param key Key.
    template<>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> float;
    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @param key Key.
    template<>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> int;
    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @param key Key.
    template<>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> Window;
    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @param key Key.
    template<>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> FtSize;
    /// Gets a value, based on a Key. Locks for safety and is therefore mutable.
    /// @param key Key.
    template<>
    auto getParameter(ProcessorParameters::Key key) const noexcept -> MixMode;

    /// Gets the current unit of time.
    auto getTimeUnit() const noexcept -> TimeUnit;

    /// Gets the quantity of time in time units.
    auto getTimeQuantity() const noexcept -> float;

    /// Gets the total number of samples within the entire processing window.
    auto getTotalNumSamples() const noexcept -> int;

    /// Gets the current position in ppq.
    auto getPosition() const noexcept -> float;

    /// Returns the block size that this processor expects for the audio input samples as static compile-time constant.
    static constexpr auto getExpectedBlockSize() noexcept -> uint32_t { return k_expectedBlockSize; }

    /// Gets the dB scale to apply to magnitude values in the spectrogram and spectrum analyzer between [-1 and 1] which respectively maps to halving
    /// or doubling the values.
    auto getAnalyzerDbScale() const noexcept -> float;

    /// Gets the number of bins used for the spectrum and spectrum analyzer.
    auto getAnalyzerNumBins() const noexcept -> uint32_t;

    /// Gets the spectrum highlight minimum threshold value in dB.
    auto getAnalyzerHighlightThreshold() const noexcept -> float;

    /// Gets the spectrum hold time in seconds. This holds the maximum values of the spectrum in place for the given time.
    auto getAnalyzerHold() const noexcept -> float;

    /// Gets the spectrogram tilt in dB per octave.
    auto getSpectrogramTiltDbPerOctave() const noexcept -> float;

    /// Gets the spectrogram magnitude attack in seconds.
    auto getSpectrogramAttack() const noexcept -> float;

    /// Gets the spectrogram magnitude release in seconds.
    auto getSpectrogramRelease() const noexcept -> float;

    /// Gets the spectrum bin falloff (release) in seconds.
    auto getSpectrumFalloff() const noexcept -> float;

    /// Gets the spectrum highlight attack in seconds.
    auto getSpectrumHighlightAttack() const noexcept -> float;

    /// Gets the spectrum highlight release in seconds.
    auto getSpectrumHighlightRelease() const noexcept -> float;

    /// Gets the waveform headroom as a ratio between [0, 1].
    auto getWaveformHeadroom() const noexcept -> float;

    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<typename T>
    void setParameter(ProcessorParameters::Key key, const T& v) noexcept;
    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<>
    void setParameter(ProcessorParameters::Key key, const float& v) noexcept;
    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<>
    void setParameter(ProcessorParameters::Key key, const int& v) noexcept;
    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<>
    void setParameter(ProcessorParameters::Key key, const Window& v) noexcept;
    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<>
    void setParameter(ProcessorParameters::Key key, const FtSize& v) noexcept;
    /// Sets a processor parameters with the given Key to the given value.
    /// @param key Key to set value for
    /// @param v Value to set for key.
    template<>
    void setParameter(ProcessorParameters::Key key, const MixMode& v) noexcept;

    /// Enables or disables the auto DC functionality.
    void setAutoDC(bool enabled) noexcept;

    /// Enables or disables the freeze state of this Processor.
    ///
    /// When frozen, the Processor will perform no processing and the data visualized will not be updated according to the most current audio data.
    void setFrozen(bool enabled) noexcept;

    /// Enables or disables the playing state of this Processor.
    ///
    /// @return Flag indicating whether or not the play state has been changed.
    auto setPlaying(bool enabled) noexcept -> bool;

    /// Sets the current position.
    ///
    /// This taking the current conditions into account, e.g. override mode is set and currently playing.
    ///
    /// @param ppq Position (in PPQ).
    void setPosition(float ppq) noexcept;

    /// Sets the absolute position.
    ///
    /// Sets the absolute ppq in the processors but does not perform any processing synchronization.
    /// This is different from setPosition and is used by the processors to maintain the most recent absolute ppq written to data for GUI
    /// syncronization purposes.
    ///
    /// @param ppq Absolute position from the host (in PPQ).
    void setAbsolutePosition(float ppq) noexcept;

    /// Sets the dB scale to apply to magnitude values in the spectrogram and spectrum analyzer between [-1 and 1] which respectively maps to halving
    /// or doubling the values.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setAnalyzerDbScale(float dB) noexcept;

    /// Sets the number of bins used for the spectrum and spectrum analyzer.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setAnalyzerNumBins(uint32_t numBins) noexcept;

    /// Sets the current normalized position [0, 1] of the spectrum analyzer to read its data from, if it is in a frozen or non-playing state or
    /// otherwise not processing data. This is typically used to change the analyzer's currently shown data whenever the app is paused, and you have
    /// an already filled buffer of data, and want to implement zooming or panning functionality that reflects back to whatever the spectrum analyzer
    /// is showing. This variable is ignored whenever audio is being streamed into the analyzer.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setAnalyzerDataPosition(float value) noexcept;

    /// Sets the spectrum highlight minimum threshold value in dB.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setAnalyzerHighlightThreshold(float value) noexcept;

    /// Sets the spectrum hold time in seconds. This holds the maximum values of the spectrum in place for the given time.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setAnalyzerHold(float value) noexcept;

    /// Sets the spectrogram tilt in dB per octave.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrogramTiltDbPerOctave(float dbPerOctave) noexcept;

    /// Sets the spectrogram magnitude attack in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrogramAttack(float seconds) noexcept;

    /// Sets the spectrogram magnitude release in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrogramRelease(float seconds) noexcept;

    /// Sets the spectrum bin falloff (release) in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrumFalloff(float seconds) noexcept;

    /// Sets the spectrum highlight attack in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrumHighlightAttack(float seconds) noexcept;

    /// Sets the spectrum highlight release in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setSpectrumHighlightRelease(float seconds) noexcept;

    /// Sets the waveform headroom as a ratio between [0, 1].
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setWaveformHeadroom(float value) noexcept;

    /// Sets the current normalized position [0, 1] of the metering to read its data from, if it is in a frozen or non-playing state or
    /// otherwise not processing data. This is typically used to change the analyzer's currently shown data whenever the app is paused, and you have
    /// an already filled buffer of data, and want to implement zooming or panning functionality that reflects back to whatever the metering
    /// is showing. This variable is ignored whenever audio is being streamed into the analyzer.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setMeterDataPosition(float value) noexcept;

    /// Sets the peak meter attack in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setMeterPeakAttack(float seconds) noexcept;

    /// Sets the rms meter attack in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setMeterRmsAttack(float seconds) noexcept;

    /// Sets the peak meter release in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setMeterPeakRelease(float seconds) noexcept;

    /// Sets the rms meter release in seconds.
    ///
    /// State parameter that is dynamically changeable without performance side-effects.
    void setMeterRmsRelease(float seconds) noexcept;

    /// Constructs an empty invalid Processor instance.
    KProcessor();
    ~KProcessor();

    KDSP_IMPL(Processor)
};

} // namespace spectrex
