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
#include <smol/smol_systems_root.h>
#include <smol/smol_sprite_batcher.h>

namespace smol
{
  struct Image;
  struct MeshData;
  struct ResourceManager;
  struct SMOL_ENGINE_API Scene final
  {
    static const Handle<SceneNode> ROOT;

    HandleList<Renderable> renderables;
    HandleList<SceneNode> nodes;
    HandleList<SpriteBatcher> batchers;
    Arena renderKeys;
    Arena renderKeysSorted;
    Handle<smol::Texture> defaultTexture;
    Handle<smol::ShaderProgram> defaultShader;
    Handle<smol::Material> defaultMaterial;
    Mat4 viewMatrix;
    Handle<SceneNode> mainCamera;
    const smol::SceneNode nullSceneNode;

    Scene();
    ~Scene();

    //
    // Resources
    //

    Handle<Renderable> createRenderable(Handle<Material> material, Handle<Mesh> mesh);
    void destroyRenderable(Handle<Renderable> handle);
    void destroyRenderable(Renderable* renderable);

    Handle<SpriteBatcher> createSpriteBatcher(Handle<Material> material, SpriteBatcher::Mode mode = SpriteBatcher::SCREEN, int capacity = 32);

  SpriteBatcher::Mode getSpriteBatcherMode(Handle<SpriteBatcher> handle) const;

  void setSpriteBatcherMode(Handle<SpriteBatcher> handle, SpriteBatcher::Mode mode);

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

    Handle<SceneNode> createPerspectiveCameraNode(float fov, float zNear, float zFar, const Transform& transform);
    Handle<SceneNode> createOrthographicCameraNode(float size, float zNear, float zFar, const Transform& transform);

    void destroyNode(Handle<SceneNode> handle);
    void destroyNode(SceneNode* node);

    void setMainCamera(Handle<SceneNode> handle);

    //
    // misc
    //
    SceneNode& getNode(Handle<SceneNode> handle) const;
  };
}

#undef GLuint

#endif  // SMOL_SCENE_H
