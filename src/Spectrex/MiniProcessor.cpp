#include <Spectrex/MiniProcessor.hpp>

// Spectrex
#include <Spectrex/Processing/Processor.hpp>

// Test signals
#undef TEST_GENERATE_CLEAR
#undef TEST_GENERATE_WHITE_NOISE
#undef TEST_GENERATE_CHIRP
#undef TEST_GENERATE_BEEP

namespace spectrex {

MiniProcessor::ProcessingThread::ProcessingThread(MiniProcessor& owner)
  : juce::Thread("processing")
  , m_owner(owner)
{
}

MiniProcessor::ProcessingThread::~ProcessingThread()
{
    // Attempt to stop the processing thread
    // Take a reasonably large timeout that takes the normal notify timeout into
    // account
    stopThread(k_processingThreadTimeoutMs * 10);
}

/// @thread processing
void
MiniProcessor::ProcessingThread::run()
{
    // Work buffers
    constexpr auto subBlockSize = KProcessor::getExpectedBlockSize();
    std::array<SyncData, subBlockSize> syncDataBlock;
    std::array<std::array<float, subBlockSize>, 2> audioSubBlocks;
    std::array<juce::MidiMessage, subBlockSize> midiSubBlock;

    // State
    double lastPpq = k_PpqInitialState;

    // Processing loop
    while (!threadShouldExit()) {
        // Check if there is any data available at all
        if (m_owner.m_numChannels > 0 &&
            m_owner.m_ringBuffer->getReadSpace() > 0) {
            // Avoid floating point denormals
            juce::ScopedNoDenormals scopedNoDenormals;

            // Ensure processor is prepared
            {
                float totalNumSamples = -1.0f;

                // If the processor could not prepare (initialize), handle this
                // gracefully and ignore all processing
                if (!m_owner.m_processor->prepare(totalNumSamples)) {
                    continue;
                }
            }

            // Get playhead information using a brief lock for a copy only
            //
            // @thread data may not be fully synced up with ringbuffer
            //
            // Do not use critical variables from playhead (ppqPosition*), use
            // SyncData instead.
            bool isPlaying =
              true; // If there is no playhead, we should always just play
            float bpm = 0.0f;
            int timeSigNumerator = 0;
            {
                /// @thread audio
                /// @thread processing
                std::scoped_lock<std::mutex> lock{ m_owner.m_playheadMutex };
                juce::AudioPlayHead::CurrentPositionInfo info =
                  m_owner.m_playhead;
                if (m_owner.m_playheadValid) {
                    isPlaying = info.isPlaying;
                    bpm = info.bpm;
                    timeSigNumerator = info.timeSigNumerator;
                }
            }

            // Set non-critical playhead related variables on processor, if any
            if (bpm > 0.0f) {
                // Set the DAW BPM
                m_owner.m_processor->setParameter<float>(
                  ProcessorParameters::Key::Bpm, bpm);
            }
            if (timeSigNumerator > 0) {
                m_owner.m_processor->setParameter<int>(
                  ProcessorParameters::Key::TimeSignatureNumerator,
                  timeSigNumerator);
            }

            // If we have enough data inside our read space to read a sub-block
            // per channel, do so and perform sub-block processing
            /// @thread m_audioRingBuffer read from processing thread (consumer)
            /// @thread m_midiRingBuffer read from processing thread (consumer)
            while (m_owner.m_ringBuffer->getReadSpace() >= subBlockSize) {
                // Read data from ring buffer
                m_owner.m_ringBuffer->read(syncDataBlock.data(), subBlockSize);

                // Unpack into individual buffers
                for (int i = 0; i < subBlockSize; ++i) {
                    const SyncData& s = syncDataBlock[i];
                    audioSubBlocks[0][i] = s.left;
                    audioSubBlocks[1][i] = s.right;

                    // Reset the play position on any note on/off event, we
                    // check the entire block here so there can be a really
                    // minor offset
                    if (s.noteOn) {
                        m_owner.m_processor->resetPosition();
                    }
                }

                // Get ppqPosition from the last sample, since it is only done
                // on a block basis, it will usually be the same throughout
                // (though not guaranteed to be). The last sample will be the
                // most recent sample.
                {
                    double ppqPosition =
                      syncDataBlock[subBlockSize - 1].ppqPosition;
                    double ppqLoopEnd =
                      syncDataBlock[subBlockSize - 1]
                        .ppqLoopEnd; // Use infinite to encode no loop

                    if (isPlaying) {
                        m_owner.m_lastTimeInQuarters.store(ppqPosition);
                    }

                    // Modulate ppq position according to loop, if
                    // information is available. This avoids some DAWs
                    // letting ppqPosition go beyond ppqLoopEnd, causing
                    // samples to be visualized beyond the loop point.
                    ppqPosition = isfinite(ppqLoopEnd) && ppqLoopEnd > 0
                                    ? (fmod(ppqPosition, ppqLoopEnd))
                                    : ppqPosition;

                    // Resync condition, any of the following:
                    // * PPQ delta exceeds threshold
                    // * PPQ jumped back in time
                    // * Processor was initialized (lastPpq is initial
                    // state)
                    // * Processor was initialized (isInitialPosition())
                    const auto ppqDelta = abs(lastPpq - ppqPosition);
                    bool ppqResync = (ppqDelta > k_resyncPpqThreshold) ||
                                     (ppqPosition < lastPpq) ||
                                     (lastPpq == k_PpqInitialState) ||
                                     m_owner.m_processor->isInitialPosition();

                    // Always force position resync to zero when in pre-roll
                    // (ppq is negative), so it always starts at the right
                    // point when the ppq becomes positive.
                    if (ppqPosition < 0) {
                        ppqPosition = 0;
                        ppqResync = true;
                    }

                    // Stores absolute ppq for GUI purposes, performs no
                    // syncronization of data
                    m_owner.m_processor->setAbsolutePosition(ppqPosition);

                    // Set is playing state and update play position
                    // according to PPQ information either if we start
                    // playing, or the PPQ delta is greater than the resync
                    // threshold (in PPQ)
                    if (m_owner.m_processor->setPlaying(isPlaying) ||
                        (isPlaying && ppqResync)) {
                        m_owner.m_processor->setPosition(ppqPosition);
                    }
                    lastPpq = ppqPosition;
                }

                // Perform processing of sub-blocks
                m_owner.m_processor->process(
                  audioSubBlocks[0], audioSubBlocks[1], m_owner.m_numChannels);
            }
        }
        // Wait for next event or timeout
        wait(k_processingThreadTimeoutMs);
    }
}

void
MiniProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) noexcept
{
    // Update the sample rate
    DBG("Sample rate = " << sampleRate);

    m_processor->setParameter<float>(ProcessorParameters::Key::SampleRate,
                                     sampleRate);
    m_sampleRate = sampleRate;

    // Perform preparation
    float totalNumSamples = -1.0f;

    m_processor->prepare(totalNumSamples);

    if (totalNumSamples > 0.0f) {
        DBG("Total number of samples visualized = " << totalNumSamples);
    }
}

void
MiniProcessor::processBlock(juce::AudioPlayHead* playHead,
                            juce::AudioSampleBuffer& buffer,
                            juce::MidiBuffer& midiMessages) noexcept
{
    // @perf This thread should touch m_processor as little as possible, to
    // decouple any synchronization behaviour from the audio thread.

    // Hardcoded configuration
    const auto triggerEnabled = false;

    // Avoid floating point denormals
    juce::ScopedNoDenormals scopedNoDenormals;

    // Retrieve audio data
    m_processingBuffer.makeCopyOf(buffer);

    // If mono (not enough channels), mirror data into stereo channel
    if (buffer.getNumChannels() < 2) {
        m_processingBuffer.setSize(2, buffer.getNumSamples(), true);
        juce::FloatVectorOperations::copy(m_processingBuffer.getWritePointer(1),
                                          m_processingBuffer.getReadPointer(0),
                                          m_processingBuffer.getNumSamples());
    }

    // Retrieve number of samples and channels
    //
    // The buffer only has place for two channels, fewer channels is fine, but
    // anything above 2 will not work, so clamp to 2
    const auto numSamples = m_processingBuffer.getNumSamples();
    constexpr auto subBlockSize = KProcessor::getExpectedBlockSize();
    const auto numChannels = std::min(2, m_processingBuffer.getNumChannels());
    m_numChannels = numChannels;

    // Nothing to do
    if (numChannels == 0 || numSamples == 0) {
        return;
    }

    // Critical playhead information that should be stored in ring buffer
    double ppqPosition = 0.0f;
    double ppqLoopEnd =
      std::numeric_limits<double>::infinity(); // Use infinite to encode no loop

    // Get playhead information, set and synchronize
    {
        /// @thread audio
        /// @thread processing
        std::scoped_lock<std::mutex> lock{ m_playheadMutex };

        bool playheadValid = false;
        if (playHead != nullptr) {
            if (playHead->getCurrentPosition(m_playhead)) {
                // Critical information
                ppqPosition = m_playhead.ppqPosition;
                if (m_playhead.isLooping) {
                    ppqLoopEnd = m_playhead.ppqLoopEnd;
                }
                playheadValid = true;
            }
        }
        m_playheadValid = playheadValid;
    }

#ifdef TEST_GENERATE_CLEAR
    // Clear data
    {
        float* const* data = m_processingBuffer.getArrayOfWritePointers();

        for (int c = 0; c < numChannels; ++c) {
            for (int i = 0; i < numSamples; ++i) {
                data[c][i] = 0.0f;
            }
        }
    }
#endif // TEST_GENERATE_CLEAR

#ifdef TEST_GENERATE_WHITE_NOISE
    // Generate white noise for testing purposes
    {
        float* const* data = m_processingBuffer.getArrayOfWritePointers();

        for (int c = 0; c < numChannels; ++c) {
            for (int i = 0; i < numSamples; ++i) {
                data[c][i] +=
                  juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            }
        }
    }
#endif // TEST_GENERATE_WHITE_NOISE

#ifdef TEST_GENERATE_CHIRP
    // Generate chirp for testing purposes
    {
        const auto T = 2.0f;

        auto chirp =
          [](double f0, double f1, double t1, double t) noexcept -> double {
            const auto beta = (f1 - f0) / t1;
            return cos(2.0 * juce::MathConstants<double>::pi *
                       (f0 * t + 0.5 * beta * t * t));
        };

        float* const* data = m_processingBuffer.getArrayOfWritePointers();

        static double t = 0.0;

        for (int i = 0; i < numSamples; ++i) {
            const auto val = chirp(20.0, 20000.0, T, t);

            for (int c = 0; c < numChannels; ++c) {
                data[c][i] += val;
            }

            t += 1.0 / getSampleRate();
            if (t >= T) {
                t = 0.0;
            }
        }
    }
#endif // TEST_GENERATE_CHIRP

#ifdef TEST_GENERATE_BEEP
    // Generate beep for testing purposes
    {
        static bool beep = true;

        static int globalSamplePosition = 0;

        static double angle = 0.0;

        const auto delta =
          440.0 / getSampleRate() * 2.0 * juce::MathConstants<double>::pi;

        const auto fadeLength = getSampleRate() / 100.0f;

        float* const* data = m_processingBuffer.getArrayOfWritePointers();

        for (int i = 0; i < numSamples; ++i, ++globalSamplePosition) {
            for (int c = 0; c < numChannels; ++c) {
                if (globalSamplePosition > getSampleRate()) {
                    beep = !beep;

                    globalSamplePosition = 0;
                    angle = 0.0;
                }

                if (beep) {
                    auto amp = 1.0f;
                    if (globalSamplePosition < fadeLength) {
                        amp = globalSamplePosition / fadeLength;
                    } else if (globalSamplePosition >=
                               getSampleRate() - fadeLength) {
                        amp = 1.0f - (globalSamplePosition -
                                      (getSampleRate() - fadeLength)) /
                                       fadeLength;
                    }

                    data[c][i] = std::sin(angle) * amp;
                }
            }
            angle += delta;
        }
    }
#endif // TEST_GENERATE_BEEP

    juce::MidiBuffer::Iterator midiIt{ midiMessages };

    // Find first MIDI message, if any
    juce::MidiMessage message{};
    int midiSamplePosition = 0;
    if (!midiIt.getNextEvent(message, midiSamplePosition)) {
        // No further MIDI messages, invalidate sample position
        midiSamplePosition = -1;
    }

    // Perform "sub-block" processing. Two ring buffers twice the size of the
    // sub-block size are used. Instead of processing the length of the input
    // buffer directly, the input buffer is broken up into chunks the size of
    // the sub-block size. Any remaining samples from the input buffer due to
    // the input buffer size not being divisible by the sub-block size are
    // contained in the channel's ring buffer and are processed once new data
    // comes in
    /// @thread m_audioRingBuffer write from audio thread (producer)
    /// @thread m_midiRingBuffer write from audio thread (producer)
    for (int i = 0; i < numSamples; i += subBlockSize) {
        const auto n = juce::jmin((int)subBlockSize, numSamples - i);

        for (int j = 0; j < n; ++j) {
            // Write channels into their buffers, if a channel is missing
            // (mono), insert zeroes to keep buffers in sync
            SyncData s;
            s.left = *(m_processingBuffer.getReadPointer(0) + i + j);
            s.right = 0.0f;
            if (numChannels == 2) {
                s.right = *(m_processingBuffer.getReadPointer(1) + i + j);
            }

            // See if we have a matching MIDI message
            s.noteOn = false;
            if (midiSamplePosition == i + j && triggerEnabled) {
                if (!midiIt.getNextEvent(message, midiSamplePosition)) {
                    // No further MIDI messages, invalidate sample position
                    midiSamplePosition = -1;
                }
                s.noteOn = true;
            }

            // Critical playhead information (per block)
            s.ppqPosition = ppqPosition;
            s.ppqLoopEnd = ppqLoopEnd;

            // Add to ring buffer
            m_ringBuffer->push(s);
        }

        // m_audioRingBuffer read will be handled async by processing thread
        // Wake up the processing thread in any case
        m_processingThread.notify();
    }

#ifdef TEST_GENERATE_CLEAR
    // Clear data
    {
        float* const* data = m_processingBuffer.getArrayOfWritePointers();

        for (int c = 0; c < numChannels; ++c) {
            for (int i = 0; i < numSamples; ++i) {
                data[c][i] = 0.0f;
            }
        }
    }
#endif // TEST_GENERATE_CLEAR
}

MiniProcessor::MiniProcessor() noexcept
  : m_processingThread(*this)
{
    // Instantiate audio processor
    m_processor = std::make_unique<spectrex::KProcessor>();

    // Configuration
    m_processor->setParameter(spectrex::ProcessorParameters::Key::FtSize,
                              spectrex::FtSize::Size256);

    // Initialize ring buffer
    constexpr auto subBlockSize = KProcessor::getExpectedBlockSize();
    m_ringBuffer = std::make_unique<RingBuffer<SyncData>>(
      subBlockSize * k_ringBufferElements, SyncData{});

    // Start the processing thread with high priority
    m_processingThread.startThread(juce::Thread::Priority::high);
}

MiniProcessor::~MiniProcessor() {}

} // namespace spectrex
