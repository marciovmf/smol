#ifndef SMOL_SCENE_H
#define SMOL_SCENE_H

#include <smol/smol_engine.h>
#include <smol/smol_resource_list.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>

#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h> //TODO(marcio): Make this API independent. Remove all GL specifics from this header
#undef SMOL_GL_DEFINE_EXTERN


namespace smol
{
  struct Image;
  struct MeshData;

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

#define SMOL_MATERIAL_MAX_TEXTURES 6
#define SMOL_MAX_BUFFERS_PER_MESH 6

  struct SMOL_ENGINE_API Material
  {
    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[SMOL_MATERIAL_MAX_TEXTURES];
    int diffuseTextureCount;
    int renderQueue;
    //TODO(marcio): Add more state relevant options here
  };

  struct SMOL_ENGINE_API Mesh
  {
    enum Attribute
    {
      //Don't change these values. They're referenced from the shader
      POSITION = 0,
      UV0 = 1,
      UV1 = 2,
      NORMAL = 3,
      COLOR = 4,
      INDEX // this one does not point to an attribute buffer
    };
    GLuint glPrimitive;
    GLuint vao;
    GLuint ibo;
    GLuint vboPosition;
    GLuint vboNormal;
    GLuint vboUV0;
    GLuint vboUV1;
    GLuint vboColor;
    unsigned int numIndices;
    unsigned int numVertices;
  };

  struct SMOL_ENGINE_API Renderable
  {
    Handle<Material> material;
    Handle<Mesh> mesh;
  };

  class SMOL_ENGINE_API Transform
  {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    float angle;
    bool dirty;
    Mat4 model;

    public:
    Transform::Transform();
    void update();
    const Mat4& getMatrix() const;
    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setRotation(float x, float y, float z, float angle);
    const Vector3& getPosition() const;
    const Vector3& getScale() const;
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
    enum SceneNodeType : char
    {
      EMPTY,
      MESH,
      SPRITE
    };

    SceneNodeType type;
    Transform transform;
    Handle<SceneNode> parent;

    union
    {
      MeshSceneNode meshNode;
      SpriteSceneNode spriteNode;
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
  struct SMOL_ENGINE_API Scene final
  {
    enum ClearOperation
    {
      DONT_CLEAR = 0,
      COLOR_BUFFER = 1,
      DEPTH_BUFFER = 1 << 1
    };

    smol::ResourceList<smol::ShaderProgram> shaders;
    smol::ResourceList<smol::Texture> textures;
    smol::ResourceList<smol::Material> materials;
    smol::ResourceList<smol::Mesh> meshes;
    smol::ResourceList<smol::Renderable> renderables;
    smol::ResourceList<smol::SceneNode> nodes;
    smol::Handle<smol::Texture> defaultTexture;
    smol::Handle<smol::ShaderProgram> defaultShader;
    smol::Handle<smol::Material> defaultMaterial;
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Vector3 clearColor;
    ClearOperation clearOperation;

    static const Handle<SceneNode> ROOT;

    Scene();

    // Shaders
    Handle<ShaderProgram> Scene::createShader(const char* vsFilePath, const char* fsFilePath, const char* gsFilePath = nullptr);
    Handle<ShaderProgram> Scene::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource = nullptr);
    void destroyShader(Handle<ShaderProgram> handle);
    void destroyShader(ShaderProgram* program);

    // Textures
    //TODO: Add texture filtering options here
    Handle<Texture> Scene::createTexture(const char* bitmapPath);
    Handle<Texture> Scene::createTexture(const Image& image);
    void destroyTexture(Handle<Texture> handle);
    void destroyTexture(Texture* texture);

    // Materials
    Handle<Material> createMaterial(Handle<ShaderProgram> shader, Handle<Texture>* diffuseTextures, int diffuseTextureCount);
    void destroyMaterial(Handle<Material> handle);

    // Meshes

    Handle<Mesh> createMesh(bool dynamic, const MeshData* meshData);
    Handle<Mesh> createMesh(bool dynamic,
        Primitive primitive,
        const Vector3* vertices, size_t verticesArraySize,
        const unsigned int* indices, size_t indicesArraySize,
        const Vector3* color = nullptr, size_t colorArraySize = 0,
        const Vector2* uv0 = nullptr, size_t uv0ArraySize = 0,
        const Vector2* uv1 = nullptr, size_t uv1ArraySize = 0,
        const Vector3* normals = nullptr, size_t normalsArraySize = 0);
    void destroyMesh(Handle<Mesh> handle);
    void destroyMesh(Mesh* mesh);

    // Renderables
    Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
    void destroyRenderable(Handle<Renderable> handle);
    void destroyRenderable(Renderable* renderable);

    // Scene Node
    //TODO(marcio): Implement createNode() for all node types
    Handle<SceneNode> createNode(
        Handle<Renderable> renderable,
        Vector3& position = Vector3{0.0f, 0.0f, 0.0f},
        Vector3& scale = Vector3{1.0f, 1.0f, 1.0f},
        Vector3& rotationAxis = Vector3{0.0f, 0.0f, 0.0f},
        float rotationAngle = 0,
        Handle<SceneNode> parent = Scene::ROOT);
    Handle<SceneNode> destroyNode(Handle<SceneNode> handle);
    Handle<SceneNode> destroyNode(SceneNode* node);
    Handle<SceneNode> clone(Handle<SceneNode> handle);

    // misc
    Transform* getTransform(Handle<SceneNode> handle);
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H