#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <smol/smol_engine.h>
#include <smol/smol_scene.h>

namespace smol
{
  class SMOL_ENGINE_API Renderer
  {
    Scene* scene;
    Rect viewport;

    public:
    Renderer (Scene& scene, int width, int height);
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    void resize(int width, int height);   // Resizes the necessary resources to accomodathe the required dimentions.
    void render();                        // Called once per frame to render the scene.
    Rect getViewport();

    // GPU resources management
    // Texture
    static bool createTextureFromImage(Texture* outTexture, const Image& image);
    static void destroyTexture(Texture*);

    // Shader
    static bool createShaderProgram(ShaderProgram* outShader, const char* vsSource, const char* fsSource, const char* gsSource);
    static void destroyShaderProgram(ShaderProgram* program);

    // Mesh
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

    ~Renderer();
  };
}
#endif  // SMOL_RENDERER_H

