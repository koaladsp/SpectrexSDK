#pragma once

// Optimization macros
#ifndef OPTNONE
#if defined(WIN32)
#define OPTNONE
#elif defined(__APPLE__)
#define OPTNONE __attribute__((optnone))
#endif
#endif

// Assertion macros
#include <assert.h>
#ifndef KASSERT
#define KASSERT(condition, msg) assert(condition)
#endif

namespace spectrex {

/// Utility class to enforce non-copyable semantics.
struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace spectrex
