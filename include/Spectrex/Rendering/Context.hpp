#pragma once

// Spectrex
#include <Spectrex/extra.h>

namespace spectrex {

/// Rendering context class.
class KContext
{
  public:
    /// One-time initialization for OpenGL. Can be called multiple times, only performs initialization once. Only call from GL thread.
    /// @thread gl
    static bool initializeGL() noexcept;

    KContext();
    ~KContext();

  private:
    KDSP_IMPL(KContext)
};

} // namespace spectrex
