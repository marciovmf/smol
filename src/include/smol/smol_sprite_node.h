#ifndef SMOL_SPRITE_NODE_H
#define SMOL_SPRITE_NODE_H

#include <smol/smol_scene_node_common.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>

namespace smol
{
  class Transform;
  struct Vector3;
  struct SpriteBatcher;

  struct SMOL_ENGINE_API SpriteNode final : public NodeComponent
  {
    Handle<SpriteBatcher> batcher;
    Rect rect;
    float width;
    float height;
    Color color;

  static Handle<SceneNode> create(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Transform& transform,
      float width,
      float height,
      const Color& color);

  static void destroy(Handle<SceneNode> handle);

  };
}
#endif //SMOL_SPRITE_NODE_H

