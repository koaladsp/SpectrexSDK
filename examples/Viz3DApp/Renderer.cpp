// Plugin
#include "Renderer.h"

#include "Rendering.h"

#include "PluginProcessor.h"

#include "Shaders.h"

// GLAD
#include <glad/glad.h>

// GLM
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Spectrex
#include <Spectrex/Processing/Processor.hpp>

// Stdlib
#include <limits>

/* SpectrumLine */

const int k_spectrumPoints = 128; // NOTE: Needs to be equal to (spectrex::FtSize / 2 + 1) to avoid bins being missed in visualization!

class SpectrumLine : public RenderingObject
{
  public:
    SpectrumLine() noexcept
      : RenderingObject(RenderingResourceFactory::createVertexArrayResource())
    {
        // Define polyline vertices
        const int numPoints = k_spectrumPoints;
        const int numVertices = numPoints * 2;
        std::vector<Vertex> vertices;
        {
            vertices.resize(numVertices);
            for (auto i = 0; i < numPoints; ++i) {
                const auto t = float(i) * (1.0f / float(numPoints));

                // 1 polygon per line segment (line strip)
                // Y coordinates will be scaled in the vertex shader, to create polyline height
                vertices[i * 2 + 0] = Vertex{ glm::vec3{ -0.5f + t, -0.5f, 0.0f }, glm::vec3{}, glm::vec2{ t, 0.0f }, glm::vec3{} };
                vertices[i * 2 + 1] = Vertex{ glm::vec3{ -0.5f + t, +0.5f, 0.0f }, glm::vec3{}, glm::vec2{ t, 0.0f }, glm::vec3{} };
            }
        }

        // Upload data to the vertex buffer
        m_vertexArray->getVertexBuffer().upload(gsl::span<const Vertex>(vertices));
        setVertexCount(numVertices);

        setPrimitiveType(RenderingPrimitiveType::TriangleStrip);
    }

    ~SpectrumLine() = default;
};

class SpectrumPoint : public RenderingObject
{
  public:
    SpectrumPoint() noexcept
      : RenderingObject(RenderingResourceFactory::createVertexArrayResource())
    {
        std::vector<Vertex> vertices{ { glm::vec3{}, glm::vec3{}, glm::vec2{}, glm::vec3{} } };
        //        vertices.emplace_back();

        m_vertexArray->getVertexBuffer().upload(gsl::span<const Vertex>(vertices));
        setVertexCount(1);

        setPrimitiveType(RenderingPrimitiveType::Points);
    }

    ~SpectrumPoint() override = default;
};

int
getLatestRowPosition(spectrex::SpectrogramInfo info, int numInstances)
{
    // Number of spectrogram rows to accumulate per instance
    const int numRowsPerInstance = (int)info.Rows / numInstances;

    // Calculate the exact position of the latest row but discretized according to numRowsPerInstance
    //   discretize(x) = floor(x / info.Height) * info.Height
    // The above simply makes sure x is moved to the lower closest increment of numRowsInstance.
    //
    // Normally you would expect to be able to use a pointer that simply points to the last row index.
    // e.g. something like info.LastWrittenRow
    //
    // Instead, we use info.RowsWritten, an always increasing counter instead.
    // If we mod this with the buffer height, we will end up with the exact row index
    // at which the last row was written:
    //   info.RowsWritten % info.Height
    //
    // The reason we cannot use something like info.LastWrittenRow is because applying the modulus
    // causes an issue when discretizing:
    //
    //   discretize(info.LastWrittenRow) != (discretize(info.RowsWritten) % info.Height)
    //
    // The former messes up whenever a wraparound takes place, because it discretizes to zero,
    // while only the latter is correct. If there were no discretization, no issue would be present.
    return (int(info.RowsWritten / numRowsPerInstance) * numRowsPerInstance) % (int)info.Height;
}

/* Renderer */

void
Renderer::render(int width, int height) noexcept
{
    // Handle delta time
    double deltaTime = 0.0;
    {
        const auto currentTime = std::chrono::high_resolution_clock::now();

        // Nanoseconds
        deltaTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_lastTime).count();

        // Milliseconds
        m_time += deltaTime * 1.0e-9;

        m_lastTime = currentTime;
    }

    // spectrex
    spectrex::SpectrogramInfo info;
    {
        // Retrieve the processor
        spectrex::KProcessor& processor = m_processor.getSpectrexMiniProcessor().getProcessor();
        if (!processor.isValid()) {
            return;
        }

        // Beginning of frame
        processor.beginFrame();

        // Synchronize
        info = processor.getSpectrogramInfo();
        processor.syncSpectrogram([&](spectrex::SyncInfo<float> first, std::optional<spectrex::SyncInfo<float>> second_) {
            // Allocate the pixel buffer, will noop whenever size is
            // already equal to the requested size
            m_spectrogramBuffer->allocate((uint32_t)info.Width * (uint32_t)info.Height * sizeof(float), BufferUsageMode::DynamicDraw);

            // Ensure that the dimensions of the spectrogram texture
            // are set correctly
            // [bins, rows]
            m_spectrogramTexture->setDimensions((uint32_t)info.Width, (uint32_t)info.Height);

            // Do sanity check, make sure k_spectrumPoints is equal to (FFT/2 + 1) which is (info.Width + 1)
            assert((k_spectrumPoints + 1) == info.Width);

            // Map the pixel buffer, copy data into the mapped pointer
            m_spectrogramBuffer->mapBuffer(
              [=](void* ptr) {
#ifdef ENABLE_NVTX
                  const auto r3 = nvtx3::scoped_range{ "Spectrogram write" };
#endif // ENABLE_NVTX

                  jassert(ptr != nullptr);
                  if (ptr != nullptr) {
                      float* fptr = (float*)ptr;

                      // Clear the entire buffer if requested
                      if (first.Clear) {
                          std::memset(fptr, 0, info.Width * info.Height * sizeof(float));

                          return;
                      } else if (!first.isValid()) {
                          return;
                      }

                      // Copy first span to buffer
                      // The first span is always there: it contains any new data
                      std::memcpy(fptr + first.RowIndex * info.Width, first.Pointer, first.Width * first.Height * sizeof(float));

                      // Copy second part to buffer (if available)
                      // The second span is only occassional, but handles a case where the buffer wraps around to zero and starts from the beginning
                      if (second_) {
                          const auto& second = *second_;
                          std::memcpy(fptr + second.RowIndex * info.Width, second.Pointer, second.Width * second.Height * sizeof(float));
                      }
                  }
              },
              BufferAccess::WriteOnly,
              BufferUsageMode::DynamicDraw,
              false);
        });

        // With the pixel buffer bound, perform a transfer from
        // the pixel buffer to the texture
        m_spectrogramBuffer->bind();
        {
#ifdef ENABLE_NVTX
            const auto r4 = nvtx3::scoped_range{ "Spectrogram sync" };
#endif // ENABLE_NVTX

            m_spectrogramTexture->upload(nullptr, 0, 0, (uint32_t)info.Width, (uint32_t)info.Height);
        }
        m_spectrogramBuffer->unbind();
    }

    glEnable(GL_MULTISAMPLE);

    switch (m_parameters.visual) {
        case 0:
            visual_1(width, height, info);
            break;

        case 1:
            visual_2(width, height, info);
            break;

        case 2:
            visual_3(width, height, info);
            break;

        default:
            break;
    }

    glDisable(GL_MULTISAMPLE);
}

void
Renderer::visual_1(int width, int height, spectrex::SpectrogramInfo info)
{
    int numInstances = std::lroundf(m_parameters.num_lines);
    int offsetX = getLatestRowPosition(info, numInstances);

    // isometric camera
    glm::mat4 view;
    glm::mat4 projection;

    {
        float rotation = 3.14159265359f * 3 / 4;
        float distance = 5.4961f;

        auto cameraCenter = glm::vec3(distance);

        cameraCenter.x *= cosf(rotation);
        cameraCenter.z *= sin(rotation);
        cameraCenter.y = tanf(glm::radians(m_parameters.cam_angle)) * glm::length(glm::vec3(cameraCenter.x, 0, cameraCenter.z));

        auto cameraTarget = glm::vec3{ 0.0f, 0.0f, 0.0f };

        view = glm::lookAt(cameraCenter, cameraTarget, glm::vec3{ 0.0f, 1.0f, 0.0f });

        float p_width = ((float)width / (float)height) * (distance + m_parameters.cam_zoom);
        float p_height = 1.0f * (distance + m_parameters.cam_zoom);

        projection = glm::ortho(-p_width / 2.f, p_width / 2.f, -p_height / 2.f, p_height / 2.f, 0.01f, 100.0f);
    }

    RenderingHelper::clear(m_parameters.background_color.x, m_parameters.background_color.y, m_parameters.background_color.z, 1.f);

    m_program_1->use();
    {
        // Matrices
        m_program_1->set("uViewProjection", projection * view);

        // spectrex
        {
            // Attach textures as follows:
            //     Uniform - Unit
            //     Spectrogram  0
            m_program_1->set("uSpectrogram", 0);

            // Bind textures as specified above
            m_spectrogramTexture->bindToTextureUnit(0);

            // Frequency range as determined by the processor based on
            // sample rate, FFT window size
            m_program_1->set("uMinFrequency", info.MinFrequency);
            m_program_1->set("uMaxFrequency", info.MaxFrequency);

            // Desired frequency range as set by the user through a
            // parameter
            m_program_1->set("uMinDesiredFrequency", std::min(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));
            m_program_1->set("uMaxDesiredFrequency", std::max(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));

            // dB range
            m_program_1->set("uMinDb", std::min(m_parameters.min_db, m_parameters.max_db));
            m_program_1->set("uMaxDb", std::max(m_parameters.min_db, m_parameters.max_db));

            // Number of visible rows in the spectrogram (may be less, e.g. half of the actual texture size)
            m_program_1->set("uSpectrogramRows", (int)info.Rows);

            // Row index of the latest row in the spectrogram
            m_program_1->set("uSpectrogramLatestRow", offsetX);
        }

        // Line variables
        m_program_1->set("uLineThickness", m_parameters.line_thickness);
        m_program_1->set("uLineSpectrumHeight", m_parameters.height);

        m_program_1->set("uWidth", m_parameters.width);
        m_program_1->set("uLength", m_parameters.length);
        m_program_1->set("uGlobalScale", m_parameters.global_scale);
        m_program_1->set("uYDisplacement", m_parameters.y_displacement);
        m_program_1->set("uNumInstances", numInstances);

        // Color
        m_program_1->set("uLineColor1", m_parameters.color_1);
        m_program_1->set("uLineColor2", m_parameters.color_2);

        m_program_1->set("uGradientPosition", m_parameters.gradient_position);
        m_program_1->set("uGradientIntensity", m_parameters.gradient_intensity);

        // Render all disc geometries
        m_spectrum_geometry->setPrimitiveType(RenderingPrimitiveType::TriangleStrip);
        m_spectrum_geometry->drawInstanced(numInstances);
    }
    m_program_1->unuse();
}

void
Renderer::visual_2(int width, int height, spectrex::SpectrogramInfo info)
{
    int numInstances = std::lroundf(m_parameters.visual_2.num_lines);
    int offsetX = getLatestRowPosition(info, numInstances);

    glm::mat4 view;
    glm::mat4 projection;

    {
        auto cameraCenter = glm::vec3(0, 0, (m_parameters.visual_2.length / 2.f) * 1.25f);
        cameraCenter.y = tanf(glm::radians(5.f)) * glm::length(glm::vec3(cameraCenter.x, 0, cameraCenter.z));
        auto cameraTarget = glm::vec3{ 0.0f, 0.0f, 0.0f };

        view = glm::lookAt(cameraCenter, cameraTarget, glm::vec3{ 0.0f, 1.0f, 0.0f });

        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 100.0f);
    }

    RenderingHelper::clear(
      m_parameters.visual_2.background_color.x, m_parameters.visual_2.background_color.y, m_parameters.visual_2.background_color.z, 1.f);

    RenderingHelper::enableDefaultAlphaBlending();
    m_program_2->use();
    {
        // Matrices
        m_program_2->set("uViewProjection", projection * view);

        // spectrex
        {
            // Attach textures as follows:
            //     Uniform - Unit
            //     Spectrogram  0
            m_program_2->set("uSpectrogram", 0);

            // Bind textures as specified above
            m_spectrogramTexture->bindToTextureUnit(0);

            // Frequency range as determined by the processor based on
            // sample rate, FFT window size
            m_program_2->set("uMinFrequency", info.MinFrequency);
            m_program_2->set("uMaxFrequency", info.MaxFrequency);

            // Desired frequency range as set by the user through a
            // parameter
            m_program_2->set("uMinDesiredFrequency", std::min(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));
            m_program_2->set("uMaxDesiredFrequency", std::max(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));

            // dB range
            m_program_2->set("uMinDb", std::min(m_parameters.min_db, m_parameters.max_db));
            m_program_2->set("uMaxDb", std::max(m_parameters.min_db, m_parameters.max_db));

            // Number of visible rows in the spectrogram (may be less, e.g. half of the actual texture size)
            m_program_2->set("uSpectrogramRows", (int)info.Rows);

            // Row index of the latest row in the spectrogram
            m_program_2->set("uSpectrogramLatestRow", offsetX);
        }

        // Line variables
        m_program_2->set("uLineThickness", m_parameters.visual_2.line_thickness);
        m_program_2->set("uLineSpectrumHeight", m_parameters.visual_2.height);

        m_program_2->set("uWidth", m_parameters.visual_2.width);
        m_program_2->set("uLength", m_parameters.visual_2.length);
        m_program_2->set("uGlobalScale", 1.f);
        m_program_2->set("uYDisplacement", m_parameters.visual_2.y_displacement);
        m_program_2->set("uNumInstances", numInstances);

        // Color
        m_program_2->set("uLineColor1", m_parameters.visual_2.color_1);
        m_program_2->set("uLineColor2", m_parameters.visual_2.color_2);

        m_program_2->set("uGradientPosition", m_parameters.visual_2.gradient_position);
        m_program_2->set("uGradientIntensity", m_parameters.visual_2.gradient_intensity);

        // Render all disc geometries
        m_spectrum_geometry->setPrimitiveType(RenderingPrimitiveType::TriangleStrip);
        m_spectrum_geometry->drawInstanced(numInstances);
    }
    m_program_2->unuse();
    RenderingHelper::disableAlphaBlending();
}

void
Renderer::visual_3(int width, int height, spectrex::SpectrogramInfo info)
{
    int numInstances = std::lroundf(m_parameters.num_lines);
    int offsetX = getLatestRowPosition(info, numInstances);

    // isometric camera
    glm::mat4 view;
    glm::mat4 projection;

    {
        float rotation = 3.14159265359f * 3 / 4;
        float distance = 3.75f;

        auto cameraCenter = glm::vec3(distance);

        cameraCenter.x *= cosf(rotation);
        cameraCenter.z *= sin(rotation);
        cameraCenter.y = tanf(glm::radians(30.f)) * glm::length(glm::vec3(cameraCenter.x, 0, cameraCenter.z));

        auto cameraTarget = glm::vec3{ 0.0f, 0.0f, 0.0f };

        view = glm::lookAt(cameraCenter, cameraTarget, glm::vec3{ 0.0f, 1.0f, 0.0f });

        float p_width = ((float)width / (float)height) * (distance + m_parameters.cam_zoom);
        float p_height = 1.0f * (distance + m_parameters.cam_zoom);

        projection = glm::ortho(-p_width / 2.f, p_width / 2.f, -p_height / 2.f, p_height / 2.f, 0.01f, 100.0f);
    }

    RenderingHelper::clear(
      m_parameters.visual_3.background_color.x, m_parameters.visual_3.background_color.y, m_parameters.visual_3.background_color.z, 1.f);

    m_program_3->use();
    {
        // Matrices
        m_program_3->set("uViewProjection", projection * view);

        // spectrex
        {
            // Attach textures as follows:
            //     Uniform - Unit
            //     Spectrogram  0
            m_program_3->set("uSpectrogram", 0);

            // Bind textures as specified above
            m_spectrogramTexture->bindToTextureUnit(0);

            // Frequency range as determined by the processor based on
            // sample rate, FFT window size
            m_program_3->set("uMinFrequency", info.MinFrequency);
            m_program_3->set("uMaxFrequency", info.MaxFrequency);

            // Desired frequency range as set by the user through a
            // parameter
            m_program_3->set("uMinDesiredFrequency", std::min(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));
            m_program_3->set("uMaxDesiredFrequency", std::max(m_parameters.min_desired_frequency, m_parameters.max_desired_frequency));

            // dB range
            m_program_3->set("uMinDb", std::min(m_parameters.min_db, m_parameters.max_db));
            m_program_3->set("uMaxDb", std::max(m_parameters.min_db, m_parameters.max_db));

            // Number of visible rows in the spectrogram (may be less, e.g. half of the actual texture size)
            m_program_3->set("uSpectrogramRows", (int)info.Rows);

            // Row index of the latest row in the spectrogram
            m_program_3->set("uSpectrogramLatestRow", offsetX);
        }

        m_program_3->set("uXAmount", (int)std::lround(m_parameters.visual_3.x_amount));
        m_program_3->set("uZAmount", (int)std::lround(m_parameters.visual_3.z_amount));

        m_program_3->set("uBaseHeight", m_parameters.visual_3.base_height);
        m_program_3->set("uYDisplacement", -0.25f);
        m_program_3->set("uGlobalScale", 1.f);

        m_program_3->set("uLineSpectrumHeight", 1.f);

        // Render all disc geometries
        m_spectrum_geometry_2->drawInstanced(std::lround(m_parameters.visual_3.x_amount) * std::lround(m_parameters.visual_3.z_amount));
    }
    m_program_3->unuse();
}

Renderer::Renderer(PluginAudioProcessor& processor, Parameters& parameters) noexcept
  : m_processor(processor)
  , m_parameters(parameters)
{
    // Initialize extensions
    if (!gladLoadGL()) {
        jassertfalse;
        return;
    }

    // Compile/link shaders
    {
        {
            auto* vertexShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual1Vertex, ShaderType::Vertex);
            auto* fragmentShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual1Fragment, ShaderType::Fragment);

            m_program_1 = std::unique_ptr<Program>(RenderingResourceFactory::createProgramResource(*vertexShader, *fragmentShader));

            delete vertexShader;
            delete fragmentShader;
        }
        {
            auto* vertexShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual1Vertex, ShaderType::Vertex);
            auto* fragmentShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual2Fragment, ShaderType::Fragment);

            m_program_2 = std::unique_ptr<Program>(RenderingResourceFactory::createProgramResource(*vertexShader, *fragmentShader));

            delete vertexShader;
            delete fragmentShader;
        }
        {
            auto* vertexShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual3Vertex, ShaderType::Vertex);
            auto* geometryShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual3Geometry, ShaderType::Geometry);
            auto* fragmentShader = RenderingResourceFactory::createShaderResource(Plugin::Shaders::Visual3Fragment, ShaderType::Fragment);

            m_program_3 = std::unique_ptr<Program>(RenderingResourceFactory::createProgramResource(*vertexShader, *geometryShader, *fragmentShader));

            delete vertexShader;
            delete geometryShader;
            delete fragmentShader;
        }
    }

    // spectrex
    m_spectrogramBuffer = std::unique_ptr<Buffer>(RenderingResourceFactory::createBufferResource(BufferType::PixelUnpackBuffer));

    // Create texture resources, the initial dimensions can be zero
    // as the textures will be resized according to the data that
    // will be written into them
    m_spectrogramTexture = std::unique_ptr<Texture>(RenderingResourceFactory::createTextureResource(
      0, 0, TextureType::Texture2D, TextureFormat::R, TextureDataType::Float, TextureWrappingType::ClampToEdge, TextureFilteringType::Bilinear));

    // Set up sprite geometry
    m_spectrum_geometry = std::make_unique<SpectrumLine>();
    m_spectrum_geometry_2 = std::make_unique<SpectrumPoint>();

    // Set initial time
    m_lastTime = std::chrono::high_resolution_clock::now();
}

Renderer::~Renderer() {}