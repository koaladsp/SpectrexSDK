#pragma once

// Spectrex
#include <Spectrex/Components/Component.hpp>
#include <Spectrex/Processing/Processor.hpp>
#include <Spectrex/Rendering/Context.hpp>

// GLM
#include <glm/glm.hpp>

// Stdlib
#include <memory>
#include <vector>

namespace spectrex {

/// KWaveformComponent properties struct.
struct WaveformComponentProperties
{
    /// Waveform background color.
    glm::vec4 BackgroundColor = { 0.012f, 0.012f, 0.012f, 1.00f };
};

/// KComponent class that visualizes a waveform.
class KWaveformComponent : public KComponent
{
  public:
    /// Draws this component.
    void draw() noexcept override;

    /// Return a mutable reference to the properties structure.
    /// @return Properties.
    auto getProperties() noexcept -> WaveformComponentProperties&;

    /// Get the position of this component that was most recently drawn.
    /// @return Position.
    auto getPositionLastDrawn() const noexcept -> float override;

    /// Get informational text for a given normalized position.
    /// @param normX Normalized X position.
    /// @param normY Normalized Y position.
    /// @return Information text string.
    auto getInfoTextForNormalizedPosition(float normX, float normY) const noexcept -> const std::string override;

    /// Sets the color ramp of the waveform.
    /// @param colorRamp Vector containing RGBA ramp
    /// colors. Vector size should be a multiple of four.
    void setColorRamp(const std::vector<float>& colorRamp) noexcept;

    /// Constructs a new Waveform component.
    /// @param processor Associated processor.
    /// @param context Associated context.
    KWaveformComponent(KProcessor& processor, KContext& context);

    ~KWaveformComponent();

  private:
    KDSP_IMPL(WaveformComponent)
    KDSP_IMPL_VIRTUAL(WaveformComponent);
};

} // namespace spectrex
