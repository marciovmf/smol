#ifndef SMOL_SCENE_NODES_H
#define SMOL_SCENE_NODES_H

#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>

namespace smol
{
  struct Scene;
  struct Renderable;
  struct SpriteBatcher;

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

#endif  // SMOL_SCENE_NODES_H
