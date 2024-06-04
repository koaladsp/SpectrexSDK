#pragma once

// Spectrex
#include <Spectrex/Processing/Parameters.hpp>

// GLM
#include <glm/glm.hpp>

// Stdlib
#include <list>
#include <string>

struct Parameters
{
    //
    // Listener functionality
    //
    struct Listener
    {
        virtual void parameterChanged(const Parameters& parameters, const std::string& name) noexcept = 0;
    };

  private:
    std::list<Listener*> m_listeners;

  public:
    void addListener(Listener* listener) noexcept
    {
        if (listener != nullptr) {
            m_listeners.push_back(listener);
        }
    }
    void removeListener(Listener* listener) noexcept
    {
        if (listener != nullptr) {
            m_listeners.remove(listener);
        }
    }
    void onParameterChanged(const std::string& name) noexcept
    {
        for (auto p : m_listeners) {
            p->parameterChanged(*this, name);
        }
    }
    void all() noexcept
    {
        // Launch callbacks for all the available parameters
        const std::string params[] = { "pause",  "min_frequency", "max_frequency",   "min_db", "max_db",
                                       "window", "stft_overlap",  "time_multiplier", "mix",    "ft_size" };
        for (const auto p : params) {
            onParameterChanged(p);
        }
    }

    //
    // Parameters
    //

    /* spectrogram */
    bool pause = false;

    float min_frequency = 100.f;
    float max_frequency = 20000.f;

    float min_db = -48.f;
    float max_db = -6.f;

    // Also see ProcessorParameters::ProcessorParameters()
    spectrex::Window window = spectrex::Window::WindowBlackman;
    float stft_overlap = 7.0f / 8.0f;
    float time_multiplier = 1.0f;
    spectrex::MixMode mix = spectrex::MixMode::Mid;
    spectrex::FtSize ft_size = spectrex::FtSize::Size512;
};
