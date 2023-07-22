#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <smol/smol_engine.h>
#include <smol/smol_stream_buffer.h>

namespace smol
{
  struct Scene;
  struct Vector2;
  struct Vector3;
  struct Color;
  struct Image;
  struct MeshData;
  struct GlobalRendererConfig;
  struct RenderTarget;

  //
  // A stream buffer is an interleaved buffer meant to be overwritten frequently
  //
  struct StreamBuffer;

  class SMOL_ENGINE_API Renderer
  {

    bool enableGammaCorrection;
    bool enableMSAA;
    static ShaderProgram defaultShader;

    public:
    enum ClearBufferFlag
    {
      CLEAR_NONE         = 0,
      CLEAR_COLOR_BUFFER = GL_COLOR_BUFFER_BIT,
      CLEAR_DEPTH_BUFFER = GL_DEPTH_BUFFER_BIT,
    };

    enum RenderMode
    {
      SHADED,
      WIREFRAME
    };

    //
    // Misc
    //
    static void initialize(const GlobalRendererConfig& config);
    void terminate();

    //
    // Texture resources
    //
    static bool createTexture(Texture* outTexture,
        const Image& image,
        Texture::Wrap wrap = Texture::Wrap::REPEAT, 
        Texture::Filter filter = Texture::Filter::LINEAR,
        Texture::Mipmap mipmap = Texture::Mipmap::NO_MIPMAP);

    static void destroyTexture(Texture*);

    //
    // Shader resources
    //
    static bool createShaderProgram(ShaderProgram* outShader, const char* vsSource, const char* fsSource, const char* gsSource);
    static void destroyShaderProgram(ShaderProgram* program);
    static ShaderProgram getDefaultShaderProgram();

    //
    // Mesh resources
    //
    static bool createMesh(Mesh* outMesh,
        bool dynamic, Primitive primitive,
        const Vector3* vertices, int numVertices,
        const unsigned int* indices, int numIndices,
        const Color* color,
        const Vector2* uv0,
        const Vector2* uv1,
        const Vector3* normals);

    static void updateMesh(Mesh* mesh, MeshData* meshData);
    static void destroyMesh(Mesh* mesh);

    static void updateGlobalShaderParams(const Mat4& proj, const Mat4& view, const Mat4& model, float deltaTime);

    //
    // Render Target
    //
    static bool createTextureRenderTarget(RenderTarget* out, int32 width, int32 height);
    static void resizeTextureRenderTarget(const RenderTarget& target, int32 width, int32 height);
    static void useRenderTarget(const RenderTarget& target);
    static void useDefaultRenderTarget();

    //
    // StreamBuffers
    //

    static bool createStreamBuffer(StreamBuffer* out, uint32 capacity = 8, StreamBuffer::Format format = StreamBuffer::POS_COLOR_UV, uint32 indicesPerElement = 6);
    static bool resizeStreamBuffer(StreamBuffer& streamBuffer, uint32 capacity);
    static void bindStreamBuffer(StreamBuffer& streamBuffer);
    static void unbindStreamBuffer(StreamBuffer& streamBuffer);
    static bool destroyStreamBuffer(StreamBuffer& streamBuffer);

    static void begin(StreamBuffer& streamBuffer);
    static void pushSprite(StreamBuffer& streamBuffer, const Vector3& position, const Vector2& size, const Rectf& uv, const Color& color);
    static void pushSprite(StreamBuffer& streamBuffer, const Vector3& position, const Vector2& size, const Rectf& uv, const Color& tlColor, const Color& trColor, const Color& blColor, const Color& brColor);
    static void pushLines(StreamBuffer& streamBuffer, const Vector2* points, int numPoints, const Color& color, float thickness);
    static void end(StreamBuffer& streamBuffer);
    static void flush(StreamBuffer& streamBuffer);

    // static state functions
    static void setMaterial(const Material* material);
    static void setMaterial(Handle<Material> handle);
    static void setViewport(uint32 x, uint32 y, uint32 w, uint32 h);
    static Rect getViewport();
    static void clearBuffers(uint32 flag);
    static void setClearColor(float r, float g, float b, float a = 1.0f);
    static void setClearColor(const Color& color);
    static void setRenderMode(RenderMode mode);
    static void beginScissor( uint32 x, uint32 y, uint32 w, uint32 h);
    static void endScissor();
  };
}
#endif  // SMOL_RENDERER_H

