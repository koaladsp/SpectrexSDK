#pragma once

// Spectrex
#include <Spectrex/Utility/RingBuffer.hpp>

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>

// Stdlib
#include <memory>

namespace spectrex {

class KProcessor;

/// MiniProcessor implements the minimum necessary processing functionality to connect Spectrex to a potential DAW or audio device. It is an open
/// implementation that can be changed as necessary.
class MiniProcessor final
{
  public:
    /// Called before playback starts, to let the processor prepare itself. Corresponds to the juce::AudioProcessor::prepareToPlay function.
    void prepareToPlay(double sampleRate, int samplesPerBlock) noexcept;
    /// Renders the next block. Corresponds to the juce::AudioProcessor::processBlock function.
    void processBlock(juce::AudioPlayHead* playHead, juce::AudioSampleBuffer&, juce::MidiBuffer&) noexcept;
    /// Returns the current sample rate. Corresponds to the juce::AudioProcessor::getSampleRate function.
    double getSampleRate() const noexcept { return m_sampleRate; }

    /// Returns the underlying spectrex::KProcessor.
    auto getProcessor() const noexcept -> spectrex::KProcessor& { return *m_processor; }

    /// Returns the most recent ppq in quarter notes given by the host.
    auto getLastPosInQtrs() const noexcept -> double { return m_lastTimeInQuarters.load(); }

    MiniProcessor() noexcept;
    ~MiniProcessor();

  private:
    /// Internal processing thread to offload audio processing (producer) and any potential audio/gl (producer/consumer) interoperability. This
    /// decouples any wait states from the audio thread.
    class ProcessingThread : public juce::Thread
    {
      public:
        void run() override;

        // Timeout for thread notify in ms
        static constexpr int k_processingThreadTimeoutMs = 15;

        ProcessingThread(MiniProcessor& owner);
        ~ProcessingThread();

      private:
        MiniProcessor& m_owner;
    };

  private:
    /// The amount of ppq change before a complete resync event is triggered.
    static constexpr double k_resyncPpqThreshold = 1.0;
    /// Initial value of the ppq counter.
    static constexpr double k_PpqInitialState = -1.0f;

    /// Underlying processor.
    std::shared_ptr<spectrex::KProcessor> m_processor;

    /// Current sample rate.
    double m_sampleRate = 0;

    /// Number of elements (processing blocks) inside ring buffers. Should be "big enough" to accommodate for a heavily lagging processing thread.
    static constexpr int k_ringBufferElements = 4096;

    /// Current number of effective channels that need to be processed.
    /// @thread audio
    /// @thread processing
    std::atomic<int> m_numChannels = 0;

    /// Ring buffer for audio and MIDI shared between audio and processing. threads.
    ///
    /// Ideally this would be a SoA not AoS to avoid unpacking but RingBuffer does not allow for this currently.
    ///
    /// @thread audio
    /// @thread processing
    struct SyncData
    {
        float left = 0;
        float right = 0;
        bool noteOn = false;

        /// Data that is stored per block only, not per sample.
        /// This is put into the ring buffer anyway to keep synchronization simple, at the expense of memory space.
        double ppqPosition = 0;
        double ppqLoopEnd = 0; // Use infinite to encode no loop
    };
    std::unique_ptr<RingBuffer<SyncData>> m_ringBuffer;

    /// Playhead information (synchronized).
    /// @thread audio
    /// @thread processing
    juce::AudioPlayHead::CurrentPositionInfo m_playhead;
    std::atomic<bool> m_playheadValid;
    std::mutex m_playheadMutex;

    /// Last time in quarters according to playhead.
    /// @thread audio
    /// @thread processing
    std::atomic<double> m_lastTimeInQuarters = 0.0;

    /// Temporary processing buffer.
    juce::AudioSampleBuffer m_processingBuffer;

    /// Processing thread.
    ProcessingThread m_processingThread;
};

} // namespace spectrex
