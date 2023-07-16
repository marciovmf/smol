#include "smol_editor.h"
#include "smol_editor_icons.h"
#include <smol/smol_log.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_project_manager.h>
#include <smol/smol_config_manager.h>
#include <smol/smol_project_manager.h>
#include <smol/smol_font.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer.h>
#include <smol/smol_gui.h>
#include <smol/smol_point.h>
#include <smol/smol_event.h>
#include <smol/smol_platform.h>
#include <smol/smol_event_manager.h>
#include <string.h>

#include <string>

namespace smol
{
  std::string pipe;
  Project newProject = {};
  const int controlHeight = 30;
  Point2 windowPos = Point2{550, 150};
  Point2 windowPos2 = Point2{100, 300};
  bool radioOption = false;
  bool breakText = true;
  int32 comboValue = 0;
  uint32 aspectComboValue = -1;
  bool showSecondWindow = false;
  float skinValue[] = {0.0f, 1.0f, 1.0f, 0.2f};
  const char* aspectOptions[] = {"Free", "16:9", "21:9"};
  float aspectValues[] = {0.0f, 16/9.0f, 21/9.0f};
  Window* window;

  bool callbackForward(const Event& event, void* payload)
  {
    Editor* instance = (Editor*) payload;
    return instance->onEvent(event);
  }

  void Editor::initialize(Window* window, Project& project)
  {
    ResourceManager& resourceManager = ResourceManager::get();
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
    this->project = &project;

    smol::window = window;
    EventManager::get().addHandler(callbackForward, Event::TEXT);
  }

  void drawModalWaitMessage(GUI& gui, int screenWidth, int screenHeight, const char* title, const char* message)
  {
    int32 width = (int32) (screenWidth * 0.8f);
    int32 height = (int32) (screenHeight * 0.6f);

    gui.beginWindow(SMOL_CONTROL_ID, title, screenWidth/2 - width/2, screenHeight/2 - height/2, width, height, true);
      gui.label(SMOL_CONTROL_ID, message, 10, 15, width, GUI::Align::NONE, Color::BLACK); 
    gui.endWindow();
  }

  void drawMainMenu(GUI& gui, int width, int height)
  {
    static int32 activeMenu = -1;
    const char* projectMenuOptions[] { 
      (const char*) "New Visual Studio 2019 project", 
      (const char*) "New Visual Studio 2017 project", 
      (const char*) "New Makefile project", 
      (const char*) "New Ninja project", 
      (const char*) "Open project" };
      const int numOptions = sizeof(projectMenuOptions) / sizeof(char*);

    // Project menu
    gui.panel(SMOL_CONTROL_ID, 0, 0, width, height);
    if (gui.doLabelButton(SMOL_CONTROL_ID, "Project", 5, 0, 64, controlHeight))
    {
      activeMenu = 0;
    }

    if (activeMenu == 0)
    {
      int32 option = gui.doOptionList(SMOL_CONTROL_ID, projectMenuOptions,
          numOptions, 0, controlHeight, 120);

      Project::CMakeGenerator generator = Project::CMakeGenerator::GENERATOR_COUNT;
      if (option != -1)
      {
        activeMenu = -1;

        char projectFileName[Platform::MAX_PATH_LEN];
        bool success = Platform::showSaveFileDialog("New project", projectFileName,
            (const char*)"Smol project Files\0project.smol\0\0",
            (const char*) "project.smol");

        if (success)
        {
          generator = (Project::CMakeGenerator) option;
          const char* projectName = "Smol Game";
          if (ProjectManager::createProject(projectFileName, projectName, generator))
            ProjectManager::loadProject(projectFileName, newProject);
          activeMenu = -1;
        }
      }
      else if (option == 1)
      {
        activeMenu = -1;
        debugLogInfo("Close Project...");
      }
    }
  }

  void Editor::render(int windowWidth, int windowHeight)
  {
    Renderer::setMaterial(gui.getMaterial());
    Renderer::setViewport(0, 0, windowWidth, windowHeight);
    const int vSpacing = 5;
    int yPos;

    gui.begin(windowWidth, windowHeight);

    drawMainMenu(gui , windowWidth, controlHeight);

    if (showSecondWindow) 
    {
      const char* menuOptions[] = {"Font Size", "Line Spacing", "Window Opacity", "Slider thickness"};
      // Window 2
      windowPos2 = gui.beginWindow(SMOL_CONTROL_ID, "Another Test window", windowPos2.x, windowPos2.y, 300, 400);
      yPos = 5;

      GUISkin& skin = gui.getSkin();
      const int numOptions = sizeof(menuOptions)/sizeof(char*);
      comboValue = gui.doComboBox(SMOL_CONTROL_ID, menuOptions, numOptions, comboValue, 5, yPos, 290);
      yPos += vSpacing + controlHeight;

      float sliderValue = skinValue[comboValue];

      sliderValue = gui.doHorizontalSlider(SMOL_CONTROL_ID, sliderValue, 5, yPos, 290);
      skinValue[comboValue] = sliderValue;
      yPos += vSpacing + controlHeight;

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

      if (gui.doButton(SMOL_CONTROL_ID, "Close", 5, yPos, 290, controlHeight))
        showSecondWindow = false;
      gui.endWindow();
    }

    // Window 1
    windowPos = gui.beginWindow(SMOL_CONTROL_ID, "Test window", windowPos.x, windowPos.y, 300, 800);
    yPos = 5;

    //
    // Toggle window button
    //
    if (gui.doButton(SMOL_CONTROL_ID, "Toggle second widow", 5, yPos, 290, controlHeight))
      showSecondWindow = !showSecondWindow;
    yPos += vSpacing + controlHeight;

    //
    // Quit button
    //
    if (gui.doButton(SMOL_CONTROL_ID, "Quit", 5, yPos, 290, controlHeight))
    {
      Event evt;
      evt.type = Event::GAME;
      evt.gameEvent.id = 0xFFFFFFFF;
      EventManager::get().pushEvent(evt);
    }
    yPos += vSpacing + controlHeight;


    //
    // Fixed aspect ratio
    //

    // Yes, it's a hack. 
    GlobalDisplayConfig& displayConfig = (GlobalDisplayConfig&) ConfigManager::get().displayConfig();
    if (aspectComboValue == -1)
    {
      for (int i = 0; i < sizeof(aspectValues); i++)
      {
        if (displayConfig.aspectRatio == aspectValues[0])
        {
          aspectComboValue = i;
          break;
        }
      }

      if (aspectComboValue == -1)
        aspectComboValue = 0;
    }

    int32 prevAspectComboValue = aspectComboValue;
    aspectComboValue = gui.doComboBox(SMOL_CONTROL_ID, aspectOptions, sizeof(aspectOptions)/sizeof(char*), aspectComboValue, 5, yPos, 290);

    if(prevAspectComboValue != aspectComboValue)
    {
      displayConfig.aspectRatio = aspectValues[aspectComboValue];
      debugLogInfo("Changing aspect to %f", displayConfig.aspectRatio);
      Event evt;
      evt.type = Event::DISPLAY;
      evt.displayEvent.type = DisplayEvent::RESIZED;
      evt.displayEvent.width = displayConfig.width;
      evt.displayEvent.height = displayConfig.height;
      EventManager::get().pushEvent(evt);
    }
    yPos += vSpacing + controlHeight;

    //
    // Fullscreen 
    //
    bool isFullScreen = Platform::isFullScreen(window);
    bool wasFullScreen = isFullScreen;
    isFullScreen = gui.doCheckBox(SMOL_CONTROL_ID, "FullScreen", isFullScreen, 5, yPos);
    if(wasFullScreen != isFullScreen)
    {
      Platform::setFullScreen(window, isFullScreen);
    }
    yPos += vSpacing + controlHeight;

    //
    // Break text
    //
    breakText = gui.doCheckBox(SMOL_CONTROL_ID, "Break Text", breakText, 5, yPos);
    yPos += vSpacing + controlHeight;

    //
    // Long sample text
    //

    gui.label(SMOL_CONTROL_ID, "Officially recognised by the Duden - Germany's pre-eminent dictionary - as the longest word in German, Kraftfahrzeug-Haftpflichtversicherung is a 36-letter, tongue-tying way of describing a rather, mundane everyday concept: motor vehicle liability insurance", 5, yPos, breakText ? 290 : 0);
    gui.endWindow();

    bool isBusy = false;
    const char* msg;
    const char* errorMsg;
    if (newProject.state == Project::GENERATING)
    {
      isBusy = true;
      msg = (const char*) "Generating project build solution...";
      errorMsg = (const char*) "Failed to generating Project build solution.";
    }

    if (newProject.state == Project::BUILDING)
    {
      isBusy = true;
      msg = (const char*) "Generating project build solution...";
      errorMsg = (const char*) "Failed to build project.";
    }

    if (newProject.state == Project::GENERATED)
    {
      isBusy = true;
      msg = (const char*) "Building project ...";
      errorMsg = (const char*) "Failed to build project.";
      ProjectManager::buildProjectModule(newProject);
    }

    if (isBusy)
    {
      int32 exitCode;
      pipe += newProject.cmdOutputBuffer;
      drawModalWaitMessage(gui, windowWidth, windowHeight, msg, pipe.c_str());
      if (ProjectManager::waitForExternalCommand(newProject, &exitCode))
      {
        pipe = "";
        if (exitCode != 0)
          Platform::messageBoxError("Project error", errorMsg);
      }
    }

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
