#ifndef SMOL_GUI_H
#define SMOL_GUI_H
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_color.h>
#include <smol/smol_arena.h>
#include <smol/smol_renderer.h>
#include <smol/smol_font.h>

#define SMOL_CONTROL_ID (__LINE__)

namespace smol
{
  typedef uint32 GUICOntrolID;
  struct SystemsRoot;

  struct SMOL_ENGINE_API GUISkin final
  {
    uint16 labelFontSize;
    Handle<Font> font;

    enum ID
    {
      TEXT,
      TEXT_DISABLED,

      BUTTON,
      BUTTON_HOVER,
      BUTTON_ACTIVE,

      FRAME,
      FRAME_HOVER,
      FRAME_ACTIVE,

      SKIN_COLOR_COUNT
    };

    Color color[SKIN_COLOR_COUNT];
  };

  class SMOL_ENGINE_API GUI final
  {
    Handle<Material> material;
    float screenW;
    float screenH;
    Arena glyphDrawDataArena;
    StreamBuffer streamBuffer;
    Rect lastRect;
    GUISkin skin;
    SystemsRoot* root;
    GUICOntrolID hoverControl;
    GUICOntrolID activeControl;

    public:
    Vector2 getScreenSize() const;
    Rect getLastRect() const;
    void begin(int screenWidth, int screenHeight);
    void panel(GUICOntrolID id, int32 x, int32 y, int32 w, int32 h);
    void label(GUICOntrolID id, const char* text, int32 x, int32 y);
    void end();

#ifndef SMOL_MODULE_GAME
    GUI(Handle<Material> material, Handle<Font> font);
    Handle<Material> getMaterial() const;
#endif

  };
}
#endif //SMOL_GUI_H

