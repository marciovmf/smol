#ifndef SMOL_GUI_H
#define SMOL_GUI_H
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_color.h>
#include <smol/smol_arena.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
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

      TOGGLE_BUTTON,
      TOGGLE_BUTTON_HOVER,
      TOGGLE_BUTTON_HOVER_ACTIVE,
      TOGGLE_BUTTON_ACTIVE,

      FRAME,
      FRAME_HOVER,
      FRAME_ACTIVE,

      WINDOW_TITLE_BAR_HOVER,
      WINDOW_TITLE_BAR,

      SEPARATOR,

      SKIN_COLOR_COUNT
    };

    Color color[SKIN_COLOR_COUNT];
  };

  class SMOL_ENGINE_API GUI final
  {

    enum
    {
      MAX_NESTED_AREAS = 16
    };

    Handle<Material> material;
    float screenW;
    float screenH;
    Arena glyphDrawDataArena;
    StreamBuffer streamBuffer;
    Rect lastRect;
    GUISkin skin;
    SystemsRoot* root;
    GUICOntrolID hoverControlId;
    GUICOntrolID activeControlId;
    GUICOntrolID draggedControlId;
    Point2 cursorDragOffset;

    int windowCount;

    uint32 areaCount;
    Rect area[MAX_NESTED_AREAS];
    Rect areaOffset;

    public:

    enum Align
    {
      LEFT,
      CENTER,
      RIGHT,
      NONE      // use text top-left corner as origin
    };

    Vector2 getScreenSize() const;
    Rect getLastRect() const;
    void begin(int screenWidth, int32 screenHeight);
    void panel(GUICOntrolID id, int32 x, int32 y, int32 w, int32 h);
    void horizontalSeparator(int32 x, int32 y, int32 width);
    void verticalSeparator(int32 x, int32 y, int32 height);
    Point2 beginWindow(GUICOntrolID id, const char* title, int32 x, int32 y, int32 w, int32 h);
    void endWindow();

    void beginArea(int32 x, int32 y, int32 w, int32 h);
    void endArea();

    void label(GUICOntrolID id, const char* text, int32 x, int32 y, Align align = NONE);
    bool doButton(GUICOntrolID id, const char* text, int32 x, int32 y, int32 w, int32 h);
    bool doToggleButton(GUICOntrolID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h);
    void end();

#ifndef SMOL_MODULE_GAME
    GUI(Handle<Material> material, Handle<Font> font);
    Handle<Material> getMaterial() const;
#endif

  };
}
#endif //SMOL_GUI_H

