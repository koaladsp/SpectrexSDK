#include "WindowOpenGLContext.h"

// JUCE OpenGL DPI handling
#if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
// Include necessary for HWND definition and proper extern linking
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// Imports low level function from
// juce_gui_basics/native/juce_win32_Windowing.cpp
namespace juce {
extern JUCE_API double
getScaleFactorForWindow(HWND h);
}
#endif

namespace utility {

void
WindowOpenGLContext::setTopLevelParentComponent(juce::Component& topLevelComponent) noexcept
{
    // Only attack if no failure has occurred when creating the context
    if (!m_failed) {
        m_openGLContext.attachTo(topLevelComponent);
        // TODO: Can listen to top-level resized() calls to detect DPI changes
        // and call updateViewportSize accordingly without the user of a timer.
    }
}

void
WindowOpenGLContext::detachTopLevelParentComponent() noexcept
{
    m_openGLContext.detach();
}

void
WindowOpenGLContext::addRenderingTarget(juce::OpenGLRenderer* newTarget) noexcept
{
    jassert(dynamic_cast<juce::Component*>(newTarget) != nullptr);

    // Only call newOpenGLContextCreated is this already has been called before (not guaranteed, e.g. during validation)
    if (m_openGLContextCreated) {
        executeOnGLThread([newTarget](juce::OpenGLContext&) { newTarget->newOpenGLContextCreated(); });
    }

    const auto lock = std::lock_guard<std::mutex>(m_renderingTargetsLock);

    m_renderingTargets.add(newTarget);
}

void
WindowOpenGLContext::removeRenderingTarget(juce::OpenGLRenderer* targetToRemove) noexcept
{
    jassert(m_renderingTargets.contains(targetToRemove));

    executeOnGLThread([targetToRemove](juce::OpenGLContext&) { targetToRemove->openGLContextClosing(); });

    const auto lock = std::lock_guard<std::mutex>(m_renderingTargetsLock);

    m_renderingTargets.removeFirstMatchingValue(targetToRemove);
}

auto
WindowOpenGLContext::getViewportScale() noexcept -> double
{
    return m_viewportScale;
}

void
WindowOpenGLContext::executeOnGLThread(std::function<void(juce::OpenGLContext&)>&& lambdaToExecute) noexcept
{
    const auto lock = std::lock_guard<std::mutex>(m_executeInRenderCallbackLock);

    m_executeInRenderCallback.emplace_back(lambdaToExecute);
}

void
WindowOpenGLContext::executeOnGLThreadMultipleTimes(std::function<void(juce::OpenGLContext&)>&& lambdaToExecute, const int repetitions) noexcept
{
    const auto lock = std::lock_guard<std::mutex>(m_executeInRenderCallbackLock);

    for (int i = 0; i < repetitions; ++i) {
        m_executeInRenderCallback.push_back(lambdaToExecute);
    }
}

void
WindowOpenGLContext::setFailureCallback(std::function<void()> callback) noexcept
{
    m_failureCallback = callback;
}

void
WindowOpenGLContext::setBeginFrameCallback(std::function<void()> callback) noexcept
{
    m_beginFrameCallback = callback;
}

auto
WindowOpenGLContext::updateViewportSize(juce::Component* targetComponent) noexcept -> juce::Rectangle<int>
{
    using namespace juce;

    // @thread ui
    JUCE_ASSERT_MESSAGE_THREAD

    // Global render target component
    // Peer doesn't have to be assigned yet
    juce::Component* _component = m_openGLContext.getTargetComponent();
    if (_component == nullptr) {
        return juce::Rectangle<int>();
    }
    juce::Component& component = *_component;

    if (auto* peer = component.getPeer()) {
        const auto displayScale = Desktop::getInstance().getDisplays().getDisplayForRect(component.getTopLevelComponent()->getScreenBounds())->scale;

        auto localBounds = component.getLocalBounds();
        auto newArea = peer->getComponent().getLocalArea(&component, localBounds).withZeroOrigin() * displayScale;

#if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
        auto newScale = getScaleFactorForWindow((HWND)peer->getNativeHandle());
        auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

        if (!juce::approximatelyEqual(1.0f, desktopScale))
            newScale *= desktopScale;
#else
        auto newScale = displayScale;
#endif

        // Transform is only accessed when the message manager is locked
        auto transform = AffineTransform::scale((float)newArea.getWidth() / (float)localBounds.getWidth(),
                                                (float)newArea.getHeight() / (float)localBounds.getHeight());

        // Update scale
        m_viewportScale = newScale;

        // Target component area relative to render target component
        // Scaled by transform (DPI)
        juce::Rectangle<int> targetComponentArea =
          component.getLocalArea(targetComponent, targetComponent->getLocalBounds()).transformedBy(transform);

        // Flip Y because the renderer uses inversed Y and JUCE doesn't
        targetComponentArea.setY(newArea.getHeight() - targetComponentArea.getHeight() - targetComponentArea.getY());

        return targetComponentArea;
    }
    // Invalid
    return juce::Rectangle<int>();
}

auto
WindowOpenGLContext::getContext() noexcept -> juce::OpenGLContext&
{
    return m_openGLContext;
}

WindowOpenGLContext::WindowOpenGLContext() noexcept
  : m_viewportScale(1.0)
{
    juce::OpenGLPixelFormat pixelFmt(8, 8, 16, 8);

    // Request MSAA 4x
    pixelFmt.multisamplingLevel = 4;

    // Request OpenGL 4.3 core profile
    m_openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_3);

    m_openGLContext.setPixelFormat(pixelFmt);
    m_openGLContext.setMultisamplingEnabled(true);
    m_openGLContext.setSwapInterval(1);

    // Enable JUCE GL rendering
    m_openGLContext.setRenderer(this);
}

WindowOpenGLContext::~WindowOpenGLContext()
{
    jassert(m_renderingTargets.size() == 0);
}

void
WindowOpenGLContext::newOpenGLContextCreated() noexcept
{
    // Load GL extensions and check GL version
    m_failed = !spectrex::KContext::initializeGL();

    if (!m_failed) {
        // Create our own context
        m_visualizationContext = std::make_unique<spectrex::KContext>();
    } else {
        // Could not initialize GL or GL version too low
        //
        // Detach to stop rendering and crashes in JUCE due to missing GL
        // functionality, but call on message thread to avoid deadlock in JUCE.
        juce::MessageManager::callAsync([this] {
            detachTopLevelParentComponent();

            // Also call the failure callback
            if (m_failureCallback) {
                m_failureCallback();
            }
        });
    }

    // Mark as created
    m_openGLContextCreated = true;

    // Call newOpenGLContextCreated on all render targets
    {
        auto lock = std::lock_guard<std::mutex>(m_renderingTargetsLock);
        for (auto* target : m_renderingTargets) {
            target->newOpenGLContextCreated();
        }
    }
}

void
WindowOpenGLContext::renderOpenGL() noexcept
{
    m_timer.start();

    // Wait for valid context
    if (m_visualizationContext == nullptr) {
        return;
    }

    // Wait for valid dimensions
    if (!m_openGLContext.getTargetComponent()) {
        return;
    }

    // Execute render tasks
    {
        auto lock = std::lock_guard<std::mutex>(m_executeInRenderCallbackLock);
        for (auto& glThreadJob : m_executeInRenderCallback) {
            glThreadJob(m_openGLContext);
        }

        m_executeInRenderCallback.clear();
    }

    // Begin frame callback
    if (m_beginFrameCallback) {
        m_beginFrameCallback();
    }

    // Perform rendering
    {
        auto lock = std::lock_guard<std::mutex>(m_renderingTargetsLock);
        for (auto* target : m_renderingTargets) {
            auto* component = dynamic_cast<juce::Component*>(target);
            if (component->isVisible()) {
                target->renderOpenGL();
            }
        }
    }

    m_timer.stop();
}

void
WindowOpenGLContext::openGLContextClosing() noexcept
{
    // Clean up our own context
    m_visualizationContext.reset();

    // Mark as not created
    m_openGLContextCreated = false;
}

auto
WindowOpenGLContext::getVisualizationContext() noexcept -> spectrex::KContext&
{
    return *m_visualizationContext;
}

auto
WindowOpenGLContext::getFrameTimeInMs() const noexcept -> double
{
    return m_timer.getTimeInMs();
}

} // namespace utility
