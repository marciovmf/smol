#ifndef SMOL_TEXT_NODE_H
#define SMOL_TEXT_NODE_H

#include <smol/smol_color.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_scene_node_common.h>

namespace smol
{

  struct SpriteBatcher;
  struct SceneNode;
  struct Font;
  struct Vector3;

  struct TextNode final : public NodeComponent
  {
    Arena arena;
    Handle<Font> font;
    Handle<SpriteBatcher> batcher;
    Color color;
    const char* text;

    void setText(const char* text);
    const char* getText() const;

    static Handle<SceneNode> create(
        Handle<SpriteBatcher> batcher,
        Handle<Font> font,
        const Vector3& position,
        const char* text,
        const Color& color = Color::WHITE,
        Handle<SceneNode> parent = INVALID_HANDLE(SceneNode));

    static void destroy(Handle<SceneNode> handle);
  };
}
#endif //SMOL_TEXT_NODE_H

