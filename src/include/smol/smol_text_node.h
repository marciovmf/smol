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
  struct SpriteBatcher;
  struct Font;

  struct GlyphDrawData
  {
    Vector3 position;
    Vector2 size;
    Rectf uv;           // in texture coords
    Color color;
  };

  struct SMOL_ENGINE_API TextNode final : public NodeComponent
  {
    Arena arena;
    Handle<Font> font;
    Handle<SpriteBatcher> batcher;
    Color color;
    Color bgColor;
    Vector3 center;
    Vector2 textBounds;
    size_t textLen;
    float lineHeightScale;
    char* text;
    GlyphDrawData* drawData;

    void setText(const char* text);
    const char* getText() const;

    static Handle<SceneNode> create(
        Handle<SpriteBatcher> batcher,
        Handle<Font> font,
        const Transform& transform,
        const char* text,
        const Color& color = Color::WHITE);

    void setTextColor(Color color);
    Color getTextColor() const;
    void setBackgroundColor(Color color);
    Color getBackgroundColor() const;
    void setLineHeightScale(float scale);
    float getLineHeightScale() const;
    static void destroy(Handle<SceneNode> handle);
  };

}
#endif //SMOL_TEXT_NODE_H

