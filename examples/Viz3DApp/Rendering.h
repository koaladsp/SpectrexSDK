#pragma once

// GLAD
#include <glad/glad.h>

// GLM
#include <glm/glm.hpp>

// GSL
#include <gsl/gsl>

// Stdlib
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#define UNDEFINED_RENDERING_RESOURCE_ID 0

/* Forward Declarations */

class Program;

/// @brief Represents an OpenGL error.
struct Error
{
    /// @brief OpenGL error flag.
    const GLenum Flag;

    /// @brief Clears the current error flag.
    inline static void clear() noexcept
    {
#ifdef HAS_ERROR_GL
        while (glGetError() != GL_NO_ERROR)
            ;
#endif
    }

    /// @brief Returns a flag indicating whether or not this instance is
    /// erroneous.
    /// @return True if error, else false.
    operator bool() const noexcept { return Flag != GL_NO_ERROR; }

    /// @brief Return a string representation of the flag.
    /// @return String representation of error flag.
    operator std::string() const noexcept
    {
        switch (Flag) {
            case GL_NO_ERROR:
                return "GL_NO_ERROR";
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY";
        }

        return "Unknown error";
    }

#ifdef HAS_ERROR_GL
    /// @brief Create an Error instance, defined according to the current GL
    /// error value.
    Error()
      : Flag(glGetError())
    {
    }
#else  // HAS_ERROR_GL
    /// @brief Create an Error instance, with flag value GL_NO_ERROR.
    Error()
      : Flag(GL_NO_ERROR)
    {
    }
#endif // HAS_ERROR_GL
};

#define ENSURE_NO_ERROR()                                                                                                                            \
    do {                                                                                                                                             \
        Error::clear();                                                                                                                              \
        const Error error;                                                                                                                           \
        if (error) {                                                                                                                                 \
            Ensures(false);                                                                                                                          \
        }                                                                                                                                            \
    } while (false);

/* --- */

/* Transform */

class Transform
{
  public:
    /// @brief Returns the model matrix of this object.
    /// @return Model matrix.
    auto getModelMatrix() const noexcept -> glm::mat4;

    /// @brief Returns the position of this object as a mutable object.
    /// @return Position.
    auto getPosition() noexcept -> glm::vec3&;

    /// @brief Returns the rotation of this object as a mutable object.
    /// @return Rotation.
    auto getRotation() noexcept -> glm::vec3&;

    /// @brief Returns the scale of this object as a mutable object.
    /// @return Scale.
    auto getScale() noexcept -> glm::vec3&;

    /// @brief Creates a transform object.
    /// @param position Initial position.
    /// @param rotation Initial rotation.
    /// @param scale Initial scale.
    Transform(glm::vec3 position = { 0.0f, 0.0f, 0.0f }, glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }, glm::vec3 scale = { 1.0f, 1.0f, 1.0f }) noexcept;

  private:
    glm::vec3 m_position;

    glm::vec3 m_rotation;

    glm::vec3 m_scale;
};

/* OrthoCamera */

/// @brief Represents an orthographic camera that has a position and target.
class OrthoCamera
{
  public:
    /// @brief Returns the view projection of this camera instance. The frustum
    /// spans from (-width/2,-height/2) to (width/2,height/2), such that (0,0)
    /// is at the center of the screen.
    /// @return View projection.
    auto getViewProjection(int64_t width, int64_t height) noexcept -> glm::mat4;

    /// @brief Returns this camera's position.
    /// @return Position.
    auto getPosition() noexcept -> glm::vec3&;

    /// @brief Returns this camera's up vector.
    /// @return Up vector.
    auto getUp() noexcept -> glm::vec3&;

    /// @brief Returns this camera's zoom factor.
    /// @return Zoom factor.
    auto getZoom() noexcept -> glm::vec2&;

    /// @brief Instantiates an orthographic camera.
    /// @param position Position (eye) of camera.
    /// @param up Up direction of camera.
    /// @param zoom Zoom factor.
    OrthoCamera(glm::vec3 position, glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2 zoom = glm::vec2{ 1.0f, 1.0f }) noexcept;

    virtual ~OrthoCamera() = default;

  private:
    /* Properties */

    glm::vec3 m_position;

    glm::vec3 m_up;

    glm::vec2 m_zoom;
};

/* OrbitCamera */

/// @brief Represents an orbit camera that has a position and target.
class OrbitCamera
{
  public:
    /// @brief Returns the perspective projection of this camera instance.
    /// @return Projection.
    auto getProjection(int64_t width, int64_t height) noexcept -> glm::mat4;

    /// @brief Return the view of this camera instance.
    /// @return View.
    auto getView() noexcept -> glm::mat4;

    /// @brief Returns this camera's position.
    /// @return Position.
    auto getPosition() noexcept -> glm::vec3&;

    /// @brief Returns this camera's target.
    /// @return Target.
    auto getTarget() noexcept -> glm::vec3&;

    /// @brief Return this camera's field of view.
    /// @return Field of view.
    auto getFov() noexcept -> float&;

    /// @brief Return this camera's near distance.
    /// @return Field of view.
    auto getNear() noexcept -> float&;

    /// @brief Return this camera's far distance.
    /// @return Field of view.
    auto getFar() noexcept -> float&;

    /// @brief Returns this camera's up vector.
    /// @return Up vector.
    auto getUp() noexcept -> glm::vec3&;

    /// @brief Instantiates an orbit camera.
    /// @param position Position (eye) of camera.
    /// @param position Target of camera.
    /// @param fov Field of view;
    /// @param near Distance of near plane.
    /// @param far Distance of far plane.
    /// @param up Up direction of camera.
    OrbitCamera(glm::vec3 position,
                glm::vec3 target,
                float fov = 45.0f,
                float near = 0.001f,
                float far = 1000.0f,
                glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f }) noexcept;

    virtual ~OrbitCamera() = default;

  private:
    /* Properties */

    glm::vec3 m_position;
    glm::vec3 m_target;

    float m_fov;

    float m_near;
    float m_far;

    glm::vec3 m_up;
};

/* RenderingResource */

/// @brief The type of a resource.
enum class RenderingResourceType
{
    Texture,
    Shader,
    Program,
    Buffer,
    VertexArray,
    RenderBuffer,
    FrameBuffer,
    RenderTarget
};

/// @brief A rendering resource describes an OpenGL resource such as a texture
/// that has an ID.
class RenderingResource
{
  public:
    /// @brief Returns the type of this rendering resource.
    /// @return Type of this resource.
    auto getType() const noexcept -> RenderingResourceType { return m_type; }

    auto getId() const noexcept -> GLuint { return m_id; }

    virtual ~RenderingResource() = default;

  protected:
    /// @brief Instantiate a RenderingResource resource.
    /// @param id ID of this resource.
    /// @param type Type of this resource.
    RenderingResource(GLuint id, RenderingResourceType type) noexcept
      : m_id(id)
      , m_type(type)
    {
    }

  private:
    const GLuint m_id;

    const RenderingResourceType m_type;
};

/* Texture */

/// @brief The texture type of a texture.
enum class TextureType
{
    Texture2D
};

/// @brief The wrapping mode of a texture.
enum class TextureWrappingType
{
    ClampToEdge
};

/// @brief The filtering mode of a texture.
enum class TextureFilteringType
{
    Nearest,
    Bilinear
};

/// @brief The pixel format of pixels of a texture.
enum class TextureFormat
{
    R,
    RG,
    RGB,
    RGBA
};

/// @brief The data type of individual pixel components of a texture.
enum class TextureDataType
{
    Undefined,
    Float,
    UnsignedByte
};

/// @brief A texture resource. A texture can be resized dynamically, although
/// its data type and format are expected to be fixed. In addition, properties
/// such as the wrapping mode can be changed dynamically.
class Texture final : public RenderingResource
{
  public:
    /// @brief Binds this texture resource.
    void bind() const noexcept;

    /// @brief Unbinds this texture resource.
    void unbind() const noexcept;

    /// @brief Binds this texture to a texture \a unit.
    /// @param unit Unit to bind to.
    void bindToTextureUnit(uint32_t unit) const noexcept;

    /// @brief Clears this texture.
    void clear() noexcept;

    /// @brief Uploads typed texture data to this texture resource. Data is
    /// expected to be sufficient in terms of its width, height, x and y and
    /// data type. A multi-sampled texture is assumed to be provided as full
    /// image data.
    /// @tparam T Type of the data to upload.
    /// @param data Data to upload.
    template<typename T>
    void upload(const gsl::span<T> data) noexcept
    {
        upload(data, 0, 0, getWidth(), getHeight());
    }

    /// @brief Uploads typed texture data to this texture resource. Data is
    /// expected to be sufficient in terms of its width, height, x and y and
    /// data type. A multi-sampled texture is assumed to be provided as full
    /// image data.
    /// @tparam T Type of the data to upload.
    /// @param data Data to upload.
    /// @param x Starting X-position to write into.
    /// @param x Starting Y-position to write into.
    /// @param width Width of data to write.
    /// @param height Height of data to write.
    template<typename T>
    void upload(const gsl::span<T> data, int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept
    {
        upload(reinterpret_cast<const uint8_t*>(data.data()), x, y, width, height);
    }

    /// @brief Uploads untyped texture data to this texture resource. Data
    /// is expected to be sufficient in terms of its width, height, x and y and
    /// data type. A multi-sampled texture is assumed to be provided as full
    /// image data.
    /// @param data Data to upload.
    /// @param x Starting X-position to write into.
    /// @param x Starting Y-position to write into.
    /// @param width Width of data to write.
    /// @param height Height of data to write.
    void upload(const uint8_t* data, int32_t x, int32_t y, uint32_t width, uint32_t height);

    /// @brief Downloads types texture data and returns the texture data.
    /// @tparam T Pixel component data type.
    /// @return Texture data.
    void download(void* destination)
    {
        bind();
        {
            glGetTexImage(GL_TEXTURE_2D, 0, getFormat(), getDataType(), destination);
        }
        unbind();

        ENSURE_NO_ERROR();
    }

    /// @brief Returns the current width of this texture resource.
    /// @return Width of the texture.
    auto getWidth() const noexcept -> uint32_t;

    /// @brief Returns the current height of this texture resource.
    /// @return Height of the texture.
    auto getHeight() const noexcept -> uint32_t;

    /// @brief Returns the stride of a single element in this texture.
    auto getStride() const noexcept -> uint32_t;

    /// @brief Sets the texture wrapping type of this texture resource.
    /// @param type Texture wrapping type to set.
    void setTextureWrappingType(TextureWrappingType type) noexcept;

    /// @brief Sets the texture filtering type of this texture resource.
    /// @param type Texture filtering type to set.
    void setTextureFilteringType(TextureFilteringType type) noexcept;

    /// @brief Sets the dimensions of this texture resource and thereby
    /// (re-)allocates texture data. If the dimensions requested are the same as
    /// the current dimensions, this call does nothing.
    /// @param width Width of texture resource to set.
    /// @param height Height of texture resource to set.
    void setDimensions(uint32_t width, uint32_t height) noexcept;

    ~Texture();

  private:
    auto getTarget() const noexcept -> GLenum;

    auto getInternalFormat() const noexcept -> GLenum;

    auto getFormat() const noexcept -> GLenum;

    auto getDataType() const noexcept -> GLenum;

    static auto construct() -> GLuint;

    /// @brief Instantiates a new texture resource with properties as specified.
    /// @param width Initial width of the texture resource.
    /// @param height Initial height of the texture resource.
    /// @param type The type of texture.
    /// @param format The format of texture.
    /// @param dataType The data type of texture, in case the texture resource
    /// is not a multi-sampled texture, otherwise this property does nothing.
    /// @param wrappingType Wrapping mode of the texture.
    /// @param filteringType Filtering type of the texture.
    /// @param hasMipMaps Whether or not the texture should have mipmaps.
    Texture(uint32_t width,
            uint32_t height,
            TextureType type,
            TextureFormat format,
            TextureDataType dataType,
            TextureWrappingType wrappingType,
            TextureFilteringType filteringType,
            bool hasMipMaps = false) noexcept;

  private:
    const bool m_hasMipMaps;

    const TextureType m_type;

    const TextureFormat m_format;

    const TextureDataType m_dataType;

    uint32_t m_width;
    uint32_t m_height;

    TextureWrappingType m_wrappingType;

    TextureFilteringType m_filteringType;

  private:
    friend class FrameBuffer;
    friend class RenderTarget;
    friend class RenderingResourceFactory;
};

/* Shader */

enum class ShaderType
{
    Vertex,
    Geometry,
    Fragment
};

/// @brief A shader resource, wraps around an OpenGL shader object.
class Shader final : public RenderingResource
{
  public:
    ~Shader();

  private:
    void checkForCompilationErrors() const noexcept;

  private:
    static auto construct(ShaderType type) -> GLuint;

    /// @brief Instantiates a shader object with a given \a type, by providing
    /// the shader source as text.
    /// @param source Shader source as text.
    /// @param type Type of shader to create.
    Shader(const char* source, ShaderType type) noexcept;

  private:
    friend class RenderingResourceFactory;
    friend class Program;
};

/* Program */

/// @brief A program resource wraps around an OpenGL program object, linking one
/// or multiple shaders.
class Program final : public RenderingResource
{
  public:
    /// @brief Set this program as the current program.
    void use() const noexcept;

    /// @brief Clear this program as the current program.
    void unuse() const noexcept;

    /// @brief Sets a uniform by name.
    /// @tparam T The uniform's type.
    /// @param uniform The uniform's name.
    /// @param t The value to set.
    /// @return True if the uniform has been set, otherwise false (due to a
    /// missing uniform).
    template<typename T>
    auto set(const std::string& uniform, const T& t) noexcept -> bool
    {
        (void)uniform;
        (void)t;

        // Unimplemented
        return false;
    }

    /// @brief Returns the active uniforms.
    /// @return Uniforms.
    auto getUniforms() noexcept -> std::vector<std::string>;

    ~Program();

  private:
    void checkForLinkingErrors() const noexcept;

    static auto construct() -> GLuint;

    /// @brief Created a shader program, linking a \a vertex and \a fragment
    /// shader. After successful construction of the Program, the shaders can be
    /// destroyed.
    /// @param vertex Vertex shader to link.
    /// @param fragment Fragment shader to link.
    Program(const Shader& vertex, const Shader& fragment) noexcept;

    /// @brief Created a shader program, linking a \a vertex and \a fragment
    /// shader. After successful construction of the Program, the shaders can be
    /// destroyed.
    /// @param vertex Vertex shader to link.
    /// @param geometry Geometry shader to link.
    /// @param fragment Fragment shader to link.
    Program(const Shader& vertex, const Shader& geometry, const Shader& fragment) noexcept;

  private:
    friend class RenderingResourceFactory;
};

/* Declarations */

template<>
auto
Program::set<bool>(const std::string& uniform, const bool& t) noexcept -> bool;

template<>
auto
Program::set<int32_t>(const std::string& uniform, const int32_t& t) noexcept -> bool;

template<>
auto
Program::set<float>(const std::string& uniform, const float& t) noexcept -> bool;

template<>
auto
Program::set<glm::vec2>(const std::string& uniform, const glm::vec2& t) noexcept -> bool;

template<>
auto
Program::set<glm::vec3>(const std::string& uniform, const glm::vec3& t) noexcept -> bool;

template<>
auto
Program::set<glm::vec4>(const std::string& uniform, const glm::vec4& t) noexcept -> bool;

template<>
auto
Program::set<glm::mat3>(const std::string& uniform, const glm::mat3& t) noexcept -> bool;

template<>
auto
Program::set<glm::mat4>(const std::string& uniform, const glm::mat4& t) noexcept -> bool;

template<>
auto
Program::set<std::vector<glm::vec3>>(const std::string& uniform, const std::vector<glm::vec3>& t) noexcept -> bool;

template<>
auto
Program::set<std::vector<float>>(const std::string& uniform, const std::vector<float>& t) noexcept -> bool;

/* Buffer */

enum class BufferType
{
    ArrayBuffer,
    ElementArrayBuffer,
    PixelPackBuffer,
    PixelUnpackBuffer
};

enum class BufferUsageMode
{
    StaticDraw,
    StreamDraw,
    StreamRead,
    DynamicDraw,
    DynamicRead,
    DynamicCopy
};

enum class BufferAccess
{
    ReadOnly,
    WriteOnly
};

/// @brief A buffer resource wraps around an OpenGL buffer. A buffer can have
/// various targets for different kinds of usage.
class Buffer : public RenderingResource
{
  public:
    using MapAccessFunctor = std::function<void(void*)>;

  public:
    /// @brief Binds this buffer resource.
    void bind() const noexcept;

    /// @brief Unbinds this buffer resource.
    void unbind() const noexcept;

    /// @brief Uploads typed texture data to this buffer resource.
    /// @tparam T Type of the data to upload.
    /// @param data Data to upload.
    /// @param usage Usage mode.
    template<typename T>
    void upload(const gsl::span<T> data, BufferUsageMode usage = BufferUsageMode::StaticDraw) noexcept
    {
        upload(reinterpret_cast<const uint8_t*>(data.data()), (uint32_t)(sizeof(T) * data.size()), usage);
    }

    /// @brief Uploads untyped texture data to this buffer resource.
    /// @tparam T Type of the data to upload.
    /// @param data Data to upload.
    /// @param usage Usage mode.
    void upload(const uint8_t* data, uint32_t size, BufferUsageMode usage = BufferUsageMode::StaticDraw);

    /// @brief Allocate buffer data.
    /// @param size Size of allocation.
    /// @param usage Usage mode.
    void allocate(uint32_t size, BufferUsageMode usage = BufferUsageMode::StaticDraw);

    void mapBuffer(const MapAccessFunctor& functor, BufferAccess access, BufferUsageMode usage, bool discardBuffer) noexcept;

    void mapBufferRange(const MapAccessFunctor& functor, int32_t offset, size_t length, BufferAccess access) noexcept;

    auto getSize() const noexcept -> uint32_t;

    ~Buffer();

  private:
    auto getTarget() const noexcept -> GLenum;

    static auto construct() -> GLuint;

    /// @brief Instantiates a buffer resource with a specified buffer type.
    /// @param type Type of buffer to create
    Buffer(BufferType type) noexcept;

  private:
    const BufferType m_type;

    uint32_t m_size;

  private:
    friend class VertexArray;
    friend class RenderingResourceFactory;
};

/* Vertex */

/// @brief Generic vertex type, expected to correspond to the VertexArray vertex
/// attributes configuration.
struct Vertex
{
    /// @brief Vertex position.
    glm::vec3 Position;

    /// @brief Normal.
    glm::vec3 Normal;

    /// @brief Vertex texture coordinate.
    glm::vec2 TexCoord;

    /// @brief Tangent.
    glm::vec3 Tangent;

    /// @brief Constructs a default vertex.
    Vertex()
      : Position()
      , Normal()
      , TexCoord()
      , Tangent()
    {
    }

    /// @brief Constructs a vertex with properties.
    /// @param position Position.
    /// @param normal Normal.
    /// @param texCoord Texture coord.
    /// @param tangent Tangent.
    Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord, const glm::vec3& tangent)
      : Position(position)
      , Normal(normal)
      , TexCoord(texCoord)
      , Tangent(tangent)
    {
    }
};

/* VertexArray */

/// @brief A vertex array resource wraps around an OpenGL vertex array,
/// combining a vertex buffer and index buffer into a drawable array of elements
/// (vertices).
class VertexArray : public RenderingResource
{
  public:
    /// @brief Binds this vertex array resource.
    void bind() const noexcept;

    /// @brief Unbinds this vertex array resource.
    void unbind() const noexcept;

    /// @brief Returns the associated vertex buffer resource.
    /// @return Vertex buffer resource.
    auto getVertexBuffer() noexcept -> Buffer&;

    ~VertexArray();

  private:
    static auto construct() -> GLuint;

    /// @brief Constructs a vertex array resource. Vertices and indices should
    /// be uploaded to the vertex- and index buffer retrievable through
    /// getVertexBuffer and getIndexBuffer.
    VertexArray() noexcept;

  private:
    Buffer m_vertexBuffer;

  private:
    friend class RenderingResourceFactory;
};

/* RenderBuffer */

/// @brief A render buffer source wraps around an OpenGL render buffer. The
/// render buffer is typically used by a render target. Its dimensions are
/// fixed. A render buffer assumed multi-sampled usage.
class RenderBuffer : public RenderingResource
{
  public:
    /// @brief Binds this render buffer resource.
    void bind() const noexcept;

    /// @brief Unbinds this render buffer resource.
    void unbind() const noexcept;

    /// @brief Returns the height of this render buffer.
    /// @return Width of the render buffer.
    auto getWidth() const noexcept -> uint32_t;

    /// @brief Returns the height of this render buffer.
    /// @return Height of the render buffer.
    auto getHeight() const noexcept -> uint32_t;

    ~RenderBuffer();

  private:
    static auto construct() -> GLuint;

    /// @brief Constructs a new render buffer object with specified properties.
    /// @param width Width of the render buffer.
    /// @param height Height of the render buffer.
    RenderBuffer(uint32_t width, uint32_t height) noexcept;

  private:
    const uint32_t m_width;
    const uint32_t m_height;

  private:
    friend class RenderingResourceFactory;
};

/* FrameBuffer */

enum class AttachmentType
{
    Depth,
    Stencil,
    DepthStencil
};

/// @brief A frame buffer resource wraps around an OpenGL frame buffer. A frame
/// buffer allows for textures to be attached to the color attachments of the
/// frame buffer and render buffers to be attached to either the depth- and/or
/// stencil attachment of the frame buffer.
///
/// A frame buffer should be ensured to be in a complete state prior to usage.
/// Please refer to the checkForCompleteStatus function to validate the
/// completeness status of a frame buffer prior to usage.
class FrameBuffer : public RenderingResource
{
  public:
    /// @brief Binds this frame buffer resource.
    void bind() const noexcept;

    /// @brief Unbinds this frame buffer resource.
    void unbind() const noexcept;

    /// @brief Attach a render buffer to this frame buffer.
    /// @param colorAttachment Color attachment to attach to.
    /// @param texture Texture to attach.
    void attachTexture(int colorAttachment, const Texture& texture) const noexcept;

    /// @brief Attach a render buffer to this frame buffer.
    /// @param renderBuffer Render buffer to attach.
    /// @param type Type of attachment.
    void attachRenderBuffer(const RenderBuffer& renderBuffer, AttachmentType type) const noexcept;

    /// @brief Checks the status of this frame buffer. If the status is
    /// incomplete an Exception will be thrown indicating the reason for the
    /// frame buffer being incomplete.
    void checkForCompleteStatus() const;

    ~FrameBuffer();

  private:
    static auto construct() -> GLuint;

    FrameBuffer() noexcept;

  private:
    friend class RenderingResourceFactory;
};

/* RenderTarget */

/// @brief Render target is a common name for an off-screen rendering
/// destination. To provide for this functionality, the render target makes use
/// or at least one texture where the color information of a frame buffer
/// associated with the render target is written into. In addition, other
/// aspects of the frame buffer can be attached to separate buffer destinations.
class RenderTarget : public RenderingResource
{
  public:
    /// @brief Binds this render target resource.
    void bind() const noexcept;

    /// @brief Unbinds this render target resource.
    void unbind() const noexcept;

    void blitToScreen() const noexcept;

    /// @brief Returns the height of this render target.
    /// @return Width of the render buffer.
    auto getWidth() const noexcept -> uint32_t;

    /// @brief Returns the height of this render target.
    /// @return Height of the render buffer.
    auto getHeight() const noexcept -> uint32_t;

    /// @brief Returns the color texture as a mutable object.
    /// @return Color texture.
    auto getColor() noexcept -> Texture&;

    /// @brief Returns the color texture as an immutable object.
    /// @return Color texture.
    auto getColor() const noexcept -> const Texture&;

    ~RenderTarget();

  private:
    RenderTarget(uint32_t width, uint32_t height) noexcept;

  private:
    std::unique_ptr<FrameBuffer> m_frameBuffer;

    std::unique_ptr<RenderBuffer> m_renderBuffer;

    std::unique_ptr<Texture> m_color;

  private:
    friend class RenderingResourceFactory;
};

/* RenderingObject */

enum class RenderingPrimitiveType
{
    Triangles,
    TriangleStrip,
    LineStrip,
    LinesAdjacency,
    Lines,
    Points
};

/// @brief A rendering object is a wrapper around the primitives and OpenGL
/// calls required for rendering one object with a shader at a particular
/// position.
class RenderingObject
{
  public:
    /// @brief Draw function, should be implemented by a concrete rendering
    /// object implementation.
    void draw() const noexcept;

    /// @brief Draw function, should be implemented by a concrete rendering
    /// object implementation.
    /// @param numInstances Number of instances to draw.
    void drawInstanced(size_t numInstances) const noexcept;

    /// @brief Returns the transform of this object as a mutable object.
    /// @return Transform of this object.
    auto getTransform() noexcept -> Transform&;

    /// @brief Sets the primitive type.
    /// @param type Primitive type.
    void setPrimitiveType(RenderingPrimitiveType type) noexcept;

    virtual ~RenderingObject();

  protected:
    void setVertexCount(size_t vertexCount) noexcept;

    RenderingObject(VertexArray* vertexArray);

  private:
    auto getType() const noexcept -> GLenum;

  protected:
    std::unique_ptr<VertexArray> m_vertexArray;

  private:
    size_t m_vertexCount;

    RenderingPrimitiveType m_primitiveType;

    Transform m_transform;
};

/* Rectangle */

///@brief A rendering object that is a rectangle.
class Rectangle : public RenderingObject
{
  public:
    ///@brief Creates a rectangle object.
    Rectangle() noexcept;

    ~Rectangle() = default;
};

/* RenderingResourceFactory */

/// @brief A factory for rendering resources. The pattern here, is such that
/// rendering resource implementations have a private constructor and are only
/// constructible by this factory. The reason for this is to guarantee that any
/// necessary error checking has been done and that, when a resource pointer is
/// successfully returned, we can guarantee that these resources are set up
/// validly.
class RenderingResourceFactory final
{
  public:
    /// @brief Creates a texture, given its desired properties.
    /// @param width Initial width of the texture resource.
    /// @param height Initial height of the texture resource.
    /// @param type The type of texture.
    /// @param format Format of the texture.
    /// @param dataType The data type of texture, in case the texture resource
    /// is not a multi-sampled texture, otherwise this property does nothing.
    /// @param wrappingType Wrapping mode of the texture.
    /// @param filteringType Filtering type of the texture.
    /// @param hasMipMaps Whether or not the texture should have mipmaps.
    static auto createTextureResource(uint32_t width,
                                      uint32_t height,
                                      TextureType type,
                                      TextureFormat format,
                                      TextureDataType dataType,
                                      TextureWrappingType wrappingType,
                                      TextureFilteringType filteringType,
                                      bool hasMipMaps = false) noexcept -> Texture*;

    /// @brief Create a shader, given its source as text.
    /// @param source Source of shader.
    /// @param type Shader type.
    static auto createShaderResource(const char* source, ShaderType type) noexcept -> Shader*;

    /// @brief Create a shader program, given a vertex- and fragment shader.
    /// @param vertex Vertex shader.
    /// @param fragment Fragment shader.
    static auto createProgramResource(const Shader& vertex, const Shader& fragment) noexcept -> Program*;

    /// @brief Create a shader program, given a vertex- and fragment shader.
    /// @param vertex Vertex shader.
    /// @param vertex Geometry shader.
    /// @param fragment Fragment shader.
    static auto createProgramResource(const Shader& vertex, const Shader& geometry, const Shader& fragment) noexcept -> Program*;

    /// @brief Create a buffer of a desired buffer \a type.
    /// @param type Type of buffer to create.
    static auto createBufferResource(BufferType type) noexcept -> Buffer*;

    /// @brief Create a vertex array.
    static auto createVertexArrayResource() noexcept -> VertexArray*;

    ///@brief Create a frame buffer.
    static auto createFrameBufferResource() noexcept -> FrameBuffer*;

    ///@brief Create a render buffer.
    ///@param width Width of the render buffer.
    ///@param height Height of the render buffer.
    static auto createRenderBufferResource(uint32_t width, uint32_t height) noexcept -> RenderBuffer*;

    /// @brief Create a render target.
    /// @param width Width of the render target.
    /// @param height Height of the render target.
    static auto createRenderTargetResource(uint32_t width, uint32_t height) noexcept -> RenderTarget*;
};

/* RenderingHelper */

/// @brief Provides functionality for common rendering operations. The reason
/// for this class to exist is to reduce the number of GL calls that should all
/// have appropriate error handling in place at the specific location where
/// drawing is required. Instead, the implementations of this class ensure that
/// any error checking has been properly implemented and that any invalid
/// parameters are handled.
class RenderingHelper final
{
  public:
    /// @brief Sets up the viewport.
    /// @param x X-position.
    /// @param y Y-position.
    /// @param width Width of the viewport.
    /// @param height Height of the viewport.
    static void setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;

    /// @brief Enables default (source alpha, 1 - source alpha) alpha blending.
    static void enableDefaultAlphaBlending() noexcept;

    /// @brief Disables alpha blending.
    static void disableAlphaBlending() noexcept;

    /// @brief Enable scissor test and define the scissor area according to the
    /// provided dimensions.
    /// @param x X-location.
    /// @param y Y-location.
    /// @param width Width of scissor area.
    /// @param height Height of scissor area.
    static void enableScissorTest(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;

    /// @brief Disable scissor test.
    static void disableScissorTest() noexcept;

    /// @brief Clear the current frame buffer.
    /// @param r Clear color red component.
    /// @param g Clear color green component.
    /// @param b Clear color blue component.
    /// @param a Clear color alpha component.
    /// @param color Clear color buffer.
    /// @param depth Clear depth buffer.
    /// @param stencil Clear stencil buffer.
    static void clear(float r, float g, float b, float a, bool color = true, bool depth = true, bool stencil = true) noexcept;

    ///@brief Returns the maximum number of samples (multi-sampling).
    ///@return Maximum number of samples.
    static auto getMaxSamples() noexcept -> uint32_t;
};
