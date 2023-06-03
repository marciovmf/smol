#include <smol/smol_handle_list.h>
#include <smol/smol_texture.h>
#include <smol/smol_rect.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_color.h>
#include <smol/smol.h>

#ifndef SMOL_FONT_H
#define SMOL_FONT_H

namespace smol
{
  struct GlyphDrawData
  {
    Vector3 position;
    Vector2 size;
    Rectf uv;           // in texture coords
    Color color;
  };

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
    Rectf rect;
  };

  struct SMOL_ENGINE_API FontInfo
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

  // Because Fonts have a variable size, the Font asset acts as a wrapper around
  // the dynamically allocated FontInfo.
  struct SMOL_ENGINE_API Font
  {
    private:
    const FontInfo* fontInfo;

    public:
    Font(FontInfo* info);
    Handle<Texture> getTexture() const;
    const char* getName() const;
    uint16 getSize() const;
    uint16 getBase() const;
    uint16 getLineHeight() const;
    uint16 getKerningCount() const;
    uint16 getGlyphCount() const;
    const Kerning* getKernings(int* count = nullptr) const; 
    const Glyph* getGlyphs(int* count = nullptr) const; 
#ifndef SMOL_MODULE_GAME
    const FontInfo* getFontInfo() const;
#endif

  Vector2 computeString(const char* str, Color color, GlyphDrawData* drawData, float lineHeightScale = 1.0f);
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::Font>;
  template class SMOL_ENGINE_API smol::Handle<smol::Font>;
}

#endif  // SMOL_FONT_H
