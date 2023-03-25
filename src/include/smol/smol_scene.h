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
#include <smol/smol_scene_nodes.h>


#define warnInvalidHandle(typeName) debugLogWarning("Attempting to destroy a '%s' resource from an invalid handle", (typeName))

template class SMOL_ENGINE_API smol::HandleList<smol::Mesh>;
template class SMOL_ENGINE_API smol::HandleList<smol::Renderable>;
template class SMOL_ENGINE_API smol::HandleList<smol::SpriteBatcher>;
template class SMOL_ENGINE_API smol::HandleList<smol::SceneNode>;

namespace smol
{
  struct Image;
  struct MeshData;
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

    HandleList<smol::Mesh> meshes;
    HandleList<smol::Renderable> renderables;
    HandleList<smol::SceneNode> nodes;
    HandleList<smol::SpriteBatcher> batchers;
    Arena renderKeys;
    Arena renderKeysSorted;
    Handle<smol::Texture> defaultTexture;
    Handle<smol::ShaderProgram> defaultShader;
    Handle<smol::Material> defaultMaterial;
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Handle<SceneNode> mainCamera;
    Mat4 projectionMatrix2D;//TODO(marcio): remove this when we have cameras and can assign different cameras to renderables
    Vector3 clearColor;
    ClearOperation clearOperation;
    const smol::SceneNode nullSceneNode;

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
    Handle<SceneNode> createMeshNode(Handle<Renderable> renderable, const Transform& transform = Transform());

    Handle<SceneNode> createSpriteNode(
        Handle<SpriteBatcher> batcher,
        const Rect& rect,
        const Vector3& position,
        float width,
        float height,
        const Color& color = Color::WHITE,
        int angle = 0,
        Handle<SceneNode> parent = Scene::ROOT);

    Handle<SceneNode> createPerspectiveCameraNode(float fov, float aspect, float zNear, float zFar, const Transform& transform);
    Handle<SceneNode> createOrthographicCameraNode(float left, float right, float top, float bottom, float zNear, float zFar, const Transform& transform);

    void destroyNode(Handle<SceneNode> handle);
    void destroyNode(SceneNode* node);
    Handle<SceneNode> clone(Handle<SceneNode> handle);

    void setMainCamera(Handle<SceneNode> handle);

    //
    // misc
    //
    SceneNode& getNode(Handle<SceneNode> handle) const;
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H
