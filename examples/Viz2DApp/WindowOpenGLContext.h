#pragma once

// Plugin
#include "Utility.h"

// Spectrex
#include <Spectrex/Rendering/Context.hpp>

// JUCE
#include <juce_opengl/juce_opengl.h>

// Stdlib
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

namespace utility {

class WindowOpenGLContext : private juce::OpenGLRenderer
{
  public:
    void setTopLevelParentComponent(juce::Component& topLevelComponent) noexcept;

    void detachTopLevelParentComponent() noexcept;

    void addRenderingTarget(juce::OpenGLRenderer* newTarget) noexcept;

    void removeRenderingTarget(juce::OpenGLRenderer* targetToRemove) noexcept;

    auto getViewportScale() noexcept -> double;

    void executeOnGLThread(std::function<void(juce::OpenGLContext&)>&& lambdaToExecute) noexcept;

    void executeOnGLThreadMultipleTimes(std::function<void(juce::OpenGLContext&)>&& lambdaToExecute, const int repetitions) noexcept;

    void setBeginFrameCallback(std::function<void()> callback) noexcept;
    void setFailureCallback(std::function<void()> callback) noexcept;

    auto getContext() noexcept -> juce::OpenGLContext&;

    auto isFailed() const noexcept -> bool { return m_failed; }

    // Our own context
    auto getVisualizationContext() noexcept -> spectrex::KContext&;

    auto getFrameTimeInMs() const noexcept -> double;

    // @thread ui
    // @return Clipping bounds relative to GL render target.
    auto updateViewportSize(juce::Component* targetComponent) noexcept -> juce::Rectangle<int>;

    WindowOpenGLContext() noexcept;
    ~WindowOpenGLContext();

  private:
    void newOpenGLContextCreated() noexcept override;

    void renderOpenGL() noexcept override;

    void openGLContextClosing() noexcept override;

  private:
    /* Properties */

    bool m_failed = false;

    double m_viewportScale;

    MovingAverageTimer m_timer;

    /* Context */

    juce::OpenGLContext m_openGLContext;

    std::atomic<bool> m_openGLContextCreated = false;

    /* Render Targets */

    juce::Array<juce::OpenGLRenderer*> m_renderingTargets;

    /* Mutex */

    std::mutex m_renderingTargetsLock;

    std::mutex m_executeInRenderCallbackLock;

    /* Callback */

    std::vector<std::function<void(juce::OpenGLContext&)>> m_executeInRenderCallback;

    std::function<void()> m_beginFrameCallback;
    std::function<void()> m_failureCallback;

    std::unique_ptr<spectrex::KContext> m_visualizationContext;
};

} // namespace utility
