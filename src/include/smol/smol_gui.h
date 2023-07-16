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
  typedef uint32 GUIControlID;
  struct SystemsRoot;

  struct SMOL_ENGINE_API GUISkin final
  {
    float labelFontSize;
    float lineHeightAdjust;
    float sliderThickness;
    float windowOpacity;
    Handle<Font> font;
    Rectf spriteRadioButton;
    Rectf spriteRadioButtonChecked;
    Rectf spriteCheckBox;
    Rectf spriteCheckBoxChecked;
    Rectf spriteSliderHandle;
    Rectf spriteComboBoxChevron;

    enum ID
    {
      TEXT,
      TEXT_DEBUG_BACKGROUND,
      TEXT_DISABLED,

      MENU,
      MENU_SELECTION,

      COMBO_BOX,
      COMBO_BOX_HOVER,

      BUTTON,
      BUTTON_HOVER,
      BUTTON_ACTIVE,

      TOGGLE_BUTTON,
      TOGGLE_BUTTON_HOVER,
      TOGGLE_BUTTON_HOVER_ACTIVE,
      TOGGLE_BUTTON_ACTIVE,

      CHECKBOX,
      CHECKBOX_HOVER,
      CHECKBOX_ACTIVE,
      CHECKBOX_CHECK,

      SLIDER,
      SLIDER_HANDLE,
      SLIDER_HANDLE_INNER,

      PANEL,

      WINDOW,
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
      MAX_NESTED_AREAS = 16,
      DEFAULT_H_SPACING = 5,
      DEFAULT_CONTROL_HEIGHT = 20,

    };

    StreamBuffer streamBuffer;
    Arena glyphDrawDataArena;
    Handle<Material> material;
    GUISkin skin;
    Rect lastRect;                    // Rect of the last control drawn
    GUIControlID hoverControlId;
    GUIControlID activeControlId;     // tracks statate of controls that have idle/hover/active states like buttons and checkboxes
    GUIControlID draggedControlId;    // tracks sliders and windows being dragged
    GUIControlID topmostWindowId;     // tracks the topmost window
    float currentCursorZ;

    Point2 cursorDragOffset;          // cursor offset related to the control it's dragging 
    uint32 windowCount;
    uint32 popupCount;
    uint32 areaCount;
    float screenW;
    float screenH;
    Rect area[MAX_NESTED_AREAS];
    Rect areaOffset;
    float z;
    //Point2 mousePos;
    Point2 mouseCursorPosition;
    bool mouseLButtonDownThisFrame;
    bool mouseLButtonUpThisFrame;
    bool mouseLButtonIsDown;

    public:

    bool enabled;
    bool changed;
    bool drawLabelDebugBackground;

    enum Align
    {
      LEFT,
      CENTER,
      RIGHT,
      NONE      // use text top-left corner as origin
    };

    Vector2 getScreenSize() const;
    GUISkin& getSkin();
    Rect getLastRect() const;
    void begin(int screenWidth, int32 screenHeight);
    void panel(GUIControlID id, int32 x, int32 y, int32 w, int32 h);
    void horizontalSeparator(int32 x, int32 y, int32 width);
    void verticalSeparator(int32 x, int32 y, int32 height);
    Point2 beginWindow(GUIControlID id, const char* title, int32 x, int32 y, int32 w, int32 h, bool topmost = false);
    void endWindow();

    void beginArea(int32 x, int32 y, int32 w, int32 h);
    void endArea();

    void label(GUIControlID id, const char* text, int32 x, int32 y, int w, Align align = NONE, Color bgColor = Color::NO_COLOR);
    bool doLabelButton(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h, Align align = CENTER, Color bgColor = Color::NO_COLOR);
    bool doButton(GUIControlID id, const char* text, int32 x, int32 y, int32 w, int32 h);
    bool doToggleButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y, int32 w, int32 h);
    bool doRadioButton(GUIControlID id, const char* text, bool toggled, int32 x, int32 y);
    bool doCheckBox(GUIControlID id, const char* text, bool toggled, int32 x, int32 y);
    int32 doPopupMenu(GUIControlID  id, const char** options, uint32 optionCount, uint32 x, uint32 y, uint32 maxWidth);
    int32 doComboBox(GUIControlID  id, const char** options, uint32 optionCount, int32 selectedIndex, uint32 x, uint32 y, uint32 w);
    float doHorizontalSlider(GUIControlID id, float value, int32 x, int32 y, int32 w);
    float doVerticalSlider(GUIControlID id, float value, int32 x, int32 y, int32 h);
    void end();

#ifndef SMOL_MODULE_GAME
    void initialize(Handle<Material> material, Handle<Font> font);
    Handle<Material> getMaterial() const;
#endif

    //private:
    // returns the index of the selected option. Returns -1 if nothing was selected
    int32 doOptionList(GUIControlID  id, const char** options, uint32 optionCount, uint32 x, uint32 y, uint32 minWidth);

  };
}
#endif //SMOL_GUI_H

