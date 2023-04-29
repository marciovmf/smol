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

#ifndef SMOL_MODULE_GAME
  const FontInfo* Font::getFontInfo() const
  {
    return fontInfo;
  }

#endif
}
