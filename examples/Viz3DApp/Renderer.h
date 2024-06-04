#pragma once

// Plugin
#include "Parameters.h"

// GLM
#include <glm/glm.hpp>

// JUCE
#include <juce_core/juce_core.h>

// spectrex
#include <Spectrex/Processing/Processor.hpp>

// Stdlib
#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

/* Forward Declarations */

class PluginAudioProcessor;

class Program;
class OrbitCamera;
class RenderTarget;
class Rectangle;
class Texture;
class Buffer;

/* Renderer */

class Renderer final
{
  public:
    void render(int width, int height) noexcept;

    explicit Renderer(PluginAudioProcessor& processor, Parameters& parameters) noexcept;

    ~Renderer();

  private:
    double m_time = 0.0;

    std::chrono::high_resolution_clock::time_point m_lastTime;

    Parameters& m_parameters;

    // Processor
    PluginAudioProcessor& m_processor;

    // Rendering resources
    std::unique_ptr<Program> m_program_1;
    std::unique_ptr<Program> m_program_2;
    std::unique_ptr<Program> m_program_3;

    std::unique_ptr<class SpectrumLine> m_spectrum_geometry;
    std::unique_ptr<class SpectrumPoint> m_spectrum_geometry_2;

    // Spectrex
    std::unique_ptr<Buffer> m_spectrogramBuffer;
    std::unique_ptr<Texture> m_spectrogramTexture;

    void visual_1(int width, int height, spectrex::SpectrogramInfo info);

    void visual_2(int width, int height, spectrex::SpectrogramInfo info);

    void visual_3(int width, int height, spectrex::SpectrogramInfo info);
};