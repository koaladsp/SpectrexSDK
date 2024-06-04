#include "PluginEditor.h"

// Plugin
#include "PluginProcessor.h"

PluginEditor::PluginEditor(PluginAudioProcessor& p)
  : AudioProcessorEditor(&p)
  , m_processor(p)
{
    setSize(1000, 600);
    setResizable(true, false);

    // OpenGL context
    m_openGLContext.setTopLevelParentComponent(*this);
    m_openGLContext.setBeginFrameCallback(std::bind(&PluginEditor::beginGLDrawFrame, this));

    // Set up callback in case of GL failure
    m_openGLContext.setFailureCallback([this]() { assert(false); });

    // Initialize visualization container component
    m_viz2DComponent = std::make_unique<Visualization2DComponent>(m_openGLContext, p, m_parameters);
    addAndMakeVisible(*m_viz2DComponent);

    // 30 Hz frame rate enforcement to force-render JUCE UI
    startTimerHz(30);

    // Parameter window
    {
#ifdef PARAMETER_WINDOW
        m_parameterWindow = std::make_unique<ParameterWindow>(m_parameters);
#endif
    }

    resized();
}

PluginEditor::~PluginEditor()
{
    m_openGLContext.detachTopLevelParentComponent();
}

void
PluginEditor::paint(juce::Graphics& g)
{
}

void
PluginEditor::resized()
{
    auto b = getLocalBounds();
    if (m_viz2DComponent) {
        m_viz2DComponent->setBounds(b);
    }
}

void
PluginEditor::timerCallback()
{
    // Force redraw
    repaint(getLocalBounds());
}

void
PluginEditor::beginGLDrawFrame()
{
    // Single synchronization point to gather any data from the
    // processors that can be used consistently throughout the entire
    // frame.
    m_processor.getSpectrexMiniProcessor().getProcessor().cacheSyncWaveformSpectrogram();
}
