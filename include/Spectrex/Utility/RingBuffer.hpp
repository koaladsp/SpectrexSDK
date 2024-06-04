#pragma once

// spectrex
#include "Utility.hpp"

// Stdlib
#include <memory>
#include <vector>

namespace spectrex {

/// Generic circular ring buffer implementation.
template<typename T>
class RingBuffer final : public spectrex::NonCopyable
{
  public:
    /// Resets the ring buffer to its initial state and clears the internal buffer.
    void reset() noexcept
    {
        m_tail = m_head = 0;
        m_buffer.clear();
    }

    /// Gets the "previous" value before the head.
    void getPreviousValue(T& output) const { output = m_buffer[(m_head - 1) % m_capacity]; }

    /// Resets the ring buffer to its initial state without clearing the internal buffer.
    void resetIndices() noexcept { m_tail = m_head = 0; }

    /// Advances the ring buffer by the given number of elements.
    void advance(int offset) noexcept
    {
        if (m_capacity > 0) {
            m_head = (m_head + offset) % m_capacity;
        }
    }

    /// Skips the ring buffer by the given number of elements.
    void skip(int n) noexcept { m_tail = (m_tail + n) % m_capacity; }

    /// Pushes a new value into the ring buffer.
    void push(const T& value)
    {
        m_buffer[m_head] = value;
        m_head = (m_head + 1) % m_capacity;
    }

    /// Reads a number of elements from the ring buffer.
    void read(T* dst, int n) noexcept
    {
        const auto nCopy = std::min((size_t)(m_capacity - m_tail), (size_t)n);
        memcpy(dst, m_buffer.data() + m_tail, nCopy * sizeof(T));
        m_tail = (m_tail + nCopy) % m_capacity;

        if (n > 0) {
            n -= nCopy;
            read(dst + nCopy, n);
        }
    }

    /// Returns the number of available elements to read from the ring buffer.
    int getReadSpace() const noexcept
    {
        int ret;

        if (m_head > m_tail) {
            ret = m_head - m_tail;
        } else if (m_head < m_tail) {
            ret = (m_head + m_capacity) - m_tail;
        } else {
            ret = 0;
        }

        return ret;
    }

    /// Constructs a ring buffer.
    explicit RingBuffer(size_t capacity, T defaultValue)
      : m_capacity(capacity)
      , m_buffer(capacity, defaultValue)
      , m_head(0)
      , m_tail(0)
    {
    }

  private:
    const size_t m_capacity;

    std::vector<T> m_buffer;

    size_t m_head = 0;
    size_t m_tail = 0;
};

} // namespace spectrex
