#include <smol/smol_handle_list.h>
#include <smol/smol.h>

#ifndef SMOL_FONT_H
#define SMOL_FONT_H

namespace smol
{
  struct SMOL_ENGINE_API Kerning
  {
    uint16 first;
    uint16 second;
    int16 amount;
  };

  struct SMOL_ENGINE_API Glyph
  {
    uint16 id;
    uint16 kerningCount;    // How many kenings are there from this gliph
    uint16 kerningStart;    // Index of first kerning from this gliph
    int16 xAdvance;
    int16 xOffset;
    int16 yOffset;
    Rect rect;
  };

  struct SMOL_ENGINE_API Font
  {
    uint16 size;
    uint16 lineHeight;
    uint16 base;
    uint16 kerningCount;
    uint16 glyphCount;
    Kerning* kerning;
    Glyph* glyph;
    const char* name;
    Handle<Texture> texture;
  };
}

#endif  // SMOL_FONT_H
