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

  struct ConfigEntry;
  struct GlobalRendererConfig;

  //
  // A stream buffer is an interleaved buffer meant to be overwritten frequently
  //
  struct StreamBuffer;
  class SMOL_ENGINE_API Renderer
  {
    Scene* scene;
    Rect viewport;
    bool enableGammaCorrection;
    bool enableMSAA;
    static ShaderProgram defaultShader;
    bool resized;
    float screenCameraSize;
    float screenCameraNear;
    float screenCameraFar;

    public:

    //
    // Misc
    //
    Renderer ();
    void initialize(const GlobalRendererConfig& config);
    ~Renderer();
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    Scene& getLoadedScene();
    Rect getViewport() const;
    float getViewportAspect() const;

    //
    // Render
    //
    void resize(int width, int height);   // Resizes the necessary resources to accomodathe the required dimentions.
    void render(float deltaTime);         // Called once per frame to render the scene.

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
    static void end(StreamBuffer& streamBuffer);
    static void flush(StreamBuffer& streamBuffer);

    // static state functions
    static void setMaterial(const Material* material);
    static void setMaterial(Handle<Material> handle);
  };
}
#endif  // SMOL_RENDERER_H

