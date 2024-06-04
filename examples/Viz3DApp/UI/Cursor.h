#pragma once

// GLM
#include <glm/glm.hpp>

// Stdlib
#include <atomic>

/// @brief The cursor captures the position and zoom level of some the
/// visualizations.
class Cursor final
{
  public:
    /// @brief Returns a singleton instance.
    /// @return Global camera.
    static auto get() noexcept -> Cursor&;

    /// @brief Returns a copy of the position.
    /// @return Position.
    auto getPosition() const noexcept -> glm::vec2;

    /// @brief Returns a copy of the scale.
    /// @return Zoom.
    auto getZoom() const noexcept -> glm::vec2;

    /// @brief Gets the linear wheel vector.
    /// @return Linear wheel vector.
    auto getWheel() const noexcept -> glm::vec2;

    /// @brief Sets the position.
    /// @param position New position.
    void setPosition(glm::vec2 position) noexcept;

    /// @brief Sets the zoom.
    /// @param zoom New zoom.
    void setZoom(glm::vec2 zoom) noexcept;

    /// @brief Sets the linear wheel vector.
    /// @param wheel Linear wheel vector.
    void setWheel(glm::vec2 wheel) noexcept;

    /// @brief Resets the cursor to its initial state.
    void reset() noexcept;

  private:
    Cursor() noexcept;

  private:
    std::atomic<glm::vec2> m_position;

    std::atomic<glm::vec2> m_zoom;
    std::atomic<glm::vec2> m_wheel;
};
