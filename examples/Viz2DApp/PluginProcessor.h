#pragma once

// JUCE
#include <juce_audio_processors/juce_audio_processors.h>

// Spectrex
#include <Spectrex/MiniProcessor.hpp>

// Stdlib
#include <memory>

class PluginAudioProcessor : public juce::AudioProcessor
{
  public:
    void changeProgramName(int index, const juce::String& newName) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioSampleBuffer&, juce::MidiBuffer&) override;

    auto createEditor() -> juce::AudioProcessorEditor* override;

    auto acceptsMidi() const -> bool override;
    auto producesMidi() const -> bool override;
    auto silenceInProducesSilenceOut() const -> bool override;

    auto hasEditor() const -> bool override;

    auto isInputChannelStereoPair(int index) const -> bool override;
    auto isOutputChannelStereoPair(int index) const -> bool override;

    auto getName() const -> const juce::String override;
    auto getInputChannelName(int channelIndex) const -> const juce::String override;
    auto getOutputChannelName(int channelIndex) const -> const juce::String override;
    auto getTailLengthSeconds() const -> double override;
    auto getNumPrograms() -> int override;
    auto getCurrentProgram() -> int override;
    auto getProgramName(int index) -> const juce::String override;
    void getStateInformation(juce::MemoryBlock& destData) override;

    void setCurrentProgram(int index) override;

    void setStateInformation(const void* data, int sizeInBytes) override;

    // Spectrex
    spectrex::MiniProcessor& getSpectrexMiniProcessor() noexcept { return m_spectrexProcessor; }

    PluginAudioProcessor();
    ~PluginAudioProcessor();

  private:
    spectrex::MiniProcessor m_spectrexProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginAudioProcessor)
};