#include <smol/smol_renderer_types.h>
#include <smol/smol_text_node.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_scene.h>
#include <string.h>

namespace smol
{

  static Vector2 computeString(const char* str, smol::Handle<smol::Font> font,
      Handle<SpriteBatcher> batcher,
      Color color,
      GlyphDrawData* drawData,
      float lineHeightScale)
  {
    float x = 0.0f;
    float y = 0.0f;
    const smol::Kerning* kerning = nullptr;
    uint16 kerningCount = 0;

    int glyphCount;
    const smol::Glyph* glyphList  = font->getGlyphs(&glyphCount);
    const smol::Kerning* kerningList = font->getKernings();
    const float lineHeight  = font->getLineHeight() * lineHeightScale;
    Vector2 bounds(0.0f);
    float advance = x;

    while (*str != 0)
    {
      for (int i = 0; i < glyphCount; i++)
      {
        uint16 id = (uint16) *str;
        const smol::Glyph& glyph = glyphList[i];
        if (glyph.id == id)
        {
          if ((char)id == '\n')
          {
            y -= lineHeight;
            advance = x;
          }

          // apply kerning
          float glyphKerning = 0.0f;
          for (int j = 0; j < kerningCount; j++)
          {
            const smol::Kerning& k = kerning[j];
            if(k.second == glyph.id)
            {
              glyphKerning = k.amount;
              break;
            }
          }

          // x bounds
          if (glyph.rect.w + advance > bounds.x)
            bounds.x = glyph.rect.w + advance;

          float glyphY = y - glyph.yOffset;
          drawData->position = smol::Vector3(advance + glyph.xOffset + glyphKerning, glyphY, 0.0f);
          drawData->size = Vector2(glyph.rect.w, glyph.rect.h);
          drawData->color = color;
          drawData->uv = glyph.rect;
          advance += glyph.xAdvance + glyphKerning;

          // y bounds
          if (abs(glyphY - glyph.rect.h)  > bounds.y)
          {
            bounds.y = abs(glyphY - glyph.rect.h);
          }

          // kerning information for the next character
          kerning = &kerningList[glyph.kerningStart];
          kerningCount = glyph.kerningCount;
          break;
        }
      }
      str++;
      drawData++;
    }

    return bounds;
  }

  Handle<SceneNode> TextNode::create(
      Handle<SpriteBatcher> batcher,
      Handle<Font> font,
      const Transform& transform,
      const char* text,
      const Color& color)
  {
    Scene& scene = SystemsRoot::get()->sceneManager.getLoadedScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::TEXT, transform);
    TextNode& textNode = scene.getNode(handle).text;
    textNode.batcher = batcher;
    textNode.font = font;
    textNode.node = handle;
    textNode.color = color;
    textNode.bgColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
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
    this->textBounds = computeString(this->text, font, batcher, color, this->drawData+1, lineHeightScale);

    background->position = Vector3(0.0f, 0.0f, -0.1f);
    background->color = bgColor;
    background->uv = Rectf();
    background->size = this->textBounds;
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

  void TextNode::setLineHeightScale(float scale) { lineHeightScale = scale; }

  float TextNode::getLineHeightScale() const { return lineHeightScale; }

  void TextNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::TEXT), "Handle passed to TextNode::destroy() is not of type TEXT");
    SystemsRoot::get()->sceneManager.getLoadedScene().destroyNode(handle);
    handle->sprite.batcher->textNodeCount--;
  }

}
