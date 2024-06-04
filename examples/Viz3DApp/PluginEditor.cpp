// Plugin
#include "PluginEditor.h"
#include "PluginProcessor.h"

PluginEditor::PluginEditor(PluginAudioProcessor& p)
  : AudioProcessorEditor(&p)
  , m_processor(p)
{
    setSize(800, 600);

    // Set up OpenGL rendering
    m_openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
    m_openGLContext.setRenderer(this);
    m_openGLContext.setContinuousRepainting(true);
    m_openGLContext.setComponentPaintingEnabled(true);
    m_openGLContext.attachTo(*this);

    // enable msaa
    m_openGLContext.setMultisamplingEnabled(true);
    juce::OpenGLPixelFormat pixelFormat;
    pixelFormat.multisamplingLevel = 8;
    m_openGLContext.setPixelFormat(pixelFormat);

    // Parameter window
    {
#ifdef PARAMETER_WINDOW
        m_parameterWindow = std::make_unique<ParameterWindow>(m_parameters);
#endif
    }
}

PluginEditor::~PluginEditor()
{
    m_openGLContext.detach();
}

void
PluginEditor::newOpenGLContextCreated()
{
    m_renderer = std::make_unique<Renderer>(m_processor, m_parameters);
}

void
PluginEditor::renderOpenGL()
{
    // A very naive but simple way of updating parameters by bruteforcing them before rendering
    // Ideally, just use a proper parameter manager that detects changes instead,
    // and only set state parameters here that don't cause any performance side-effects!
    m_processor.getSpectrexMiniProcessor().getProcessor().setSpectrogramAttack(m_parameters.attack_seconds);
    m_processor.getSpectrexMiniProcessor().getProcessor().setSpectrogramRelease(m_parameters.release_seconds);

    m_renderer->render(getWidth(), getHeight());
}

void
PluginEditor::openGLContextClosing()
{
    m_renderer.reset();
}
