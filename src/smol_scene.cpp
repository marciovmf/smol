#include <smol/smol_scene.h>
#include <smol/smol_scene_node.h>
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
    return batchers.add(SpriteBatcher(material, mode, capacity));
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
      //destroyRenderable(batcher->renderable);
      //destroyMaterial(batcher->material);
      batchers.remove(handle);
    }
  }

  //
  // Scene Node utility functions
  //

#ifndef SMOL_MODULE_GAME
  Handle<SceneNode> Scene::createNode(SceneNode::Type type, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, type, transform));
    handle->setDirty(true);
    return handle;
  }
#endif

#ifndef SMOL_MODULE_GAME
  void Scene::destroyNode(Handle<SceneNode> handle)
  {
    nodes.remove(handle);
  }
#endif

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
