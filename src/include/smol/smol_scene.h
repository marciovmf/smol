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

#define INVALID_HANDLE(T) (Handle<T>{ (int) 0xFFFFFFFF, (int) 0xFFFFFFFF})
#define warnInvalidHandle(typeName) debugLogWarning("Attempting to destroy a '%s' resource from an invalid handle", (typeName))

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

  struct SMOL_ENGINE_API Rect
  {
    int x, y, w, h;
  };

  struct SMOL_ENGINE_API Rectf
  {
    float x, y, w, h;
  };

  struct SMOL_ENGINE_API Color
  {
    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color LIME;
    static const Color BLUE;
    static const Color YELLOW;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color SILVER;
    static const Color GRAY;
    static const Color MAROON;
    static const Color OLIVE;
    static const Color GREEN;
    static const Color PURPLE;
    static const Color TEAL;
    static const Color NAVY;

    float r, g, b, a;

    Color(int r, int g, int b, int a = 255);
    Color(float r, float g, float b, float a = 1.0f);
  };

  struct SMOL_ENGINE_API Texture
  {
    int width;
    int height;
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
    bool dynamic;
    GLuint glPrimitive;
    GLuint vao;
    GLuint ibo;
    GLuint vboPosition;
    GLuint vboNormal;
    GLuint vboUV0;
    GLuint vboUV1;
    GLuint vboColor;
    size_t verticesArraySize;
    size_t indicesArraySize;
    unsigned int numIndices;
    unsigned int numVertices;
  };

  struct SMOL_ENGINE_API Renderable
  {
    Handle<Material> material;
    Handle<Mesh> mesh;
  };

  struct SMOL_ENGINE_API SpriteBatcher
  {
    static const size_t positionsSize;
    static const size_t indicesSize;
    static const size_t colorsSize;
    static const size_t uvsSize;
    static const size_t totalSpriteSize;

    Handle<Renderable> renderable;
    Arena arena;
    int spriteCount;
    int spriteCapacity;
    bool dirty;
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
    bool update();
    const Mat4& getMatrix() const;
    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setRotation(float x, float y, float z, float angle);
    const Vector3& getPosition() const;
    const Vector3& getScale() const;
    bool isDirty() const;
  };

  struct EmptySceneNode { };

  struct MeshSceneNode
  {
    Handle<Renderable> renderable;
  };

  struct SpriteSceneNode
  {
    Handle<SpriteBatcher> batcher;
    Rect rect;
    float width;
    float height;
    Color color;
    int angle;
  };

  struct SceneNode
  {
    enum SceneNodeType : char
    {
      EMPTY = 0,
      MESH,
      SPRITE,
    };

    bool active = true;
    bool dirty = true; // changed this frame
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
template class SMOL_ENGINE_API smol::ResourceList<smol::SpriteBatcher>;

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
    smol::ResourceList<smol::SpriteBatcher> batchers;
    smol::Arena renderKeys;
    smol::Arena renderKeysSorted;
    smol::Handle<smol::Texture> defaultTexture;
    smol::Handle<smol::ShaderProgram> defaultShader;
    smol::Handle<smol::Material> defaultMaterial;
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 projectionMatrix2D;//TODO(marcio): remove this when we have cameras and can assign different cameras to renderables
    Vector3 clearColor;
    ClearOperation clearOperation;

    static const Handle<SceneNode> ROOT;

    Scene();

    void setNodeActive(Handle<SceneNode> handle, bool status);
    bool isNodeActive(Handle<SceneNode> handle);

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
        const Vector3* vertices, int numVertices,
        const unsigned int* indices, int numIndices,
        const Color* color = nullptr,
        const Vector2* uv0 = nullptr,
        const Vector2* uv1 = nullptr,
        const Vector3* normals = nullptr);

    void updateMesh(Handle<Mesh> handle,
        const Vector3* vertices, int numVertices,
        const unsigned int* indices, int numIndices,
        const Color* color = nullptr,
        const Vector2* uv0 = nullptr,
        const Vector2* uv1 = nullptr,
        const Vector3* normals = nullptr);

    void destroyMesh(Handle<Mesh> handle);
    void destroyMesh(Mesh* mesh);

    // Renderables
    Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
    void destroyRenderable(Handle<Renderable> handle);
    void destroyRenderable(Renderable* renderable);

    // Sprite Batcher
    Handle<SpriteBatcher> createSpriteBatcher(Handle<Material> material, int capacity = 32);

    // Scene Node
    //TODO(marcio): Implement createNode() for all node types
    Handle<SceneNode> createMeshNode(
        Handle<Renderable> renderable,
        Vector3& position = Vector3{0.0f, 0.0f, 0.0f},
        Vector3& scale = Vector3{1.0f, 1.0f, 1.0f},
        Vector3& rotationAxis = Vector3{0.0f, 0.0f, 0.0f},
        float rotationAngle = 0,
        Handle<SceneNode> parent = Scene::ROOT);

    Handle<SceneNode> createSpriteNode(
        Handle<SpriteBatcher> batcher,
        Rect& rect,
        Vector3& position,
        float width,
        float height,
        const Color& color = Color::WHITE,
        int angle = 0,
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
