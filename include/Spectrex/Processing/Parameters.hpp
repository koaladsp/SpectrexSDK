#pragma once

// Spectrex
#include <Spectrex/Processing/Parameters.hpp>
#include <Spectrex/Utility/Utility.hpp>

// GSL
#include <gsl/span>

// Stdlib
#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace spectrex {

/// Window parameter.
enum class Window
{
    WindowNone,
    WindowHann,
    WindowBlackman
};

/// MixMode parameter.
enum class MixMode
{
    Stereo,
    Left,
    Right,
    Mid,
    Side
};

/// Fourier Transform size (number of bins) parameter.
enum class FtSize
{
    Size256,
    Size512,

    /*** Enhanced features ***/
    /// @enhanced
    Size1024,
    Size2048,
    Size4096,
    Size8192
};

/// Returns the actual Fourier Transform size (number of bins), for an FtSize value.
/// @param ftSize FtSize value to return FT size for.
/// @return FT size.
inline auto
getFtSize(FtSize ftSize) noexcept -> uint32_t
{
    uint32_t ret = 0;

    switch (ftSize) {
        case FtSize::Size256:
            ret = 256;
            break;
        case FtSize::Size512:
            ret = 512;
            break;
        case FtSize::Size1024:
            ret = 1024;
            break;
        case FtSize::Size2048:
            ret = 2048;
            break;
        case FtSize::Size4096:
            ret = 4096;
            break;
        case FtSize::Size8192:
            ret = 8192;
            break;
        default:
            ret = 0;
            break;
    }

    // FtSize value not handled
    Ensures(ret != 0);

    return ret;
}

/// Returns the STFT stride, given the STFT overlap and FFT size.
/// @param ftSize FT size.
/// @param stftOverlap STFT overlap (domain [0, 1]).
/// @return STFT stride.
inline auto
getStftStride(FtSize ftSize, float stftOverlap) noexcept -> uint32_t
{
    Expects(stftOverlap >= 0.0f && stftOverlap <= 1.0f);

    return (uint32_t)((1.0f - stftOverlap) * getFtSize(ftSize));
}

/// KProcessor parameters class.
///
/// All parameters marked @user are changeable by the user, and can be
/// expected to be initialized/set to a default value elsewhere after this
/// constructor.
///
/// All parameters marked @host are set by the host application (e.g. DAW)
/// and may not always be initialized/set, and probably need to be listed
/// here in any case.
class ProcessorParameters
{
  public:
    /// Key type for a processor parameter.
    enum class Key
    {
        /// Fourier Transform size (number of bins) for spectrogram calculation.
        /// @user
        FtSize,

        /// Window function.
        /// @user
        Window,

        /// STFT Overlap.
        /// @user
        StftOverlap,

        /// BPM for window length calculation.
        /// @host
        Bpm,

        /// Time signature numerator for window length calculation.
        /// @host
        TimeSignatureNumerator,

        /// Time factor, can be a factor of bar length or factor of
        /// seconds.
        /// @user
        TimeFactor,

        /// Time multiplier.
        /// @user
        TimeMultiplier,

        /// Sample rate of input audio.
        /// @host
        SampleRate,

        /*** Enhanced features ***/

        /// Specifies whether or not the processor is in override mode.
        /// @user
        /// @enhanced
        Override,

        /// Specifies whether or not the processor is synced to the DAW
        /// playhead.
        /// @user
        /// @enhanced
        PlayHeadSynced,

        /// Mix mode, according to the MixMode type. Uses Mid by default.
        /// @user
        /// @enhanced
        MixMode,

        /// Specifies whether the graphs are rotated to a secondary
        /// layout
        /// @user
        /// @enhanced
        Rotate,

        /// Flattens the spectrum analyzer.
        /// @user
        /// @enhanced
        Flatten,

        ///
        End,
        First = FtSize
    };

  private:
    /// Represents an internal ProcessorParameters value. A Value can represent any underlying type.
    class Value final : public NonCopyable
    {
      public:
        /// Returns a flag indicating whether or not the type of this
        /// Value is equal to the template parameter.
        /// @tparam T Type to match.
        /// @return True if type matches, otherwise false.
        template<typename T>
        OPTNONE auto isType() const noexcept -> bool
        {
            return m_getType ? typeid(T) == m_getType() : false;
        }

        /// Returns a flag indicating whether or not this Value has a
        /// value set.
        /// @return True if this Value has a value set, otherwise false.
        OPTNONE bool hasValue() const noexcept { return m_data != nullptr; }

        /// Returns the value of this Value. Note that this template
        /// parameter (type) should match, otherwise the returned value is
        /// undefined.
        /// @tparam T Type of this Value.
        /// @return Value set.
        template<typename T>
        OPTNONE auto getValue() const noexcept -> const T&
        {
            static T None{};
            KASSERT(isType<T>(), "Unexpected type");
            return hasValue() ? *reinterpret_cast<T*>(m_data) : None;
        }

        /// Creates a null Value.
        OPTNONE Value()
          : m_getType([]() -> std::type_info const& { return typeid(nullptr); })
          , m_destroy([](void* data) { (void)data; })
          , m_data(nullptr)
        {
        }

        /// Creates a Value with a value set.
        /// @tparam T Type of value.
        /// @param value Value to set.
        template<typename T>
        OPTNONE explicit Value(const T& value) noexcept
          : m_getType([]() -> std::type_info const& { return typeid(T); })
          , m_destroy([](void* data) {
              if (data != nullptr) {
                  delete reinterpret_cast<T*>(data);
              }
          })
          , m_data(new T(value))
        {
        }

        /// Moves a Value into this object.
        /// @param other Value to consume.
        OPTNONE Value(Value&& other) noexcept
          : m_getType(std::move(other.m_getType))
          , m_destroy(std::move(other.m_destroy))
          , m_data(std::move(other.m_data))
        {
            // Reinitialize to null Value, so that any future destructor of a
            // moved instance will work properly
            other.m_getType = []() -> std::type_info const& { return typeid(nullptr); };
            other.m_destroy = [](void* data) { (void)data; };
            other.m_data = nullptr;
        }

        OPTNONE ~Value()
        {
            m_destroy(m_data);
            m_data = nullptr;
        }

      private:
        std::function<const std::type_info&()> m_getType;
        std::function<void(void*)> m_destroy;

        void* m_data;
    };

  public:
    /// Returns a flag, indicating whether or not this object contains any null values.
    ///
    /// If true is returned every value is valid, as no values are updated
    /// without being validated.
    ///
    /// @return True if no values are null, otherwise false.
    OPTNONE operator bool() const noexcept
    {
        return std::all_of(m_values.begin(), m_values.end(), [](const auto& pair) { return pair.second.hasValue(); });
    }

    /// Returns a flag indicating whether or not \a value for \a key contains a new value wrt. the value that is currently stored.
    /// @tparam T Type of value.
    /// @param key Key.
    /// @param v Value.
    /// @return True if value is new, false otherwise.
    template<typename T>
    OPTNONE auto hasNewValue(Key key, const T& v) const noexcept -> bool
    {
        if (!m_values.count(key)) {
            KASSERT(false,
                    "Value is expected to exist here, implementation error (key "
                    "is invalid)");

            return true;
        }

        const auto& currentValue = m_values.at(key);

        // If we currently have no value, any value is new
        if (!currentValue.hasValue()) {
            return true;
        }

        // Otherwise, we expect types to match, otherwise this is an
        // implementation error
        if (!currentValue.isType<T>()) {
            KASSERT(false, "Unexpected type, implementation error");

            return false;
        }

        const auto& a = currentValue.getValue<T>();
        const auto& b = v;

        // Finally, check the current value
        return a != b;
    }

    /// Returns a flag indicating whether or not a value exists for a \a key.
    /// @param key Key to query.
    /// @return True if a value exists, otherwise false.
    OPTNONE auto hasValue(Key key) const noexcept -> bool
    {
        if (!m_values.count(key)) {
            KASSERT(false,
                    "Value is expected to exist here, implementation error (key "
                    "is invalid)");

            return false;
        }

        return m_values.at(key).hasValue();
    }

    /// Get a value, based on a Key.
    /// @tparam T Return type.
    /// @param key Key.
    /// @return Value.
    template<typename T>
    OPTNONE auto getValue(Key key) const noexcept -> T
    {
        if (!hasValue(key)) {
            // No value set yet, return default value
            return T{};
        }

        auto& value = m_values.at(key);

        if (!value.isType<T>()) {
            KASSERT(false, "Unexpected type");

            return T{};
        }

        T t = value.getValue<T>();
        return t;
    }

    /// Set value for Key.
    ///
    /// The new value is validated, depending on \a key. Iff \a value is valid
    /// wrt. \a key, the value of \a key is updated.
    ///
    /// @tparam Value type.
    /// @param key Key.
    /// @param v Value.
    /// @return True if value is updated else false.
    template<typename T>
    OPTNONE auto setValue(Key key, const T& v) noexcept -> bool
    {
        KASSERT(m_values.count(key), "Implementation error (key is invalid)");

        auto value = Value{ v };

        // Check for validity of value
        if (!validate(key, value)) {
            return false;
        }

        // Erase
        m_values.erase(key);

        // Update
        m_values.emplace(key, std::move(value));

        return true;
    }

    /// Constructs a new ProcessorParameters object, initializing every parameter to a default (null) state.
    OPTNONE ProcessorParameters() noexcept;

  private:
    /// Returns a flag, indicating whether or not \a value is a valid value for \a key.
    /// @param key Key.
    /// @return True if valid, else false.
    OPTNONE auto validate(Key key, const Value& value) const noexcept -> bool
    {
        switch (key) {
            /* Spectrogram Parameters */
            case Key::FtSize: {
                return true;
            } break;
            case Key::Window: {
                return true;
            } break;
            case Key::StftOverlap: {
                if (!value.isType<float>()) {
                    return false;
                }

                const auto& stftOverlap = value.getValue<float>();
                return stftOverlap > 0.0f && stftOverlap < 1.0f;
            } break;
            /* Timing */
            case Key::Bpm: {
                if (!value.isType<float>()) {
                    return false;
                }

                const auto& bpm = value.getValue<float>();
                return bpm > 0.0f;
            } break;
            case Key::TimeSignatureNumerator: {
                if (!value.isType<int>()) {
                    return false;
                }

                const auto& timeSignatureNumerator = value.getValue<int>();
                return timeSignatureNumerator > 0;
            } break;
            case Key::TimeFactor: {
                if (!value.isType<float>()) {
                    return false;
                }

                const auto& timeFactor = value.getValue<float>();
                return timeFactor > 0.0f;
            } break;
            case Key::TimeMultiplier: {
                if (!value.isType<float>()) {
                    return false;
                }

                const auto& timeMultiplier = value.getValue<float>();
                return timeMultiplier > 0.0f;
            } break;
            case Key::SampleRate: {
                if (!value.isType<float>()) {
                    return false;
                }

                const auto& sampleRate = value.getValue<float>();
                return sampleRate > 0.0f;
            } break;
            /* Mode */
            case Key::Override: {
                return true;
            } break;
            case Key::PlayHeadSynced: {
                return true;
            } break;
            case Key::MixMode: {
                return true;
            } break;
            case Key::Rotate: {
                return true;
            } break;
            case Key::Flatten: {
                return true;
            } break;
            default: {
                // Key is not handled
                KASSERT(false, "Key is not handled");
            }
        }

        return false;
    }

  private:
    /// Parameter values.
    std::unordered_map<Key, Value> m_values;
};

inline ProcessorParameters::Key
operator++(ProcessorParameters::Key& x)
{
    return x = (ProcessorParameters::Key)(std::underlying_type<ProcessorParameters::Key>::type(x) + 1);
}

inline ProcessorParameters::Key
operator*(ProcessorParameters::Key c)
{
    return c;
}

inline ProcessorParameters::Key
begin(ProcessorParameters::Key r)
{
    (void)r;
    return ProcessorParameters::Key::First;
}

inline ProcessorParameters::Key
end(ProcessorParameters::Key r)
{
    (void)r;
    return ProcessorParameters::Key::End;
}

inline OPTNONE
ProcessorParameters::ProcessorParameters() noexcept
{
    for (const auto& key : Key()) {
        m_values[key];
    }

    // Explicitly initialize values that may not always be set
    // (required or visualization won't work)
    //
    // All parameters marked @user are changeable by the user, and can be
    // expected to be initialized/set to a default value elsewhere after this
    // constructor.
    //
    // All parameters marked @host are set by the host application (e.g. DAW)
    // and may not always be initialized/set, and probably need to be listed
    // here in any case.
    setValue<FtSize>(Key::FtSize, FtSize::Size4096);
    setValue<Window>(Key::Window, Window::WindowBlackman);
    setValue(Key::StftOverlap, 7.0f / 8.0f);
    setValue(Key::Bpm, 120.0f);               // @host
    setValue(Key::TimeSignatureNumerator, 1); // @host
    setValue(Key::TimeFactor, 1.0f);
    setValue(Key::TimeMultiplier, 1.0f);
    // SampleRate is not necessary and will by set by @host
    setValue(Key::Override, false);
    setValue(Key::PlayHeadSynced, false);
    setValue<MixMode>(Key::MixMode, MixMode::Mid);
    setValue(Key::Rotate, false);
    setValue(Key::Flatten, false);
}

} // namespace spectrex
