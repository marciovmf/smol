#include <smol/smol_renderer_types.h>
#include <smol/smol_text_node.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_scene_manager.h>
#include <smol/smol_scene.h>
#include <smol/smol_font.h>
#include <string.h>

namespace smol
{

  Handle<SceneNode> TextNode::create(
      Handle<SpriteBatcher> batcher,
      Handle<Font> font,
      const Transform& transform,
      const char* text,
      const Color& color)
  {
    Scene& scene = SceneManager::get().getCurrentScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::TEXT, transform);
    TextNode& textNode = handle->text;
    textNode.batcher = batcher;
    textNode.font = font;
    textNode.node = handle;
    textNode.color = color;
    textNode.bgColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    textNode.drawBackground = false;
    textNode.arena.initialize(0); // let setText() decide how much to allocate
    textNode.batcher->textNodeCount++;
    textNode.batcher->dirty = true;
    textNode.lineHeightScale = 1.0f;
    handle->text.setText(text);
    return handle;
  }

  void TextNode::setText(const char* text)
  {
    textLen = strlen(text) + 1;
    size_t memSize = textLen + 1 + textLen * sizeof(GlyphDrawData);
    if (arena.getCapacity() == 0)
    {
      arena.initialize(memSize);
    }
    else
    {
      arena.reset();
    }

    char* memory =  (char*) arena.pushSize(memSize);
    this->drawData = (GlyphDrawData*) memory;
    this->text = memory + (sizeof(GlyphDrawData) * textLen);

    // Copy the source text
    strncpy(this->text, text, textLen + 1);

    GlyphDrawData* background = drawData;
    this->textBounds = font->computeString(this->text, color, this->drawData+1, 0.0f, lineHeightScale);

    background->position = Vector3(0.0f, 0.0f, -0.2f);
    background->color = bgColor;
    background->uv = Rectf();
    if (drawBackground || true)
      background->size = this->textBounds;
    else
      background->size = Vector2(0.0f);
  }

  const char* TextNode::getText() const
  {
    return text;
  }

  void TextNode::setBackgroundColor(Color color) 
  {
    this->bgColor = color; 
    if (drawData)
      drawData->color = color;
  }


  void TextNode::setTextColor(Color color)
  {
    this->color = color;
    if (drawData)
    {
      // start at 1 to skip the background
      for (int i = 1; i < textLen; i++) 
      {
        drawData[i].color = color;
      }
    }
  }

  Color TextNode::getTextColor() const
  {
    return color;
  }

  Color TextNode::getBackgroundColor() const { return bgColor; }

  void TextNode::enableTextBackground(bool value)
  {
    drawBackground = value;
    if (drawData)
    {
      if (drawBackground)
        drawData[0].size = this->textBounds;
      else
        drawData[0].size = Vector2(0.0f);
    }
  }

  bool TextNode::isTextBackgroundEnabled() const
  {
    return drawBackground;
  }

  void TextNode::setLineHeightScale(float scale) { lineHeightScale = scale; }

  float TextNode::getLineHeightScale() const { return lineHeightScale; }

  void TextNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::TEXT), "Handle passed to TextNode::destroy() is not of type TEXT");
    SceneManager::get().getCurrentScene().destroyNode(handle);
    handle->sprite.batcher->textNodeCount--;
  }

}
