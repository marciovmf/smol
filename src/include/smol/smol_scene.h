#ifndef SMOL_SCENE_H
#define SMOL_SCENE_H

#include <smol/smol_engine.h>
#include <smol/smol_resource_list.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_transform.h>
#include <smol/smol_color.h>

#define warnInvalidHandle(typeName) debugLogWarning("Attempting to destroy a '%s' resource from an invalid handle", (typeName))

namespace smol
{
  struct Image;
  struct MeshData;
  struct Scene;

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
    Scene& scene;
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

    SceneNode(Scene* scene);
    bool isActive();
    bool isActiveInHierarchy();
    void setActive(bool status);
    void setParent(Handle<SceneNode> parent);
  };
}

template class SMOL_ENGINE_API smol::ResourceList<smol::ShaderProgram>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Texture>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Material>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Mesh>;
template class SMOL_ENGINE_API smol::ResourceList<smol::Renderable>;
template class SMOL_ENGINE_API smol::ResourceList<smol::SpriteBatcher>;
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

    // Shaders
    Handle<ShaderProgram> Scene::createShader(const char* vsFilePath, const char* fsFilePath, const char* gsFilePath = nullptr);
    Handle<ShaderProgram> Scene::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource = nullptr);
    void destroyShader(Handle<ShaderProgram> handle);
    void destroyShader(ShaderProgram* program);

    //
    // Resources
    //
    Handle<Texture> Scene::createTexture(const char* bitmapPath); //TODO: Add texture filtering options here
    Handle<Texture> Scene::createTexture(const Image& image); //TODO: Add texture filtering options here
    void destroyTexture(Handle<Texture> handle);
    void destroyTexture(Texture* texture);

    Handle<Material> createMaterial(Handle<ShaderProgram> shader, Handle<Texture>* diffuseTextures, int diffuseTextureCount);
    void destroyMaterial(Handle<Material> handle);

    Handle<Mesh> createMesh(bool dynamic, const MeshData* meshData);
    Handle<Mesh> createMesh(bool dynamic,
        Primitive primitive,
        const Vector3* vertices, int numVertices,
        const unsigned int* indices, int numIndices,
        const Color* color = nullptr,
        const Vector2* uv0 = nullptr,
        const Vector2* uv1 = nullptr,
        const Vector3* normals = nullptr);

    void updateMesh(Handle<Mesh> handle, MeshData* meshData);

    void destroyMesh(Handle<Mesh> handle);
    void destroyMesh(Mesh* mesh);

    Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
    void destroyRenderable(Handle<Renderable> handle);
    void destroyRenderable(Renderable* renderable);

    Handle<SpriteBatcher> createSpriteBatcher(Handle<Material> material, int capacity = 32);
    void destroySpriteBatcher(Handle<SpriteBatcher> handle);

    //
    // Scene Node utility functions
    //

    void setNodeActive(Handle<SceneNode> handle, bool status);
    bool isNodeActive(Handle<SceneNode> handle);
    bool isNodeActiveInHierarchy(Handle<SceneNode> handle);

    //
    // Scene Node creation
    //
    Handle<SceneNode> createMeshNode(
        Handle<Renderable> renderable,
        Vector3& position = Vector3{0.0f, 0.0f, 0.0f},
        Vector3& scale = Vector3{1.0f, 1.0f, 1.0f},
        Vector3& rotationAxis = Vector3{0.0f, 0.0f, 0.0f},
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

    //
    // misc
    //
    SceneNode* getNode(Handle<SceneNode> handle);
    Transform* getTransform(Handle<SceneNode> handle);
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H
