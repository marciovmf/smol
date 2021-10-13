#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <vector>
#include <smol/smol_engine.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_gl.h>
#include <smol/smol_resource_list.h>

#define SMOL_POSITION_ATTRIB_LOCATION  0
#define SMOL_UV0_ATTRIB_LOCATION       1
#define SMOL_UV1_ATTRIB_LOCATION       2
#define SMOL_NORMAL_ATTRIB_LOCATION    3

namespace smol
{
  struct Image;

  enum ClearOperation : char
  {
    DONT_CLEAR = 0,
    COLOR_BUFFER = 1,
    DEPTH_BUFFER = 1 << 1
  };

  enum RenderQueue : char
  {
    QUEUE_OPAQUE = 10,
    QUEUE_TRANSPARENT = 20,
    QUEUE_GUI = 30,
    QUEUE_TERRAIN = 40
  };

  enum Primitive : char
  {
    TRIANGLE,
    TRIANGLE_STRIP,
    LINE,
    POINT
  };

  enum SceneNodeType : char
  {
    EMPTY,
    MESH,
    SPRITE
  };

  struct SMOL_ENGINE_API Texture
  {
    GLuint textureObject;
  };

  struct SMOL_ENGINE_API ShaderProgram
  {
    bool valid;
    GLuint programId;
    //TODO(marcio): store uniform locations here
  };

#define SMOL_MATERIAL_TEXTURE_DIFFUSE_MAX 6
#define SMOL_MAX_BUFFERS_PER_MESH 6

  struct SMOL_ENGINE_API Material
  {
    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[SMOL_MATERIAL_TEXTURE_DIFFUSE_MAX];
    int diffuseTextureCount;
    int renderQueue;
    //TODO(marcio): Add more state relevant options here
  };

  struct SMOL_ENGINE_API Mesh
  {
    GLuint glPrimitive;
    GLuint vao;
    GLuint ibo;
    GLuint vboPosition;
    GLuint vboNormal;
    GLuint vboUV0;
    GLuint vboUV1;

    unsigned int numIndices;
    unsigned int numVertices;
  };

  struct SMOL_ENGINE_API Renderable
  {
    Handle<Material> material;
    Handle<Mesh> mesh;
  };

  struct Transform
  {
    Mat4 mat;
  };

  struct EmptySceneNode { };

  struct MeshSceneNode
  {
    Handle<Renderable> renderable;
  };

  struct SpriteSceneNode
  {
    float x, y, w, h;
    smol::Renderable& renderable;
  };

  struct SceneNode
  {
    SceneNodeType type;
    Transform transform;
    Transform& parentTransform;

    union
    {
      EmptySceneNode emptySceneNode;
      MeshSceneNode meshSceneNode;
      SpriteSceneNode spriteSceneNode;
    };
  };
}

template class SMOL_ENGINE_API smol::ResourceList<smol::ShaderProgram>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Texture>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Material>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Mesh>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Renderable>;
template class SMOL_ENGINE_API smol::ResourceList<smol::SceneNode>;

namespace smol
{
  struct SMOL_ENGINE_API Scene
  {
    smol::ResourceList<smol::ShaderProgram> shaders;
    smol::ResourceList<smol::Texture> textures;
    smol::ResourceList<smol::Material> materials;
    smol::ResourceList<smol::Mesh> meshes;
    smol::ResourceList<smol::Renderable> renderables;
    smol::ResourceList<smol::SceneNode> sceneNodes;
    Mat4 perspective;
    Mat4 orthographic;
    Vector3 clearColor;
    ClearOperation clearOperation;

    Scene();

    // Shaders
    Handle<ShaderProgram> Scene::createShader(const char* vsFilePath, const char* fsFilePath, const char* gsFilePath = nullptr);
    void destroyShader(Handle<ShaderProgram> handle);

    // Textures
    //TODO: Add texture filtering options here
    Handle<Texture> Scene::createTexture(const char* bitmapPath);
    Handle<Texture> Scene::createTexture(const Image& image);
    void destroyTexture(Handle<Texture> handle);

    // Materials
    Handle<Material> createMaterial(Handle<ShaderProgram> shader, Handle<Texture>* diffuseTextures, int diffuseTextureCount);
    void destroyMaterial(Handle<Material> handle);

    // Meshes
    Handle<Mesh> createMesh(Primitive primitive,
        Vector3* vertices, size_t verticesArraySize,
        unsigned int* indices, size_t indicesArraySize,
        Vector2* uv0, size_t uv0ArraySize,
        Vector2* uv1, size_t uv1ArraySize,
        Vector3* normals, size_t normalsArraySize);
    void destroyMesh(Handle<Mesh> handle);

    // Renderables
    Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
    void destroyRenderable(Handle<Renderable> handle);

    // Scene Node
    //TODO(marcio): Implemente these...
    //Handle<SceneNode> Scene::createEmptyNode(const SceneNode& node);
    //Handle<SceneNode> Scene::createMeshNode(const SceneNode& node);
    //Handle<SceneNode> Scene::createSpriteNode(const SceneNode& node);
  };

  struct SMOL_ENGINE_API Renderer
  {
    int width;
    int height;
    Scene* scene;

    Renderer (Scene& scene, int width, int height);
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    void resize(int width, int height);   // Resizes the necessary resources to accomodathe the required dimentions.
    void render();                        // Called once per frame to render the scene.
  };

}


#endif  // SMOL_RENDERER_H

