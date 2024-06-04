#include "Rendering.h"

// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// JUCE
#include <juce_core/juce_core.h>

/* Static zero buffer for clearing textures */

static uint8_t*
getZeroBuffer(const size_t length)
{
    // Ensure buffer size
    static std::vector<uint8_t> g_zeroData;
    if (g_zeroData.size() < length) {
        g_zeroData.resize(length);
        std::fill(g_zeroData.begin(), g_zeroData.end(), (uint8_t)0);
    }
    return g_zeroData.data();
}

/* Transform */

auto
Transform::getModelMatrix() const noexcept -> glm::mat4
{
    glm::mat4 m = glm::mat4(1.0);
    m = glm::translate(m, m_position);
    m = glm::rotate(m, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, m_scale);
    return m;
}

auto
Transform::getPosition() noexcept -> glm::vec3&
{
    return m_position;
}

auto
Transform::getRotation() noexcept -> glm::vec3&
{
    return m_rotation;
}

auto
Transform::getScale() noexcept -> glm::vec3&
{
    return m_scale;
}

Transform::Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) noexcept
  : m_position(position)
  , m_rotation(rotation)
  , m_scale(scale)
{
}

/* OrthoCamera */

auto
OrthoCamera::getViewProjection(int64_t width, int64_t height) noexcept -> glm::mat4
{
    const auto w_2 = width / 2.0f;
    const auto h_2 = height / 2.0f;

    const auto center = m_position;
    const auto eye = center + glm::vec3{ 0.0f, 0.0f, 0.5f };

    // clang-format off
    return glm::ortho(-w_2 * m_zoom.x, w_2 * m_zoom.x, //   L R
                      -h_2 * m_zoom.y, h_2 * m_zoom.y, //   B T
                      -1.0f, 1.0f                      //   N F
                      )                                // Projection
           * glm::lookAt(eye,                          //   Eye
                         center,                       //   Center
                         glm::vec3{0.0f, 1.0f, 0.0f}   //   Up
           );                                          // View
    // clang-format on
}

auto
OrthoCamera::getPosition() noexcept -> glm::vec3&
{
    return m_position;
}

auto
OrthoCamera::getUp() noexcept -> glm::vec3&
{
    return m_up;
}

auto
OrthoCamera::getZoom() noexcept -> glm::vec2&
{
    return m_zoom;
}

OrthoCamera::OrthoCamera(glm::vec3 position, glm::vec3 up, glm::vec2 zoom) noexcept
  : m_position(position)
  , m_up(up)
  , m_zoom(zoom)
{
}

/* OrbitCamera */

auto
OrbitCamera::getProjection(int64_t width, int64_t height) noexcept -> glm::mat4
{
    return glm::perspective(glm::radians(m_fov),          // FOV
                            (float)width / (float)height, // Aspect
                            m_near,                       // Near
                            m_far                         // Far
    );
}

auto
OrbitCamera::getView() noexcept -> glm::mat4
{
    return glm::lookAt(m_position,                   //   Eye
                       m_target,                     //   Center
                       glm::vec3{ 0.0f, 1.0f, 0.0f } //   Up
    );
}

auto
OrbitCamera::getPosition() noexcept -> glm::vec3&
{
    return m_position;
}

auto
OrbitCamera::getTarget() noexcept -> glm::vec3&
{
    return m_target;
}

auto
OrbitCamera::getFov() noexcept -> float&
{
    return m_fov;
}

auto
OrbitCamera::getNear() noexcept -> float&
{
    return m_near;
}

auto
OrbitCamera::getFar() noexcept -> float&
{
    return m_far;
}

auto
OrbitCamera::getUp() noexcept -> glm::vec3&
{
    return m_up;
}

OrbitCamera::OrbitCamera(glm::vec3 position, glm::vec3 target, float fov, float near, float far, glm::vec3 up) noexcept
  : m_position(position)
  , m_target(target)
  , m_fov(fov)
  , m_near(near)
  , m_far(far)
  , m_up(up)
{
}

/* Texture */

void
Texture::bind() const noexcept
{
    glBindTexture(getTarget(), getId());
}

void
Texture::unbind() const noexcept
{
    glBindTexture(getTarget(), 0);
}

void
Texture::bindToTextureUnit(uint32_t unit) const noexcept
{
    jassert(unit >= 0 && unit < 80); // Invalid texture unit

    glActiveTexture(GL_TEXTURE0 + unit);
    bind();
}

void
Texture::clear() noexcept
{
    // Ensure zero data vector is allocated because glClearTexSubImage doesn't
    // exist in this version of OpenGL
    const auto clearSize = m_width * m_height * getStride();

    bind();
    {
        switch (m_type) {
            // Texture 2D
            case TextureType::Texture2D: {
                glTexImage2D(getTarget(),             // Target
                             0,                       // Level
                             getInternalFormat(),     // Internal format
                             (GLsizei)m_width,        // Width
                             (GLsizei)m_height,       // Height
                             0,                       // Border
                             getFormat(),             // Format
                             getDataType(),           // Type
                             getZeroBuffer(clearSize) // Data
                );
            } break;
        }
    }
    unbind();
}

void
Texture::upload(const uint8_t* data, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    // Upload texture data
    bind();
    {
        switch (m_type) {
            // Texture 2D
            case TextureType::Texture2D: {
                glTexSubImage2D(getTarget(), 0, x, y, (GLsizei)width, (GLsizei)height, getFormat(), getDataType(), data);
            } break;
        }

        if (m_hasMipMaps) {
            glGenerateMipmap(getTarget());
        }
    }
    unbind();
}

auto
Texture::getWidth() const noexcept -> uint32_t
{
    return m_width;
}

auto
Texture::getHeight() const noexcept -> uint32_t
{
    return m_height;
}

auto
Texture::getStride() const noexcept -> uint32_t
{
    auto stride = 0;

    switch (m_dataType) {
        case TextureDataType::Float:
            stride = sizeof(GLfloat);
            break;
        case TextureDataType::UnsignedByte:
            stride = sizeof(GLubyte);
            break;
        default:
            jassertfalse; // Not implemented
            return GL_NONE;
    }

    switch (m_format) {
        case TextureFormat::R:
            // 1
            break;
        case TextureFormat::RG:
            stride *= 2;
            break;
        case TextureFormat::RGB:
            stride *= 3;
            break;
        case TextureFormat::RGBA:
            stride *= 4;
            break;
        default:
            jassertfalse; // Not implemented
    }

    return stride;
}

void
Texture::setTextureWrappingType(TextureWrappingType type) noexcept
{
    m_wrappingType = type;

    // Acquire the target
    const auto target = getTarget();

    // Set the wrapping mode of the texture object
    bind();
    {
        switch (m_type) {
            case TextureType::Texture2D: {
                switch (m_wrappingType) {
                    // Clamp to edge
                    case TextureWrappingType::ClampToEdge:
                        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        break;
                    default:
                        jassertfalse; // Not implemented
                        break;
                }
            } break;
            default:
                jassertfalse; // Not implemented
                break;
        }
    }
    unbind();
}

void
Texture::setTextureFilteringType(TextureFilteringType type) noexcept
{
    m_filteringType = type;

    // Acquire the target
    const auto target = getTarget();

    // Set the filtering mode of the texture object
    bind();
    {

        switch (m_type) {
            case TextureType::Texture2D: {
                switch (m_filteringType) {
                    // Nearest (point) filtering
                    case TextureFilteringType::Nearest:
                        if (m_hasMipMaps) {
                            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                        } else {
                            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        }
                        break;
                    // Bilinear filtering
                    case TextureFilteringType::Bilinear:
                        if (m_hasMipMaps) {
                            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        } else {
                            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        }
                        break;
                    default:
                        jassertfalse; // Not implemented
                        break;
                }
            } break;
            default:
                jassertfalse; // Not implemented
                break;
        }
    }
    unbind();
}

void
Texture::setDimensions(uint32_t width, uint32_t height) noexcept
{
    // Nothing to do
    if (width == m_width && height == m_height) {
        return;
    }

    m_width = width;
    m_height = height;

    // Acquire the target
    const auto target = getTarget();

    // Changing dimensions requires (re-)allocation of the texture data
    bind();
    {
        switch (m_type) {
            // Texture 2D
            case TextureType::Texture2D: {
                glTexImage2D(target,              // Target
                             0,                   // Level
                             getInternalFormat(), // Internal format
                             (GLsizei)m_width,    // Width
                             (GLsizei)m_height,   // Height
                             0,                   // Border
                             getFormat(),         // Format
                             getDataType(),       // Type
                             nullptr              // Data
                );
            } break;
            default:
                jassertfalse; // Not implemented
                break;
        }
    }
    unbind();

    // Zero texture data
    clear();
}

Texture::~Texture()
{
    GLuint id = getId();
    glDeleteTextures(1, &id);
}

auto
Texture::getTarget() const noexcept -> GLenum
{
    switch (m_type) {
        case TextureType::Texture2D:
            return GL_TEXTURE_2D;
        default:
            jassertfalse; // Not implemented
            return GL_NONE;
    }
}

auto
Texture::getInternalFormat() const noexcept -> GLenum
{
    switch (m_dataType) {
        case TextureDataType::Float: {
            switch (m_format) {
                case TextureFormat::R:
                    return GL_R32F;
                case TextureFormat::RG:
                    return GL_RG32F;
                case TextureFormat::RGB:
                    return GL_RGB32F;
                case TextureFormat::RGBA:
                    return GL_RGBA32F;
                default:
                    jassertfalse; // Not implemented
                    return GL_NONE;
            }
        } break;
        case TextureDataType::UnsignedByte: {
            switch (m_format) {
                case TextureFormat::R:
                    return GL_R8;
                case TextureFormat::RG:
                    return GL_RG8;
                case TextureFormat::RGB:
                    return GL_RGB8;
                case TextureFormat::RGBA:
                    return GL_RGBA8;
                default:
                    jassertfalse; // Not implemented
                    return GL_NONE;
            }
        } break;
        default:
            jassertfalse; // Not implemented
            return GL_NONE;
    }
}

auto
Texture::getFormat() const noexcept -> GLenum
{
    switch (m_format) {
        case TextureFormat::R:
            return GL_RED;
        case TextureFormat::RG:
            return GL_RG;
        case TextureFormat::RGB:
            return GL_RGB;
        case TextureFormat::RGBA:
            return GL_RGBA;
        default:
            jassertfalse; // Not implemented
            return GL_NONE;
    }
}

auto
Texture::getDataType() const noexcept -> GLenum
{
    switch (m_dataType) {
        case TextureDataType::Float:
            return GL_FLOAT;
        case TextureDataType::UnsignedByte:
            return GL_UNSIGNED_BYTE;
        default:
            jassertfalse; // Not implemented
            return GL_NONE;
    }
}

auto
Texture::construct() -> GLuint
{
    GLuint ret = 0;
    glGenTextures(1, &ret);

    return ret;
}

Texture::Texture(uint32_t width,
                 uint32_t height,
                 TextureType type,
                 TextureFormat format,
                 TextureDataType dataType,
                 TextureWrappingType wrappingType,
                 TextureFilteringType filteringType,
                 bool hasMipMaps) noexcept
  : RenderingResource(construct(), RenderingResourceType::Texture)
  , m_hasMipMaps(hasMipMaps)
  , m_type(type)
  , m_format(format)
  , m_dataType(dataType)
{
    setTextureWrappingType(wrappingType);
    setTextureFilteringType(filteringType);

    setDimensions(width, height);
}

/* Shader */

Shader::~Shader()
{
    glDeleteShader(getId());
}

void
Shader::checkForCompilationErrors() const noexcept
{
    int success = 0;

    // Retrieve the compilation status
    glGetShaderiv(getId(), GL_COMPILE_STATUS, &success);

    // In case compilation is unsuccessful
    if (!success) {
        char log[4096];
        glGetShaderInfoLog(getId(), 4096, nullptr, log);

        DBG(log);
    }
}

auto
Shader::construct(ShaderType type) -> GLuint
{
    const auto shaderType = [type]() {
        switch (type) {
            case ShaderType::Vertex:
                return GL_VERTEX_SHADER;
            case ShaderType::Geometry:
                return GL_GEOMETRY_SHADER;
            case ShaderType::Fragment:
                return GL_FRAGMENT_SHADER;
            default:
                jassertfalse; // Not implemented
                return GL_NONE;
        }
    };

    return glCreateShader(shaderType());
}

Shader::Shader(const char* source, ShaderType type) noexcept
  : RenderingResource(construct(type), RenderingResourceType::Shader)
{
    // Provide shader sources as text
    glShaderSource(getId(), 1, &source, nullptr);

    // Compile the shader source
    glCompileShader(getId());

    checkForCompilationErrors();
}

/* Program */

template<>
auto
Program::set<bool>(const std::string& uniform, const bool& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform1i(location, (int)t);

    return true;
}

template<>
auto
Program::set<int>(const std::string& uniform, const int& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform1i(location, t);

    return true;
}

template<>
auto
Program::set<float>(const std::string& uniform, const float& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform1f(location, t);

    return true;
}

template<>
auto
Program::set<glm::vec2>(const std::string& uniform, const glm::vec2& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform2fv(location, 1, glm::value_ptr(t));

    return true;
}

template<>
auto
Program::set<glm::vec3>(const std::string& uniform, const glm::vec3& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform3fv(location, 1, glm::value_ptr(t));

    return true;
}

template<>
auto
Program::set<glm::vec4>(const std::string& uniform, const glm::vec4& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform4fv(location, 1, glm::value_ptr(t));

    return true;
}

template<>
auto
Program::set<glm::mat3>(const std::string& uniform, const glm::mat3& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(t));

    return true;
}

template<>
auto
Program::set<glm::mat4>(const std::string& uniform, const glm::mat4& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(t));

    return true;
}

template<>
auto
Program::set<std::vector<glm::vec3>>(const std::string& uniform, const std::vector<glm::vec3>& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform3fv(location, (GLsizei)t.size(), reinterpret_cast<const GLfloat*>(t.data()));

    return true;
}

template<>
auto
Program::set<std::vector<float>>(const std::string& uniform, const std::vector<float>& t) noexcept -> bool
{
    // Retrieve the location
    const auto location = glGetUniformLocation(getId(), uniform.c_str());
    if (location < 0) {
        // jassertfalse; // Uniform not found
        return false;
    }

    // Set value
    glUniform1fv(location, (GLsizei)t.size(), reinterpret_cast<const GLfloat*>(t.data()));

    return true;
}

void
Program::use() const noexcept
{
    glUseProgram(getId());
}

void
Program::unuse() const noexcept
{
    glUseProgram(0);
}

auto
Program::getUniforms() noexcept -> std::vector<std::string>
{
    std::vector<std::string> ret;

    GLint count = 0;
    glGetProgramiv(getId(), GL_ACTIVE_UNIFORMS, &count);

    GLint size;
    GLenum type;

    const GLsizei bufSize = 16;

    GLchar name[bufSize];
    GLsizei length;

    for (GLint i = 0; i < count; ++i) {
        glGetActiveUniform(getId(), (GLuint)i, bufSize, &length, &size, &type, name);

        ret.push_back(std::string{ name } + ", Type: " + std::to_string(type) + ", Index: " + std::to_string(i));
    }

    return ret;
}

Program::~Program()
{
    glDeleteProgram(getId());
}

void
Program::checkForLinkingErrors() const noexcept
{
    int success = 0;

    // Retrieve the linking status
    glGetProgramiv(getId(), GL_LINK_STATUS, &success);

    // In case compilation is unsuccessful
    if (!success) {
        char log[4096];
        glGetProgramInfoLog(getId(), 4096, nullptr, log);

        DBG(log);
    }
}

auto
Program::construct() -> GLuint
{
    return glCreateProgram();
}

Program::Program(const Shader& vertex, const Shader& fragment) noexcept
  : RenderingResource(construct(), RenderingResourceType::Program)
{
    // Attach shaders
    glAttachShader(getId(), vertex.getId());
    glAttachShader(getId(), fragment.getId());

    // Link program
    glLinkProgram(getId());

    checkForLinkingErrors();
}

Program::Program(const Shader& vertex, const Shader& geometry, const Shader& fragment) noexcept
  : RenderingResource(construct(), RenderingResourceType::Program)
{
    // Attach shaders
    glAttachShader(getId(), vertex.getId());
    glAttachShader(getId(), geometry.getId());
    glAttachShader(getId(), fragment.getId());

    // Link program
    glLinkProgram(getId());

    checkForLinkingErrors();
}

/* Buffer */

void
Buffer::bind() const noexcept
{
    glBindBuffer(getTarget(), getId());
}

void
Buffer::unbind() const noexcept
{
    glBindBuffer(getTarget(), 0);
}

void
Buffer::upload(const uint8_t* data, uint32_t size, BufferUsageMode usage)
{
    if (m_size < size) {
        allocate(size, usage);
    }

    // Upload buffer data
    bind();
    {
        glBufferSubData(getTarget(), 0, size, data);
    }
    unbind();
}

void
Buffer::allocate(uint32_t size, BufferUsageMode usage)
{
    if (m_size == size) {
        return;
    }

    const auto usageFlag = [usage]() {
        switch (usage) {
            case BufferUsageMode::StaticDraw:
                return GL_STATIC_DRAW;
            case BufferUsageMode::StreamDraw:
                return GL_STREAM_DRAW;
            case BufferUsageMode::StreamRead:
                return GL_STREAM_READ;
            case BufferUsageMode::DynamicDraw:
                return GL_DYNAMIC_DRAW;
            case BufferUsageMode::DynamicRead:
                return GL_DYNAMIC_READ;
            case BufferUsageMode::DynamicCopy:
                return GL_DYNAMIC_COPY;
            default:
                jassertfalse; // Missing implementation
                return GL_NONE;
        }
    };

    // Allocate buffer data
    bind();
    {
        glBufferData(getTarget(), size, nullptr, usageFlag());
    }
    unbind();

    // Update size
    m_size = size;
}

void
Buffer::mapBuffer(const MapAccessFunctor& functor, BufferAccess access, BufferUsageMode usage, bool discardBuffer) noexcept
{
    const auto accessFlag = [access]() {
        switch (access) {
            case BufferAccess::ReadOnly:
                return GL_READ_ONLY;
            case BufferAccess::WriteOnly:
                return GL_WRITE_ONLY;
            default:
                jassertfalse; // Missing implementation
                return GL_NONE;
        }
    };

    const auto usageFlag = [usage]() {
        switch (usage) {
            case BufferUsageMode::StaticDraw:
                return GL_STATIC_DRAW;
            case BufferUsageMode::StreamDraw:
                return GL_STREAM_DRAW;
            case BufferUsageMode::DynamicDraw:
                return GL_DYNAMIC_DRAW;
            default:
                jassertfalse; // Missing implementation
                return GL_NONE;
        }
    };

    bind();
    {
        if (discardBuffer) {
            glBufferData(getTarget(), getSize(), nullptr, usageFlag());
        }

        auto* ptr = glMapBuffer(getTarget(), accessFlag());
        {
            functor(ptr);
        }
        glUnmapBuffer(getTarget());
    }
    unbind();
}

void
Buffer::mapBufferRange(const MapAccessFunctor& functor, int32_t offset, size_t length, BufferAccess access) noexcept
{
    const auto accessFlag = [access]() {
        switch (access) {
            case BufferAccess::ReadOnly:
                return GL_MAP_READ_BIT;
            case BufferAccess::WriteOnly:
                return GL_MAP_WRITE_BIT;
            default:
                jassertfalse; // Missing implementation
                return GL_NONE;
        }
    };

    bind();
    {
        auto* ptr = glMapBufferRange(getTarget(), offset, length, accessFlag());
        {
            functor(ptr);
        }
        glUnmapBuffer(getTarget());
    }
    unbind();
}

auto
Buffer::getSize() const noexcept -> uint32_t
{
    return m_size;
}

Buffer::~Buffer()
{
    GLuint id = getId();
    glDeleteBuffers(1, &id);
}

auto
Buffer::getTarget() const noexcept -> GLenum
{
    switch (m_type) {
        case BufferType::ArrayBuffer:
            return GL_ARRAY_BUFFER;
        case BufferType::ElementArrayBuffer:
            return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::PixelPackBuffer:
            return GL_PIXEL_PACK_BUFFER;
        case BufferType::PixelUnpackBuffer:
            return GL_PIXEL_UNPACK_BUFFER;
        default:
            jassertfalse; // Missing implementation
            return GL_NONE;
    }
}

auto
Buffer::construct() -> GLuint
{
    GLuint ret = 0;
    glGenBuffers(1, &ret);

    return ret;
}

Buffer::Buffer(BufferType type) noexcept
  : RenderingResource(construct(), RenderingResourceType::Buffer)
  , m_type(type)
  , m_size(0)
{
}

/* VertexArray */

void
VertexArray::bind() const noexcept
{
    glBindVertexArray(getId());
}

void
VertexArray::unbind() const noexcept
{
    glBindVertexArray(0);
}

auto
VertexArray::getVertexBuffer() noexcept -> Buffer&
{
    return m_vertexBuffer;
}

VertexArray::~VertexArray()
{
    GLuint id = getId();
    glDeleteVertexArrays(1, &id);
}
auto
VertexArray::construct() -> GLuint
{
    GLuint ret = 0;
    glGenVertexArrays(1, &ret);

    return ret;
}

VertexArray::VertexArray() noexcept
  : RenderingResource(construct(), RenderingResourceType::VertexArray)
  , m_vertexBuffer(BufferType::ArrayBuffer)
{
    bind();
    {
        // Bind vertex- and index buffers
        m_vertexBuffer.bind();

        // Specify vertex attributes, we assume that these attributes suffice.
        // If there is a reason to add more vertex attributes, this is the place
        // to add them, or extend this type to support variable vertex attribute
        // layouts
        {
            // Specify position location
            glVertexAttribPointer(0,              // Index
                                  3,              // Number of components
                                  GL_FLOAT,       // Data type
                                  GL_FALSE,       // Normalization
                                  sizeof(Vertex), // Stride
                                  (void*)0);      // Pointer

            // Enable position attribute
            glEnableVertexAttribArray(0);

            // Specify texture coordinate location
            glVertexAttribPointer(1,                         // Index
                                  3,                         // Number of components
                                  GL_FLOAT,                  // Data type
                                  GL_TRUE,                   // Normalization
                                  sizeof(Vertex),            // Stride
                                  (void*)(sizeof(glm::vec3)) // Pointer
            );

            // Enable position attribute
            glEnableVertexAttribArray(1);

            // Specify normal location
            glVertexAttribPointer(2,                                             // Index
                                  2,                                             // Number of components
                                  GL_FLOAT,                                      // Data type
                                  GL_TRUE,                                       // Normalization
                                  sizeof(Vertex),                                // Stride
                                  (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)) // Pointer
            );

            // Enable position attribute
            glEnableVertexAttribArray(2);

            // Specify tangent location
            glVertexAttribPointer(3,                                                                 // Index
                                  3,                                                                 // Number of components
                                  GL_FLOAT,                                                          // Data type
                                  GL_TRUE,                                                           // Normalization
                                  sizeof(Vertex),                                                    // Stride
                                  (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)) // Pointer
            );

            // Enable position attribute
            glEnableVertexAttribArray(3);
        }
    }
    unbind();
}

/* RenderBuffer */

void
RenderBuffer::bind() const noexcept
{
    glBindRenderbuffer(GL_RENDERBUFFER, getId());
}

void
RenderBuffer::unbind() const noexcept
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

auto
RenderBuffer::getWidth() const noexcept -> uint32_t
{
    return m_width;
}

auto
RenderBuffer::getHeight() const noexcept -> uint32_t
{
    return m_height;
}

RenderBuffer::~RenderBuffer()
{
    GLuint id = getId();
    glDeleteRenderbuffers(1, &id);
}

auto
RenderBuffer::construct() -> GLuint
{
    GLuint ret = 0;
    glGenRenderbuffers(1, &ret);

    return ret;
}

RenderBuffer::RenderBuffer(uint32_t width, uint32_t height) noexcept
  : RenderingResource(construct(), RenderingResourceType::RenderBuffer)
  , m_width(width)
  , m_height(height)
{
    bind();
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
    unbind();
}

/* FrameBuffer */

void
FrameBuffer::bind() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, getId());
}

void
FrameBuffer::unbind() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
FrameBuffer::attachTexture(int colorAttachment, const Texture& texture) const noexcept
{
    // Attach to a color attachment
    bind();
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachment, texture.getTarget(), texture.getId(), 0);
    }
    unbind();
}

void
FrameBuffer::attachRenderBuffer(const RenderBuffer& renderBuffer, AttachmentType type) const noexcept
{
    // Retrieve the enum value for the attachment type
    const auto attachmentType = [type]() {
        switch (type) {
            case AttachmentType::Depth:
                return GL_DEPTH_ATTACHMENT;
            case AttachmentType::Stencil:
                return GL_STENCIL_ATTACHMENT;
            case AttachmentType::DepthStencil:
                return GL_DEPTH_STENCIL_ATTACHMENT;
            default:
                jassertfalse; // Missing implementation
                return GL_NONE;
        }
    };

    // Attach the render buffer
    bind();
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType(), GL_RENDERBUFFER, renderBuffer.getId());
    }
    unbind();
}

FrameBuffer::~FrameBuffer()
{
    GLuint id = getId();
    glDeleteFramebuffers(1, &id);
}

void
FrameBuffer::checkForCompleteStatus() const
{
    bind();
    {
        const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            switch (status) {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    jassertfalse; // Framebuffer incomplete: Incomplete attachment
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    jassertfalse; // Frame buffer incomplete: Incomplete missing attachment
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    jassertfalse; // Frame buffer incomplete: Incomplete draw buffer
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    jassertfalse; // Frame buffer incomplete: Incomplete read buffer
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    jassertfalse; // Frame buffer incomplete: Unsupported
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    jassertfalse; // Frame buffer incomplete: Incomplete multisample
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    jassertfalse; // Frame buffer incomplete: Incomplete layer targets
                    break;
                case GL_FRAMEBUFFER_UNDEFINED:
                default:
                    jassertfalse; // Frame buffer incomplete: Undefined
                    break;
            }

            jassertfalse; // Implementation error: Frame buffer is incomplete
        }
    }
    unbind();
}

auto
FrameBuffer::construct() -> GLuint
{
    GLuint ret = 0;
    glGenFramebuffers(1, &ret);

    return ret;
}

FrameBuffer::FrameBuffer() noexcept
  : RenderingResource(construct(), RenderingResourceType::FrameBuffer)
{
}

/* RenderTarget */

void
RenderTarget::bind() const noexcept
{
    // What is meant by binding a render target, is actually binding the
    // underlying frame buffer. As the intention of the caller is to render
    // into the render target, which translates to, switching the frame
    // buffer to the one associated with the render target.
    m_frameBuffer->bind();
}

void
RenderTarget::unbind() const noexcept
{
    // Switch back to the default frame buffer
    m_frameBuffer->unbind();
}

void
RenderTarget::blitToScreen() const noexcept
{
    Error::clear();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBuffer->getId());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    {
        glBlitFramebuffer(0, 0, getWidth(), getHeight(), 0, 0, getWidth(), getHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

auto
RenderTarget::getWidth() const noexcept -> uint32_t
{
    assert(m_color->getWidth() == m_renderBuffer->getWidth());
    return m_color->getWidth();
}

auto
RenderTarget::getHeight() const noexcept -> uint32_t
{
    assert(m_color->getHeight() == m_renderBuffer->getHeight());
    return m_color->getHeight();
}

auto
RenderTarget::getColor() noexcept -> Texture&
{
    return *m_color;
}

auto
RenderTarget::getColor() const noexcept -> const Texture&
{
    return *m_color;
}

RenderTarget::~RenderTarget() {}

RenderTarget::RenderTarget(uint32_t width,
                           uint32_t height) noexcept
  : RenderingResource(UNDEFINED_RENDERING_RESOURCE_ID, // As a render target is just wrapper
                                                       // around a texture and render
                                                       // buffer, it does not have an OpenGL
                                                       // object ID
                      RenderingResourceType::RenderTarget)
  , m_frameBuffer(RenderingResourceFactory::createFrameBufferResource())
  , m_renderBuffer(RenderingResourceFactory::createRenderBufferResource(width, height))
  , m_color(RenderingResourceFactory::createTextureResource(width,
                                                            height,
                                                            TextureType::Texture2D,
                                                            TextureFormat::RGBA,
                                                            TextureDataType::UnsignedByte,
                                                            TextureWrappingType::ClampToEdge,
                                                            TextureFilteringType::Bilinear))
{
    Error::clear();

    // Bind to color attachment 0, if we want to support multiple color
    // attachments, this is the place to implement that
    m_frameBuffer->attachTexture(0, *m_color);

    // Attach to the depth buffer, if we want to support multiple
    // attachments (e.g. depth), this is the place to implement that
    m_frameBuffer->attachRenderBuffer(*m_renderBuffer, AttachmentType::DepthStencil);

    // Now that a color attachment and stencil has been attached, ensure
    // that the status of this frame buffer is complete NOTE: This is
    // commented out, should anything strange happen wrt. the rendering of
    // something that makes use of a render target, make sure that the
    // completeness status is OK
    m_frameBuffer->checkForCompleteStatus();
}

/* RenderingObject */

void
RenderingObject::draw() const noexcept
{
    if (m_vertexArray) {
        m_vertexArray->bind();
        {
            // Add rendering hint for dFdX and dFdY use
            glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

            glDrawArrays(getType(), 0, m_vertexCount);
        }
        m_vertexArray->unbind();
    } else {
        jassertfalse; // Program does not exist
    }
}

void
RenderingObject::drawInstanced(size_t numInstances) const noexcept
{
    if (m_vertexArray) {
        m_vertexArray->bind();
        {
            // Add rendering hint for dFdX and dFdY use
            glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

            glDrawArraysInstanced(getType(), 0, m_vertexCount, numInstances);
        }
        m_vertexArray->unbind();
    } else {
        jassertfalse; // Program does not exist
    }
}

auto
RenderingObject::getTransform() noexcept -> Transform&
{
    return m_transform;
}

void
RenderingObject::setPrimitiveType(RenderingPrimitiveType type) noexcept
{
    m_primitiveType = type;
}

RenderingObject::~RenderingObject() {}

auto
RenderingObject::getType() const noexcept -> GLenum
{
    switch (m_primitiveType) {
        case RenderingPrimitiveType::Triangles:
            return GL_TRIANGLES;
        case RenderingPrimitiveType::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case RenderingPrimitiveType::LineStrip:
            return GL_LINE_STRIP;
        case RenderingPrimitiveType::LinesAdjacency:
            return GL_LINES_ADJACENCY;
        case RenderingPrimitiveType::Lines:
            return GL_LINES;
        case RenderingPrimitiveType::Points:
            return GL_POINTS;
        default:
            jassertfalse; // Missing implementation
            return GL_NONE;
    }
}

void
RenderingObject::setVertexCount(size_t vertexCount) noexcept
{
    m_vertexCount = vertexCount;
}

RenderingObject::RenderingObject(VertexArray* vertexArray)
  : m_vertexArray(vertexArray)
  , m_vertexCount(0)
  , m_primitiveType(RenderingPrimitiveType::Triangles)
{
    jassert(m_vertexArray != nullptr); // Vertex array is not created
}

/* Rectangle */

Rectangle::Rectangle() noexcept
  : RenderingObject(RenderingResourceFactory::createVertexArrayResource())
{
    // Define vertex data for a rectangle
    // clang-format off
    const std::array<Vertex, 6> vertexData = { 
        Vertex{ glm::vec3{ -0.5f, -0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 0.0f, 0.0f }, glm::vec3{} },
        Vertex{ glm::vec3{  0.5f, -0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 1.0f, 0.0f }, glm::vec3{} },
        Vertex{ glm::vec3{  0.5f,  0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 1.0f, 1.0f }, glm::vec3{} },
        Vertex{ glm::vec3{ -0.5f, -0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 0.0f, 0.0f }, glm::vec3{} },
        Vertex{ glm::vec3{  0.5f,  0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 1.0f, 1.0f }, glm::vec3{} },
        Vertex{ glm::vec3{ -0.5f,  0.5f, 0.0f }, glm::vec3{}, glm::vec2{ 0.0f, 1.0f }, glm::vec3{} },
    };
    // clang-format on

    // Upload data to the vertex buffer
    m_vertexArray->getVertexBuffer().upload(gsl::span<const Vertex>(vertexData));

    setVertexCount(6);

    setPrimitiveType(RenderingPrimitiveType::Triangles);
}

/* RenderingResourceFactory */

auto
RenderingResourceFactory::createTextureResource(uint32_t width,
                                                uint32_t height,
                                                TextureType type,
                                                TextureFormat format,
                                                TextureDataType dataType,
                                                TextureWrappingType wrappingType,
                                                TextureFilteringType filteringType,
                                                bool hasMipMaps) noexcept -> Texture*
{
    // Create texture object
    auto* ret = new Texture(width, height, type, format, dataType, wrappingType, filteringType, hasMipMaps);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createShaderResource(const char* source, ShaderType type) noexcept -> Shader*
{
    // Create shader object
    auto* ret = new Shader(source, type);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createProgramResource(const Shader& vertex, const Shader& fragment) noexcept -> Program*
{
    // Create shader object
    auto* ret = new Program(vertex, fragment);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createProgramResource(const Shader& vertex, const Shader& geometry, const Shader& fragment) noexcept -> Program*
{
    // Create shader object
    auto* ret = new Program(vertex, geometry, fragment);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createBufferResource(BufferType type) noexcept -> Buffer*
{
    // Create buffer object
    auto* ret = new Buffer(type);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createVertexArrayResource() noexcept -> VertexArray*
{
    // Create vertex array object
    auto* ret = new VertexArray;

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createFrameBufferResource() noexcept -> FrameBuffer*
{
    // Create render target object
    auto* ret = new FrameBuffer();

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createRenderBufferResource(uint32_t width, uint32_t height) noexcept -> RenderBuffer*
{
    // Create render target object
    auto* ret = new RenderBuffer(width, height);

    ENSURE_NO_ERROR();

    return ret;
}

auto
RenderingResourceFactory::createRenderTargetResource(uint32_t width, uint32_t height) noexcept -> RenderTarget*
{
    // Create render target object
    auto* ret = new RenderTarget(width, height);

    ENSURE_NO_ERROR();

    return ret;
}

/* RenderingHelper */

void
RenderingHelper::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept
{
    jassert(width >= 0);  // Invalid width
    jassert(height >= 0); // Invalid height

    // Set the viewport
    glViewport(x, y, width, height);

    ENSURE_NO_ERROR();
}

void
RenderingHelper::enableDefaultAlphaBlending() noexcept
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ENSURE_NO_ERROR();
}

void
RenderingHelper::disableAlphaBlending() noexcept
{
    glDisable(GL_BLEND);

    ENSURE_NO_ERROR();
}

void
RenderingHelper::enableScissorTest(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);

    ENSURE_NO_ERROR();
}

void
RenderingHelper::disableScissorTest() noexcept
{
    glDisable(GL_SCISSOR_TEST);

    ENSURE_NO_ERROR();
}

void
RenderingHelper::clear(float r, float g, float b, float a, bool color, bool depth, bool stencil) noexcept
{
    GLenum flag = GL_NONE;

    flag |= color ? GL_COLOR_BUFFER_BIT : GL_NONE;
    flag |= depth ? GL_DEPTH_BUFFER_BIT : GL_NONE;
    flag |= stencil ? GL_STENCIL_BUFFER_BIT : GL_NONE;

    glClearColor(r, g, b, a);
    glClear(flag);

    ENSURE_NO_ERROR();
}

auto
RenderingHelper::getMaxSamples() noexcept -> uint32_t
{
    int maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    ENSURE_NO_ERROR();

    return maxSamples;
}
