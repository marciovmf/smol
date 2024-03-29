#include <smol/smol_transform.h>
#include <smol/smol_scene.h> // also includes smol_scene_nodes.h
#include <smol/smol_renderer_types.h>

namespace smol
{
  struct Scene;

  //---------------------------------------------------------------------------
  // SceneNode
  //---------------------------------------------------------------------------
  SceneNode::~SceneNode() { }

  SceneNode::SceneNode(Scene* scene, SceneNode::Type type, const Transform& transform)
    :transform(transform), scene(*scene), active(true), dirty(true), type(type), layer(Layer::LAYER_0)
  { 
  }

  void SceneNode::setActive(bool status)
  {
    if (!isValid())
      return;

    active = status;
    dirty = true;
  }

  void SceneNode::setDirty(bool value) { dirty = value; }

  bool SceneNode::isValid() const { return type != SceneNode::Type::INVALID; }

  bool SceneNode::isActive() const { return active; }

  bool SceneNode::isDirty() const { return dirty; }

  SceneNode::Type SceneNode::getType() const { return type; }

  Layer SceneNode::getLayer() const { return layer; }

  bool SceneNode::typeIs(Type t) const { return type == t; }

  bool SceneNode::isActiveInHierarchy() const
  {
    if (!active)
      return false;

    Handle<SceneNode> parent = transform.getParent();

    if (parent == INVALID_HANDLE(SceneNode))
      return true;

    SceneNode& parentPtr = *(parent.operator->());
    if (!parentPtr.isValid())
      return false;

    return parentPtr.isActiveInHierarchy();
  }

  void SceneNode::setParent(Handle<SceneNode> parent)
  {
    const SceneNode& parentNode = *(parent.operator->());
    if (! parentNode.isValid())
    {
      Log::error("Trying to set parent of node with an invalid parent node handle");
      transform.setParent(INVALID_HANDLE(SceneNode));
      return;
    }
    transform.setParent(parent);
  }

  void SceneNode::setLayer(smol::Layer l) { layer = l; }
}
