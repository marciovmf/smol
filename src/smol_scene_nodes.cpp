#include <smol/smol_transform.h>
#include <smol/smol_scene.h> // also includes smol_scene_nodes.h

namespace smol
{
  struct Scene;

  SceneNode::SceneNode(Scene* scene, SceneNode::Type type, const Transform& transform)
    :transform(transform), scene(*scene), active(true), dirty(true), type(type), layer(Layer::LAYER_0)
  { 
  }

  bool SceneNode::isActiveInHierarchy()
  {
    if (!active)
      return false;

    Handle<SceneNode> parent = transform.getParent();

    if (parent == Scene::ROOT)
      return true;

    SceneNode& parentPtr = scene.getNode(parent);
    if (!parentPtr.isValid())
      return false;

    return parentPtr.isActiveInHierarchy();
  }

  void SceneNode::setParent(Handle<SceneNode> parent)
  {
    // root nodes have no parent
    if (type == SceneNode::ROOT)
      return;

    if (!scene.getNode(parent).isValid())
    {
      Log::error("Trying to set parent of node with an invalid parent node handle");
      transform.setParent(Scene::ROOT);
      return;
    }
    transform.setParent(parent);
  }

  void SceneNode::setActive(bool status)
  {
    if (!isValid())
      return;

    active = status;
    dirty = true;
  }

}
