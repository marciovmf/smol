#ifndef SMOL_TEXT_NODE_H
#define SMOL_TEXT_NODE_H

#include <smol/smol_color.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_scene_node_common.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <smol/smol_rect.h>
#include <smol/smol_color.h>

namespace smol
{
  class Transform;

  struct GlyphDrawData
  {
    Vector3 position;
    Vector2 size;
    Rectf uv;
    Color color;
  };

  struct SpriteBatcher;
  struct SceneNode;
  struct Font;
  struct Vector3;
  struct GlyphDrawData;

  struct SMOL_ENGINE_API TextNode final : public NodeComponent
  {
    Arena arena;
    Handle<Font> font;
    Handle<SpriteBatcher> batcher;
    Color color;
    char* text;
    GlyphDrawData* drawData;
    size_t textLen;

    void setText(const char* text);
    const char* getText() const;

    static Handle<SceneNode> create(
        Handle<SpriteBatcher> batcher,
        Handle<Font> font,
        const Transform& transform,
        const char* text,
        const Color& color = Color::WHITE);


    static void destroy(Handle<SceneNode> handle);
  };
}
#endif //SMOL_TEXT_NODE_H

