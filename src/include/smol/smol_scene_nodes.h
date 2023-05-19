#ifndef SMOL_SCENE_NODES_H
#define SMOL_SCENE_NODES_H

#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_camera.h>

namespace smol
{
  struct Scene;
  struct Renderable;
  struct SpriteBatcher;

  struct Sprite
  {
    Rect rect;
    float width;
    float height;
    Color color;
    int angle;
    Sprite() : color(Color::WHITE){ }
  };

  struct MeshNodeInfo
  {
    Handle<Renderable> renderable;
    virtual ~MeshNodeInfo();
  };

  struct SpriteNodeInfo final : public MeshNodeInfo
  {
    Handle<SpriteBatcher> batcher;
    union 
    {
      Sprite sprite;
      Arena arena;
    };
    int spriteCount;
    SpriteNodeInfo();
    ~SpriteNodeInfo() override;
  };

  struct SMOL_ENGINE_API SceneNode final
  {
    enum Type : char
    {
      INVALID = -1,
      ROOT = 0, // there must be only ONE root node in a scene
      CAMERA,
      MESH,
      SPRITE
    };

    Transform transform;
    union
    {
      MeshNodeInfo mesh;
      SpriteNodeInfo spriteInfo;
      Camera camera;
    };

    private:
    Scene& scene;
    bool active;   // active state for the node, not the hierarchy
    bool dirty;    // changed this frame
    Type type;
    Layer layer;

    public:
    SceneNode();
    ~SceneNode();
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


  template class SMOL_ENGINE_API smol::HandleList<smol::SceneNode>;
  template class SMOL_ENGINE_API smol::Handle<smol::SceneNode>;

}

#endif  // SMOL_SCENE_NODES_H
