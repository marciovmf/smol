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
      float maxLineWidth,
      float lineHeightScale)
  {
    int glyphCount = 0;
    uint16 kerningCount = 0;
    const smol::Glyph* glyphList  = getGlyphs(&glyphCount);
    const smol::Kerning* kerning = nullptr;
    const smol::Kerning* kerningList = getKernings();
    const float lineHeight =  (float)getLineHeight();
    Vector2 bounds(0.0f);
    float advance = 0.0f;
    float y = 0.0f;

    const Vector2 textureSize = getTexture()->getDimention();
    while (*str != 0)
    {
      for (int i = 0; i < glyphCount; i++)
      {
        uint16 id = (uint16) *str;
        const smol::Glyph& glyph = glyphList[i];

        if (glyph.id != id)
          continue;

        if ((char)id == '\n')
        {
          advance = 0.0f;
          y -= lineHeight * lineHeightScale;
          bounds.y += lineHeight * lineHeightScale;
        }

        if (bounds.y == 0.0f)
          bounds.y = lineHeight;

        // Check for kerning
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

        float glyphX = 0.0f;
        float glyphY = 0.0f;

        // x bounds
        float xBounds = (glyph.rect.w + advance);
        // Break line when possible

        if ( xBounds / lineHeight > maxLineWidth && maxLineWidth > 0.000f) 
        {
          advance = 0.0f;
          glyphX = 0.0f;
          y -= lineHeight * lineHeightScale;
          bounds.y += lineHeight * lineHeightScale;
        }
        else
        {
          glyphX = advance + glyph.xOffset + glyphKerning;
          if (xBounds > bounds.x)
          {
            bounds.x = xBounds;
          }
        }

        glyphY = y - glyph.yOffset;


        // Negative Y because sprites are pushed with flipped Y
        drawData->color = color;
        drawData->position = smol::Vector3(glyphX, -glyphY, 0.0f);
        drawData->size = Vector2(glyph.rect.w, glyph.rect.h);

        /**
         * We need a way to output text at the same scale regardless of the image
         * size or space each glyph occupies in the image. We achieve this by
         * dividing glyphs coordinates byt the font's lineHeight property which
         * is measured in pixels. As no glyph is expected to exceed the
         * lineHeight, this results in coordinates ranging from 0.0 to 1.0
         */
        drawData->position.div(lineHeight);
        drawData->size.div(lineHeight);

        // convert UVs from pixels to 0~1 range
        Rectf uvRect;
        uvRect.x = glyph.rect.x / (float) textureSize.x;
        uvRect.y = 1 - (glyph.rect.y /(float) textureSize.y); 
        uvRect.w = glyph.rect.w / (float) textureSize.x;
        uvRect.h = glyph.rect.h / (float) textureSize.y;
        advance += glyph.xAdvance + glyphKerning;
        drawData->uv = uvRect;

        // kerning information for the next character
        kerning = &kerningList[glyph.kerningStart];
        kerningCount = glyph.kerningCount;
        break;
      }
      str++;
      drawData++;
    }

    bounds.div(lineHeight);
    return bounds;
  }


#ifndef SMOL_MODULE_GAME
  const FontInfo* Font::getFontInfo() const
  {
    return fontInfo;
  }

#endif
}
