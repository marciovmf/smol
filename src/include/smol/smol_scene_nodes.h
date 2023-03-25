#ifndef SMOL_SCENE_NODES_H
#define SMOL_SCENE_NODES_H

#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>
#include <smol/smol_renderer_types.h>

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

  struct CameraSceneNode
  {
    Camera camera;
  };

  struct SMOL_ENGINE_API SceneNode
  {
    enum Type : char
    {
      INVALID = -1,
      ROOT = 0, // there must be only ONE roote node in a scene
      MESH,
      SPRITE,
      CAMERA
    };

    Transform transform;
    union
    {
      MeshSceneNode meshNode;
      SpriteSceneNode spriteNode;
      CameraSceneNode cameraNode;
    };

    private:
    Scene& scene;
    bool active = true;   // active state for the node, not the hierarchy
    bool dirty = true;    // changed this frame
    Type type;
    Layer layer;

    public:
    SceneNode(Scene* scene, SceneNode::Type type, const Transform& transform = Transform());
    void setActive(bool status);
    void setDirty(bool value);
    bool isValid() const;
    bool isActive() const;
    bool isDirty() const;
    Type getType() const;
    Layer getLayer() const;
    bool typeIs(Type t) const;
    bool isActiveInHierarchy() const;
    void setParent(Handle<SceneNode> parent);
    void setLayer(Layer l);
  };
}

#endif  // SMOL_SCENE_NODES_H
