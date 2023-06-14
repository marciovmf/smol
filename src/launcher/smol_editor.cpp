#include "smol_editor.h"
#include "smol_editor_icons.h"
#include <smol/smol_log.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_font.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer.h>
#include <smol/smol_gui.h>
#include <smol/smol_point.h>
#include <string.h>

namespace smol
{
  Point2 windowPos = Point2{550, 150};
  bool wireframeMode = false;
  bool radioOption = false;
  float sliderLineThickness = 0.2f;
  float sliderHandleThickness = 0.2f;

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
  }

  void Editor::render(int windowWidth, int windowHeight)
  {
    Renderer::setMaterial(gui.getMaterial());
    Renderer::setViewport(0, 0, windowWidth, windowHeight);

    char text[128];
    const int buttonHeight = 30;
    const int vSpacing = 5;
    int yPos = 5;

    gui.begin(windowWidth, windowHeight);

    windowPos = gui.beginWindow(SMOL_CONTROL_ID, "Test window", windowPos.x, windowPos.y, 300, 450);
      if (gui.doButton(SMOL_CONTROL_ID, "Button 1", 5, yPos, 290, buttonHeight))
        debugLogInfo("Button 1 clicked!");
      yPos += vSpacing + buttonHeight;

      snprintf(text, 128, "Wireframe mode: '%s'", wireframeMode ? "On":"Off");
      wireframeMode = gui.doCheckBox(SMOL_CONTROL_ID, text, wireframeMode, 5, yPos);
      yPos += vSpacing + buttonHeight;

      radioOption = gui.doRadioButton(SMOL_CONTROL_ID, "Affect Sliders", radioOption, 6, yPos);
      yPos += vSpacing + buttonHeight;

      // Slider line thickness
      sliderLineThickness = gui.doHorizontalSlider(SMOL_CONTROL_ID, sliderLineThickness, 5, yPos, 290);
      yPos += vSpacing + buttonHeight;

      // Slider handle thickness
    sliderHandleThickness = gui.doHorizontalSlider(SMOL_CONTROL_ID, sliderHandleThickness, 5, yPos, 290);
      yPos += vSpacing + buttonHeight;

      if (radioOption)
      {
        gui.getSkin().sliderThickness = sliderLineThickness;
        gui.getSkin().sliderHandleThickness = sliderHandleThickness;
      }

    float opacity =  gui.getSkin().windowOpacity;
        opacity = gui.doHorizontalSlider(SMOL_CONTROL_ID, opacity, 5, yPos, 290);
        gui.getSkin().windowOpacity = opacity;
        yPos += vSpacing + buttonHeight;

    gui.endWindow();
    gui.end();

  }

  void Editor::terminate()
  {
  }
}
