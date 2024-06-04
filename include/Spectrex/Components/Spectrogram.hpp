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

/// KSpectrogramComponent properties struct.
struct SpectrogramComponentProperties
{
    /// Spectrogram background color.
    glm::vec4 BackgroundColor = { 0.012f, 0.012f, 0.012f, 1.00f };
};

/// KComponent class that visualizes a 2D spectrogram.
class KSpectrogramComponent : public KComponent
{
  public:
    /// Draws this component.
    void draw() noexcept override;

    /// Get the position of this component that was most recently drawn.
    /// @return Position.
    auto getPositionLastDrawn() const noexcept -> float override;

    /// Return a mutable reference to the properties structure.
    /// @return Properties.
    auto getProperties() noexcept -> SpectrogramComponentProperties&;

    /// Returns the current color ramp.
    /// @return Color ramp data.
    auto getColorRamp() const noexcept -> const std::vector<float>&;

    /// Sets the color ramp of the spectrogram.
    /// @param colorRamp Vector containing RGBA ramp
    /// colors. Vector size should be a multiple of four.
    void setColorRamp(const std::vector<float>& colorRamp) noexcept;

    /// Get informational text for a given normalized position.
    /// @param normX Normalized X position.
    /// @param normY Normalized Y position.
    /// @return Information text string.
    auto getInfoTextForNormalizedPosition(float normX, float normY) const noexcept -> const std::string override;

    /// Constructs a new Spectrogram component.
    /// @param processor Associated processor.
    /// @param context Associated context.
    KSpectrogramComponent(KProcessor& processor, KContext& context);

    ~KSpectrogramComponent();

  private:
    KDSP_IMPL(SpectrogramComponent)
    KDSP_IMPL_VIRTUAL(SpectrogramComponent);
};

} // namespace spectrex
