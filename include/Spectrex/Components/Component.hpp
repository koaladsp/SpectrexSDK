#pragma once

// Spectrex
#include <Spectrex/Processing/Processor.hpp>
#include <Spectrex/Rendering/Context.hpp>

// GLM
#include <glm/glm.hpp>

// Stdlib
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

namespace spectrex {

/// Base interface class for audio visualization components.
class KComponent
{
  protected:
    /// Underlying type for DirectionFlag (unsigned integer).
    using Direction = uint32_t;
    /// Indicates direction for panning and zooming.
    enum DirectionFlag : Direction
    {
        Horizontal = (1 << 0),
        Vertical = (1 << 1)
    };

  public:
    /// Draws the component.
    virtual void draw() noexcept = 0;

    /// Get the position of this component that was most recently drawn.
    /// @return Position.
    virtual auto getPositionLastDrawn() const noexcept -> float = 0;

    /// Returns a custom string to describe the data in the component at a given normalized mouse position. Should be overriden for each individual
    /// type of Component.
    /// @param normX A normalized [0,1] mouse X position in the component. Does not account for zoom.
    /// @param normY A normalized [0,1] mouse Y position in the component. Does not account for zoom.
    /// @return A std::string describing the data being displayed at the given position.
    virtual auto getInfoTextForNormalizedPosition(float normX, float normY) const noexcept -> const std::string = 0;

    /// Returns the current mouse cursor position.
    /// @return Mouse cursor position.
    auto getCurrent() noexcept -> glm::vec2;

    /// Returns the minimum frequency, in case relevant for this component.
    /// @return Minimum frequency.
    auto getMinFrequency() const noexcept -> float;

    /// Returns the maximum frequency, in case relevant for this component.
    /// @return Maximum frequency.
    auto getMaxFrequency() const noexcept -> float;

    /// Returns the minimum dB, in case relevant for this component.
    /// @return Minimum dB.
    auto getMinDb() const noexcept -> float;

    /// Returns the maximum dB, in case relevant for this component.
    /// @return Maximum dB.
    auto getMaxDb() const noexcept -> float;

    /// Returns the number of musical bars displayed.
    /// @return The number of bars.
    auto getNumBars() const noexcept -> float;

    /// Returns the width of this component.
    /// @return Width.
    auto getWidth() const noexcept -> uint32_t;

    /// Returns the height of this component.
    /// @return Height.
    auto getHeight() const noexcept -> uint32_t;

    /// Returns the current rotation.
    /// @return Rotation.
    auto getRotation() const noexcept -> glm::vec3;

    /// Returns the current scale.
    /// @return Scale.
    auto getScale() const noexcept -> glm::vec3;

    /// Returns the X-position of this component.
    /// @return X-position.
    auto getX() const noexcept -> int32_t;

    /// Returns the Y-position of this component.
    /// @return Y-position.
    auto getY() const noexcept -> int32_t;

    /// Returns the the viewbox boundaries (taking into account zooming and panning) within [0, 1] range, where 0 is the actual left/bottom of the
    /// component and 1 is the actual right/top of the component.
    /// @return Normalized viewbox boundaries, order: left, right, bottom, top.
    auto getViewBox() const noexcept -> std::tuple<float, float, float, float>;

    /// Returns the current shift value.
    /// @return Shift value.
    auto getShift() const noexcept -> float;

    /// Sets the minimum frequency, in case relevant for this component.
    /// @param minFrequency Minimum frequency.
    void setMinFrequency(float minFrequency) noexcept;

    /// Sets the maximum frequency, in case relevant for this component.
    /// @param maxFrequency Maximum frequency.
    void setMaxFrequency(float maxFrequency) noexcept;

    /// Sets the minimum dB, in case relevant for this component.
    /// @param minDb Minimum dB.
    void setMinDb(float minDb) noexcept;

    /// Sets the maximum dB, in case relevant for this component.
    /// @param maxDb Maximum dB.
    void setMaxDb(float maxDb) noexcept;

    /// Sets the number of musical bars displayed.
    /// @param numBars The number of bars.
    void setNumBars(float numBars) noexcept;

    /// Sets the width of the component.
    /// @param width Width.
    void setWidth(uint32_t width) noexcept;

    /// Sets the height of the component.
    /// @param height Height.
    void setHeight(uint32_t height) noexcept;

    /// Sets the three-dimensional rotation of this component.
    /// @param z Z-rotation in radians.
    /// @param x X-rotation in radians.
    /// @param y Y-rotation in radians.
    void setRotation(float z, float x = 0.0f, float y = 0.0f);

    /// Sets the three-dimensional scale of this component.
    /// @param x X-scale.
    /// @param y Y-scale.
    /// @param z Z-scale.
    void setScale(float x, float y, float z);

    /// Sets the X-position of the component.
    /// @param x X-position.
    void setX(int32_t x);

    /// Sets the Y-position of the component.
    /// @param y Y-position.
    void setY(int32_t y);

    /// Sets the pan direction of the component.
    /// @param direction pan direction.
    void setPanDirection(Direction direction) noexcept;

    /// Sets the zoom direction of the component.
    /// @param direction Zoom direction.
    void setZoomDirection(Direction direction) noexcept;

    /// Sets the current shift. This is used only to set state after construction to properly maintain the users last state upon closing.
    /// @param shift The current shift value, should be in the range [-1,1]
    void setShift(float shift) noexcept;

    /// Called upon mouse move.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseMove(int32_t x, int32_t y) noexcept;

    /// Called upon mouse down.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseStart(int32_t x, int32_t y) noexcept;

    /// Called upon mouse double click.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseDoubleClick(int32_t x, int32_t y) noexcept;

    /// Called upon double click while holding shift.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseShiftDoubleClick(int32_t x, int32_t y) noexcept;

    /// Called upon mouse drag.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseDrag(int32_t x, int32_t y) noexcept;

    /// Called upon mouse drag when shift is held down.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseShiftDrag(int32_t x, int32_t y) noexcept;

    /// Called upon mouse up.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    void onMouseEnd(int32_t x, int32_t y) noexcept;

    /// Called upon mousewheel movement.
    /// @param x X-position of mouse event.
    /// @param y Y-position of mouse event.
    /// @param wheelDelta Wheel delta.
    void onMouseWheel(int32_t x, int32_t y, float wheelDelta) noexcept;

    virtual ~KComponent();

  protected:
    KComponent();

    //! @cond
    KDSP_IMPL(Component);
    KDSP_IMPL_VIRTUAL_INTERFACE(Component);
    //! @endcond
};

} // namespace spectrex
