#pragma once

// Stdlib
#include <sstream>

namespace spectrex {

/// Generic exception class.
class Exception final
{
  public:
    /// Append an object to this exception's reason.
    /// @tparam T Type of object to add.
    /// @param t Object to add.
    /// @return This exception object.
    template<typename T>
    auto operator<<(const T& t) -> Exception&
    {
        std::stringstream stream;
        stream << m_reason << t;

        m_reason = stream.str();

        return *this;
    }

    /// Returns the reason the error was thrown as a string.
    /// @return Reason of error.
    auto getReason() const noexcept -> const std::string& { return m_reason; }

    /// Instantiate a new Error instance.
    /// @param reason Reason the Error is thrown.
    Exception(const std::string& reason)
      : m_reason(reason)
    {
    }

    ~Exception() = default;

  private:
    /// Reason for the error
    std::string m_reason;
};

} // namespace spectrex
