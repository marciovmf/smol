#include "smol_editor.h"
#include "smol_editor_icons.h"
#include <smol/smol_log.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_font.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer.h>
#include <smol/smol_gui.h>
#include <smol/smol_point.h>
#include <smol/smol_event.h>
#include <smol/smol_event_manager.h>
#include <string.h>

namespace smol
{
  Point2 windowPos = Point2{550, 150};
  Point2 windowPos2 = Point2{100, 300};
  bool radioOption = false;
  bool breakText = true;
  int32 comboValue = 0;
  bool showSecondWindow = false;
  float skinValue[] = {0.0f, 1.0f, 1.0f, 0.2f};

  bool callbackForward(const Event& event, void* payload)
  {
    Editor* instance = (Editor*) payload;
    return instance->onEvent(event);
  }

  void Editor::initialize()
  {
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
    Handle<Font> uiFont = resourceManager.loadFont("assets/font/segoeui.font");
    Handle<Material> uiMaterial = resourceManager.loadMaterial("assets/ui.material");
    uiMaterial->setSampler2D("mainTex", uiFont->getTexture());
    gui.initialize(uiMaterial, uiFont);

    GUISkin& skin = gui.getSkin();
    skin.spriteCheckBox = iconCHECKBOX();
    skin.spriteCheckBoxChecked = iconCHECKBOX_CHECKED();
    skin.spriteRadioButton = iconRADIO();
    skin.spriteRadioButtonChecked = iconRADIO_CHECKED();
    skin.spriteSliderHandle = iconSLIDER_HANDLE();
    skin.spriteComboBoxChevron = iconCHEVRON_DOWN();

    EventManager::get().addHandler(callbackForward, Event::TEXT);
  }

  void Editor::render(int windowWidth, int windowHeight)
  {
    Renderer::setMaterial(gui.getMaterial());
    Renderer::setViewport(0, 0, windowWidth, windowHeight);

    char text[128];
    const int buttonHeight = 30;
    const int vSpacing = 5;
    int yPos;

    gui.begin(windowWidth, windowHeight);

    if (showSecondWindow) 
    {
      const char* menuOptions[] = {"Font Size", "Line Spacing", "Window Opacity", "Slider thickness"};
      // Window 2
      windowPos2 = gui.beginWindow(SMOL_CONTROL_ID, "Another Test window", windowPos2.x, windowPos2.y, 300, 400);
      yPos = 5;

      GUISkin& skin = gui.getSkin();
      const int numOptions = sizeof(menuOptions)/sizeof(char*);
      comboValue = gui.doComboBox(SMOL_CONTROL_ID, menuOptions, numOptions, comboValue, 5, yPos, 290);
      yPos += vSpacing + buttonHeight;

      float sliderValue = skinValue[comboValue];

      sliderValue = gui.doHorizontalSlider(SMOL_CONTROL_ID, sliderValue, 5, yPos, 290);
      skinValue[comboValue] = sliderValue;
      yPos += vSpacing + buttonHeight;

      // Font size
      if (comboValue == 0)
        skin.labelFontSize = 16 + (8 * sliderValue);
      else if (comboValue == 1)
        // between 0 and 1
        skin.lineHeightAdjust = 2 * sliderValue - 1;
      else if (comboValue == 2)
        skin.windowOpacity = sliderValue;
      else if (comboValue == 3)
        skin.sliderThickness = sliderValue;

      if (gui.doButton(SMOL_CONTROL_ID, "Close", 5, yPos, 290, buttonHeight))
        showSecondWindow = false;
      gui.endWindow();
    }

    // Window 1
    windowPos = gui.beginWindow(SMOL_CONTROL_ID, "Test window", windowPos.x, windowPos.y, 300, 800);
    yPos = 5;

    if (gui.doButton(SMOL_CONTROL_ID, "Toggle second widow", 5, yPos, 290, buttonHeight))
      showSecondWindow = !showSecondWindow;
    yPos += vSpacing + buttonHeight;

    if (gui.doButton(SMOL_CONTROL_ID, "Quit", 5, yPos, 290, buttonHeight))
    {
      Event evt;
      evt.type = Event::GAME;
      evt.gameEvent.id = 0xFFFFFFFF;
      EventManager::get().pushEvent(evt);
    }
    yPos += vSpacing + buttonHeight;

    snprintf(text, 128, "Text debug background : '%s'", gui.drawLabelDebugBackground ? "On":"Off");
    gui.drawLabelDebugBackground = gui.doCheckBox(SMOL_CONTROL_ID, text, gui.drawLabelDebugBackground, 5, yPos);
    yPos += vSpacing + buttonHeight;

    breakText = gui.doCheckBox(SMOL_CONTROL_ID, "Break Text", breakText, 5, yPos);
    yPos += vSpacing + buttonHeight;

    gui.label(SMOL_CONTROL_ID, "Officially recognised by the Duden - Germany's pre-eminent dictionary - as the longest word in German, Kraftfahrzeug-Haftpflichtversicherung is a 36-letter, tongue-tying way of describing a rather, mundane everyday concept: motor vehicle liability insurance", 5, yPos, breakText ? 290 : 0);
    gui.endWindow();

    gui.end();

  }

  bool Editor::onEvent(const Event& event)
  {
    //debugLogInfo("TextEvent %c", (char)event.textEvent.character);
    return true;
  }

  void Editor::terminate()
  {
  }

}
