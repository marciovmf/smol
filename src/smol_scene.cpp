#include <smol/smol_scene.h>
#include <smol/smol_scene_nodes.h>
#include <smol/smol_platform.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_systems_root.h>
#include <string.h>
#include <utility>

#define warnInvalidHandle(typeName) debugLogWarning("Attempting to reference a '%s' resource from an invalid handle", (typeName))
namespace smol
{
  // First node handle always points to the ROOT scene node
  const Handle<SceneNode> Scene::ROOT = (Handle<SceneNode>{ (int) 0, (int) 0 });

  Scene::Scene():
    renderables(1024 * sizeof(Renderable)),
    nodes(1024 * sizeof(SceneNode)), 
    batchers(8 * sizeof(SpriteBatcher)),
    renderKeys(1024 * sizeof(uint64)),
    renderKeysSorted(1024 * sizeof(uint64)),
    mainCamera(INVALID_HANDLE(SceneNode)),
    nullSceneNode(this, SceneNode::Type::INVALID)
  {
    viewMatrix = Mat4::initIdentity();

    // Creates a ROOT node
    nodes.add((const SceneNode&) SceneNode(this, SceneNode::Type::ROOT));
  }

  //---------------------------------------------------------------------------
  // Scene
  //---------------------------------------------------------------------------

  Handle<Renderable> Scene::createRenderable(Handle<Material> material, Handle<Mesh> mesh)
  {
    Handle<Renderable> handle = renderables.add(Renderable(material, mesh));
    Renderable* renderable = renderables.lookup(handle);
    if (!renderable)
    {
      return INVALID_HANDLE(Renderable);
    }
    return handle;
  }

  void Scene::destroyRenderable(Handle<Renderable> handle)
  {
    Renderable* renderable = renderables.lookup(handle);
    if(!renderable)
    {
      warnInvalidHandle("Renderable");
    }
    else
    {
      renderables.remove(handle);
    }
  }

  // ##################################################################
  //  SpriteBatcher handling 
  // ##################################################################
  Handle<SpriteBatcher> Scene::createSpriteBatcher(Handle<Material> material, SpriteBatcher::Mode mode, int capacity)
  {
    return batchers.add(std::move(SpriteBatcher(material, mode, capacity)));
  }


  void Scene::destroySpriteBatcher(Handle<SpriteBatcher> handle)
  {
    SpriteBatcher* batcher = batchers.lookup(handle);
    if(!batcher)
    {
      warnInvalidHandle("SpriteBatcher");
    }
    else
    {
      destroyRenderable(batcher->renderable);
      batchers.remove(handle);
    }
  }

  //
  // Scene Node utility functions
  //
  Handle<SceneNode> Scene::createMeshNode(Handle<Renderable> renderable, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::MESH, transform));
    nodes.lookup(handle)->mesh.renderable = renderable;
    return handle;
  }

  Handle<SceneNode> Scene::createSpriteNode(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Vector3& position,
      float width,
      float height,
      const Color& color,
      int angle,
      Handle<SceneNode> parent)
  {
    const Transform& t = Transform(
        position,
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        parent);

    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::SPRITE, t));
    SceneNode* node = nodes.lookup(handle);

    node->sprite.rect = rect;
    node->sprite.batcher = batcher;
    node->sprite.width = width;
    node->sprite.height = height;
    node->sprite.angle = angle;
    node->sprite.color = color;


    SpriteBatcher* batcherPtr = batchers.lookup(batcher);
    if (batcherPtr)
    {
      // copy the renderable handle to the node level
      node->sprite.renderable = batcherPtr->renderable;
      batcherPtr->spriteCount++;
      batcherPtr->dirty = true;
    }

    return handle;
  }

  Handle<SceneNode> Scene::createPerspectiveCameraNode(float fov, float zNear, float zFar, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, transform));
    SceneNode* node = nodes.lookup(handle);
    node->camera = Camera(Camera::PERSPECTIVE, fov, zNear, zFar);
    return handle;
  }

  Handle<SceneNode> Scene::createOrthographicCameraNode(float size, float zNear, float zFar, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, transform));
    SceneNode* node = nodes.lookup(handle);
    node->camera = Camera(Camera::ORTHOGRAPHIC, size, zNear, zFar);
    return handle;
  }

  void Scene::setMainCamera(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if (!node || !node->typeIs(SceneNode::Type::CAMERA))
      return;

    mainCamera = handle;
  }

  Handle<SceneNode> Scene::clone(Handle<SceneNode> handle)
  {
    Handle<SceneNode> newHandle = nodes.reserve();
    SceneNode* newNode = nodes.lookup(newHandle);
    SceneNode* original = nodes.lookup(handle);
    memcpy(newNode, original, sizeof(SceneNode));

    //TODO(marcio): this won't work for sprites because it does not update spriteCount on the spriteBatcher. Fix it!
    return newHandle;
  }

  SceneNode& Scene::getNode(Handle<SceneNode> handle) const
  {
    SceneNode* node = nodes.lookup(handle);
    if(node)
      return *node;

    warnInvalidHandle("SceneNode");
    return (SceneNode&) nullSceneNode;
  }

  SpriteBatcher::Mode Scene::getSpriteBatcherMode(Handle<SpriteBatcher> handle) const
  {
    SpriteBatcher* batcher = batchers.lookup(handle);
    if(!batcher)
    {
      warnInvalidHandle("SpriteBatcher");
      return SpriteBatcher::CAMERA;
    }

    return batcher->mode;
  }

  void Scene::setSpriteBatcherMode(Handle<SpriteBatcher> handle, SpriteBatcher::Mode mode)
  {
    SpriteBatcher* batcher = batchers.lookup(handle);
    if(!batcher)
    {
      warnInvalidHandle("SpriteBatcher");
      return;
    }

    batcher->mode = mode;
  }


  void Scene::destroyNode(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if (node)
    {
      nodes.remove(handle);
    }
  }

  Scene::~Scene()
  {
    debugLogInfo("Scene Released Renderable x%d, SpriteBatcher x%d, SceneNode x%d.", 
        renderables.count(),
        batchers.count(),
        nodes.count());
  }

}

#undef INVALID_HANDLE
#undef warnInvalidHandle
