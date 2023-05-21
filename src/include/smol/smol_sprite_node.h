#ifndef SMOL_SPRITE_NODE_H
#define SMOL_SPRITE_NODE_H

#include <smol/smol_scene_node_common.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>

namespace smol
{
  struct SpriteBatcher;
  struct Vector3;

  struct SMOL_ENGINE_API SpriteNode final : public NodeComponent
  {
    Handle<SpriteBatcher> batcher;
    Rect rect;
    float width;
    float height;
    Color color;
    int angle;

  static Handle<SceneNode> create(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Vector3& position,
      float width,
      float height,
      const Color& color,
      int angle = 0,
      Handle<SceneNode> parent = INVALID_HANDLE(SceneNode));

  static void destroy(Handle<SceneNode> handle);

  };
}
#endif //SMOL_SPRITE_NODE_H

