#ifndef SMOL_SCENE_H
#define SMOL_SCENE_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
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

  struct MeshSceneNode
  {
    Handle<Renderable> renderable;
  };

  struct SpriteSceneNode : public MeshSceneNode
  {
    Handle<SpriteBatcher> batcher;
    Rect rect;
    float width;
    float height;
    Color color;
    int angle;
  };

  struct SMOL_ENGINE_API SceneNode
  {
    enum Type : char
    {
      INVALID = -1,
      ROOT = 0, // there must be only ONE roote node in a scene
      MESH,
      SPRITE,
    };

    Transform transform;
    union
    {
      MeshSceneNode meshNode;
      SpriteSceneNode spriteNode;
    };

    private:
    Scene& scene;
    bool active = true;   // active state for the node, not the hierarchy
    bool dirty = true;    // changed this frame
    Type type;

    public:
    SceneNode(Scene* scene, SceneNode::Type type, const Transform& transform = Transform());
    void setActive(bool status);
    inline bool isValid()  { return type != SceneNode::Type::INVALID; }
    inline bool isActive() { return active; }
    inline bool isDirty() { return dirty; }
    inline void setDirty(bool value) { dirty = value; }
    inline Type getType() { return type; }
    inline bool typeIs(Type t) { return type == t; }
    bool isActiveInHierarchy();
    void setParent(Handle<SceneNode> parent);
  };
}

template class SMOL_ENGINE_API smol::HandleList<smol::Mesh>;
template class SMOL_ENGINE_API smol::HandleList<smol::Renderable>;
template class SMOL_ENGINE_API smol::HandleList<smol::SpriteBatcher>;
template class SMOL_ENGINE_API smol::HandleList<smol::SceneNode>;

namespace smol
{
  struct ResourceManager;
  struct SMOL_ENGINE_API Scene final
  {
    static const Handle<SceneNode> ROOT;

    enum ClearOperation
    {
      DONT_CLEAR = 0,
      COLOR_BUFFER = 1,
      DEPTH_BUFFER = 1 << 1
    };

    smol::HandleList<smol::Mesh> meshes;
    smol::HandleList<smol::Renderable> renderables;
    smol::HandleList<smol::SceneNode> nodes;
    smol::HandleList<smol::SpriteBatcher> batchers;
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
    const smol::SceneNode& nullSceneNode;

    Scene(ResourceManager& resourceManager);

    //
    // Resources
    //
   
    Handle<Mesh> createMesh(bool dynamic, const MeshData& meshData);
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
    // Scene Node creation
    //
    Handle<SceneNode> createMeshNode(
        Handle<Renderable> renderable,
        Transform& transform = Transform());

    Handle<SceneNode> createSpriteNode(
        Handle<SpriteBatcher> batcher,
        const Rect& rect,
        const Vector3& position,
        float width,
        float height,
        const Color& color = Color::WHITE,
        int angle = 0,
        Handle<SceneNode> parent = Scene::ROOT);

    void destroyNode(Handle<SceneNode> handle);
    void destroyNode(SceneNode* node);
    Handle<SceneNode> clone(Handle<SceneNode> handle);

    //
    // misc
    //
    SceneNode& getNode(Handle<SceneNode> handle);
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H
