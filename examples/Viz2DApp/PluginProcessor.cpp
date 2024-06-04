#include "PluginProcessor.h"

// Plugin
#include "PluginEditor.h"

// JUCE
#include <juce_audio_formats/juce_audio_formats.h>

void
PluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void
PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_spectrexProcessor.prepareToPlay(sampleRate, samplesPerBlock);
}

void
PluginAudioProcessor::releaseResources()
{
}

void
PluginAudioProcessor::processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages)
{
    m_spectrexProcessor.processBlock(playHead, buffer, midiMessages);
}

juce::AudioProcessorEditor*
PluginAudioProcessor::createEditor()
{
    return new PluginEditor(*this);
}

auto
PluginAudioProcessor::acceptsMidi() const -> bool
{
    return false;
}

auto
PluginAudioProcessor::producesMidi() const -> bool
{
    return false;
}

auto
PluginAudioProcessor::silenceInProducesSilenceOut() const -> bool
{
    return false;
}

auto
PluginAudioProcessor::hasEditor() const -> bool
{
    return true;
}

auto
PluginAudioProcessor::isInputChannelStereoPair(int index) const -> bool
{
    return true;
}

auto
PluginAudioProcessor::isOutputChannelStereoPair(int index) const -> bool
{
    return true;
}

auto
PluginAudioProcessor::getName() const -> const juce::String
{
    return JucePlugin_Name;
}

auto
PluginAudioProcessor::getInputChannelName(int channelIndex) const -> const juce::String
{
    return juce::String(channelIndex + 1);
}

auto
PluginAudioProcessor::getOutputChannelName(int channelIndex) const -> const juce::String
{
    return juce::String(channelIndex + 1);
}

auto
PluginAudioProcessor::getTailLengthSeconds() const -> double
{
    return 0.0;
}

auto
PluginAudioProcessor::getNumPrograms() -> int
{
    return 1;
}

auto
PluginAudioProcessor::getCurrentProgram() -> int
{
    return 0;
}

auto
PluginAudioProcessor::getProgramName(int index) -> const juce::String
{
    return juce::String();
}

void
PluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void
PluginAudioProcessor::setCurrentProgram(int index)
{
}

void
PluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

PluginAudioProcessor::PluginAudioProcessor()
  : juce::AudioProcessor(
      BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true))
  , m_spectrexProcessor()
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
}

PluginAudioProcessor::~PluginAudioProcessor() {}

juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new PluginAudioProcessor();
}
