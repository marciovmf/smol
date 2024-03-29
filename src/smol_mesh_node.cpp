
#include <smol/smol_mesh_node.h>
#include <smol/smol_transform.h>
#include <smol/smol_scene_manager.h>

namespace smol
{
  Handle<SceneNode> MeshNode::create(Handle<Renderable> renderable, Transform& transform)
  {
    Scene& scene = SceneManager::get().getCurrentScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::MESH, transform);
    handle->mesh.renderable = renderable;
    return handle;
  }

  void MeshNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::MESH), "Handle passed to MeshNode::destroy() is not of type MESH");
    SceneManager::get().getCurrentScene().destroyNode(handle);
  }
}
