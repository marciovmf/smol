#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <smol/smol_engine.h>
#include <smol/smol_renderer_types.h>

namespace smol
{
  struct Scene;
  struct Vector2;
  struct Vector3;
  struct Color;
  struct Image;
  struct MeshData;

  class SMOL_ENGINE_API Renderer
  {
    Scene* scene;
    Rect viewport;
    static ShaderProgram defaultShader;
    bool resized;

    public:

    //
    // Misc
    //
    Renderer (Scene& scene, int width, int height);
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    Scene& getLoadedScene();
    Rect getViewport();
    
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
  };
}
#endif  // SMOL_RENDERER_H

