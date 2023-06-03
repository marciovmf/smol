#include <smol/smol_font.h>

namespace smol
{
  Font::Font(FontInfo* info): fontInfo(info) 
  { }

  Handle<Texture> Font::getTexture() const
  { return fontInfo->texture; }

  const char* Font::getName() const
  { return fontInfo->name; }

  uint16 Font::getSize() const
  { return fontInfo->size; }

  uint16 Font::getBase() const
  { return fontInfo->base; }

  uint16 Font::getLineHeight() const
  { return fontInfo->lineHeight; }

  uint16 Font::getKerningCount() const
  { return fontInfo->kerningCount; }

  uint16 Font::getGlyphCount() const
  { return fontInfo->glyphCount; }

  const Kerning* Font::getKernings(int* count) const
  { 
    if (count)
      *count = fontInfo->kerningCount;
    return fontInfo->kerning;
  }

  const Glyph* Font::getGlyphs(int* count) const
  { 
    if (count)
      *count = fontInfo->glyphCount;
    return fontInfo->glyph;
  }


    Vector2 Font::computeString(const char* str,
        Color color,
        GlyphDrawData* drawData,
        float lineHeightScale)
  {
    float y = 0.0f;
    const smol::Kerning* kerning = nullptr;
    uint16 kerningCount = 0;

    int glyphCount;
    const smol::Glyph* glyphList  = getGlyphs(&glyphCount);
    const smol::Kerning* kerningList = getKernings();
    const float lineHeight  = getLineHeight() * lineHeightScale;
    Vector2 bounds(0.0f);
    float advance = 0.0f;

    const Vector2 textureSize = getTexture()->getDimention();
    // scale it to "1%" so it's easier to scale it propperly when rendering different sizes.
    const float scale = 0.01f;

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
            advance = 0.0f;
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
          const float xBounds = (glyph.rect.w + advance) * scale;
          if (xBounds > bounds.x)
          {
            bounds.x = xBounds;
          }

          float glyphY = y - glyph.yOffset;
          // Negative Y because sprites are pushed with flipped Y
          drawData->color = color;
          drawData->position = smol::Vector3(advance + glyph.xOffset + glyphKerning, -glyphY, 0.0f);
          drawData->size = Vector2(glyph.rect.w, glyph.rect.h);

          drawData->position.mult(scale);
          drawData->size.mult(scale);

          // convert UVs from pixels to 0~1 range
          Rectf uvRect;
          uvRect.x = glyph.rect.x / (float) textureSize.x;
          uvRect.y = 1 - (glyph.rect.y /(float) textureSize.y); 
          uvRect.w = glyph.rect.w / (float) textureSize.x;
          uvRect.h = glyph.rect.h / (float) textureSize.y;
          advance += glyph.xAdvance + glyphKerning;
          drawData->uv = uvRect;

          // y bounds

          const float yBounds = abs(glyphY - glyph.rect.h) * scale;
          if ( yBounds > bounds.y)
          {
            bounds.y = yBounds;
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


#ifndef SMOL_MODULE_GAME
  const FontInfo* Font::getFontInfo() const
  {
    return fontInfo;
  }

#endif
}
