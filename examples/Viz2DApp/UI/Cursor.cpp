#include "Cursor.h"

auto
Cursor::get() noexcept -> Cursor&
{
    static Cursor ret;
    return ret;
}

auto
Cursor::getPosition() const noexcept -> glm::vec2
{
    return m_position;
}

auto
Cursor::getZoom() const noexcept -> glm::vec2
{
    return m_zoom;
}

auto
Cursor::getWheel() const noexcept -> glm::vec2
{
    return m_wheel;
}

void
Cursor::setPosition(glm::vec2 position) noexcept
{
    m_position = position;
}

void
Cursor::setZoom(glm::vec2 zoom) noexcept
{
    m_zoom = zoom;
}

void
Cursor::setWheel(glm::vec2 wheel) noexcept
{
    m_wheel = wheel;
}

void
Cursor::reset() noexcept
{
    m_position = glm::vec2{ 0.0f, 0.0f };
    m_zoom = glm::vec2{ 1.0f, 1.0f };
    m_wheel = glm::vec2{ 0.0f, 0.0f };
}

Cursor::Cursor() noexcept
{
    reset();
}
