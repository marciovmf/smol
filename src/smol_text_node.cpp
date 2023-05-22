#include <smol/smol_renderer_types.h>
#include <smol/smol_text_node.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_transform.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_scene.h>
#include <string.h>

namespace smol
{
  // Helper functions for generating draw data for text based on the current font
  static float computeGlyph(const smol::Glyph& glyph, float x, float y, float z,
      const smol::Kerning* kernings, uint16 kerningCount,
      Handle<SpriteBatcher> batcher,
      smol::Color color,
      Vector2& textureSize,
      GlyphDrawData* drawData)
  {
    const float scale = 0.01f;
    const float width   = glyph.rect.w * scale;
    const float height  = glyph.rect.h * scale;
    const float yOffset = glyph.yOffset * scale;

    // offset by half height because our pivots are at center
    const float xOffset = glyph.xOffset * scale + width/2;
    const float xAdvance = glyph.xAdvance * scale;
    // offset by half height because our pivots are at center
    y -= height/2.0f + yOffset;

    // apply kerning
    float kerning = 0.0f;
    for (int i = 0; i < kerningCount; i++)
    {
      const smol::Kerning& k = kernings[i];
      if(k.second == glyph.id)
      {
        kerning = k.amount * scale;
        break;
      }
    }

    drawData->position = smol::Vector3(x + xOffset + kerning, y, z);
    drawData->size = Vector2(width, height);
    drawData->color = color;

    // convert UVs from pixels to 0 ~ 1 range
    drawData->uv.x = glyph.rect.x / textureSize.x;
    drawData->uv.y = 1 - (glyph.rect.y / textureSize.y);
    drawData->uv.w = glyph.rect.w / textureSize.x;
    drawData->uv.h = glyph.rect.h / textureSize.y;

    return x + xAdvance + kerning;
  }

  static void computeString(const char* str, smol::Handle<smol::Font> font, Vector3& position,
      Handle<SpriteBatcher> batcher, smol::Color color, GlyphDrawData* drawData)
  {
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float advance = x;
    const smol::Kerning* kerning = nullptr;
    uint16 kerningCount = 0;

    int glyphCount;
    const smol::Glyph* glyphList = font->getGlyphs(&glyphCount);

    const smol::Kerning* kerningList = font->getKernings();
    const uint16 lineHeight = font->getLineHeight();
    Vector2 textureSize = batcher->material->textureDiffuse[0]->getDimention();

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
            y -= lineHeight / 100.0f;
            advance = x;
          }
          advance = computeGlyph(glyph, advance, y, z, kerning, kerningCount, batcher, color, textureSize, drawData++);
          // kerning information for the next character
          kerning = &kerningList[glyph.kerningStart];
          kerningCount = glyph.kerningCount;
          break;
        }
      }
      str++;
    }
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
    textNode.arena.initialize(0); // let setText() decide how much to allocate
    textNode.batcher->textNodeCount++;
    textNode.batcher->dirty = true;
    handle->text.setText(text);
    return handle;
  }

  void TextNode::setText(const char* text)
  {
    textLen = strlen(text);
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

    Vector3 position = node->transform.getPosition();

    // Copy the source text
    strncpy(this->text, text, textLen + 1);
    computeString(this->text, font, position, batcher, color, this->drawData);
  }

  const char* TextNode::getText() const
  {
    return text;
  }

  void TextNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::TEXT), "Handle passed to TextNode::destroy() is not of type TEXT");
    SystemsRoot::get()->sceneManager.getLoadedScene().destroyNode(handle);
    handle->sprite.batcher->textNodeCount--;
  }

}
