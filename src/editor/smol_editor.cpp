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
#include <smol/smol_game.h>
#include <smol/smol_point.h>
#include <smol/smol_event.h>
#include <smol/smol_platform.h>
#include <smol/smol_event_manager.h>
#include <string.h>
#include <string>
#include <xerrc.h>

void dummyOnGameStartCallback() {}
void dummyOnGameStopCallback() {}
void dummyOnGameUpdateCallback(float) {}
void dummyOnGameGUICallback(smol::GUI&) {}

namespace smol
{
  std::string pipe;
  const int controlHeight = GUI::DEFAULT_CONTROL_HEIGHT;
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
    gui.getSkin().lineHeightAdjust = -0.05f;

    GUISkin& skin = gui.getSkin();
    skin.spriteCheckBox = iconCHECKBOX();
    skin.spriteCheckBoxChecked = iconCHECKBOX_CHECKED();
    skin.spriteRadioButton = iconRADIO();
    skin.spriteRadioButtonChecked = iconRADIO_CHECKED();
    skin.spriteSliderHandle = iconSLIDER_HANDLE();
    skin.spriteComboBoxChevron = iconCHEVRON_DOWN();
    this->project = &project;
    this->reopenProjectFilePath[0] = 0;
    this->closeFlag = false;
    this->mode = Mode::MODE_EDIT;

    smol::window = window;
    EventManager::get().addHandler(callbackForward, Event::TEXT | Event::KEYBOARD, this);
  }

  void drawModalWaitMessage(GUI& gui, int screenWidth, int screenHeight, const char* title, const char* message)
  {
    int32 width = (int32) (screenWidth * 0.8f);
    int32 height = (int32) (screenHeight * 0.6f);

    gui.beginWindow(SMOL_CONTROL_ID, title, screenWidth/2 - width/2, screenHeight/2 - height/2, width, height, true);
    gui.label(SMOL_CONTROL_ID, message, 10, 15, width, GUI::Align::NONE, Color::BLACK); 
    gui.endWindow();
  }

  void Editor::drawMainMenu(int width, int height)
  {
    static int32 activeMenu = -1;
    const char* projectMenuOptions[] { 
      (const char*) "New Visual Studio 2019 project", 
        (const char*) "New Visual Studio 2017 project", 
        (const char*) "New Makefile project", 
        (const char*) "New Ninja project", 
        (const char*) "",
        (const char*) "Open project" };
    const int numOptions = sizeof(projectMenuOptions) / sizeof(char*);
    const int32 openProjectOption = 4;

    // Project menu
    gui.panel(SMOL_CONTROL_ID, 0, 0, width, height);


    //if (gui.doLabelButton(projectControlId, "Project", 5, 0, 64, controlHeight))
    if (gui.doLabelButton(SMOL_CONTROL_ID, "Project", 5, 0, 64, controlHeight))
    {
      activeMenu = 0;
    }

    if (activeMenu == 0)
    {
      int32 option = gui.doOptionList(SMOL_CONTROL_ID, projectMenuOptions, numOptions, 0, controlHeight, 120);
      Project::CMakeGenerator generator = Project::CMakeGenerator::GENERATOR_COUNT;
      if (option == GUI::POPUP_MENU_DMISMISS)
        activeMenu = -1;
      else if (option != GUI::POPUP_MENU_IDLE)
      {
        activeMenu = -1;
        if (option == openProjectOption)
        {
          bool success = Platform::showOpenFileDialog("Open project", reopenProjectFilePath,
              (const char*)"Smol project Files\0project.smol\0\0",
              (const char*) "project.smol");
          if (success)
          {
            closeFlag = true;
          }
        }
        else
        {
          bool success = Platform::showSaveFileDialog("New project", reopenProjectFilePath,
              (const char*)"Smol project Files\0project.smol\0\0",
              (const char*) "project.smol");
          if (success)
          {
            generator = (Project::CMakeGenerator) option;
            const char* projectName = "Smol Game";
            if (ProjectManager::createProject(reopenProjectFilePath, projectName, generator))
            {
              closeFlag = true;
            }
          }
        }

      }
    }
  }

  void Editor::update(int windowWidth, int windowHeight)
  {
    //
    // Should game.onStart be called ?
    //
    bool shouldAlwaysBuild = ConfigManager::get().editorConfig().alwaysRebuildBeforeRun;
    if (mode == Mode::MODE_PRERUN)
    {
      if((project->state == Project::READY || (project->state == Project::GENERATED && !shouldAlwaysBuild)))
      {
        mode = Mode::MODE_RUNNING;
        loadGameModule("game.dll");
        gameModule.onStart();
        return;
      }
    }

    Renderer::setMaterial(gui.getMaterial());
    Renderer::setViewport(0, 0, windowWidth, windowHeight);
    const int vSpacing = 5;
    int yPos;

    gui.begin(windowWidth, windowHeight);

    if (mode == MODE_EDIT)
    {
      drawMainMenu(windowWidth, controlHeight);
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
      // Run button
      //

      gui.enabled = (project->state == Project::GENERATED || project->state == Project::READY);
      if (gui.doButton(SMOL_CONTROL_ID, "Play", 5, yPos, 290, controlHeight))
      {
        toggleMode();
      }

      gui.enabled = true;
      yPos += vSpacing + controlHeight;

      //
      // Quit button
      //
      if (gui.doButton(SMOL_CONTROL_ID, "Quit", 5, yPos, 290, controlHeight))
      {
        closeFlag = true;
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
      // Compile always 
      //
      GlobalEditorConfig& editorConfig = (GlobalEditorConfig&) ConfigManager::get().editorConfig();
      editorConfig.alwaysRebuildBeforeRun = gui.doCheckBox(SMOL_CONTROL_ID, "Always compile on run", editorConfig.alwaysRebuildBeforeRun, 5, yPos);
      yPos += vSpacing + controlHeight;


      //
      // Crop area Color
      //
      if (gui.doButton(SMOL_CONTROL_ID, "Bordee area color", 5, yPos, 290, controlHeight))
      {
        displayConfig .cropAreaColor = Platform::showColorPickerDialog();
      }
      yPos += vSpacing + controlHeight;

      gui.endWindow();

    }

    //
    // Shows a blocking popup if we are building or generating the solution
    //
    bool isBusy = false;
    const char* msg;
    const char* errorMsg;

    if (project->state == Project::CREATED)
    {
      ProjectManager::generateProject(*project);
    }

    if (project->state == Project::GENERATING)
    {
      isBusy = true;
      msg = (const char*) "Generating solution...";
      errorMsg = (const char*) "Failed to generating Project build solution.";
    }
    else if (project->state == Project::BUILDING)
    {
      isBusy = true;
      msg = (const char*) "Building ...";
      errorMsg = (const char*) "Failed to build project.";
    }

    if (isBusy)
    {
      int32 exitCode;
      pipe += project->cmdOutputBuffer;
      drawModalWaitMessage(gui, windowWidth, windowHeight, msg, pipe.c_str());
      if (ProjectManager::waitForExternalCommand(*project, &exitCode))
      {
        pipe = "";
        if (exitCode != 0)
          Platform::messageBoxError("Project error", errorMsg);
      }
    }

    gui.end();
  }

  void Editor::toggleMode()
  {
    if (mode == Mode::MODE_EDIT)
    {
      // (Re)build the game module before running it
      if (project->state == Project::GENERATED || project->state == Project::READY)
      {
        debugLogInfo("Editor mode change: RUN"); 
        mode = Mode::MODE_PRERUN;
        if (!Platform::fileExists("game.dll") || ConfigManager::get().editorConfig().alwaysRebuildBeforeRun)
        {
          ProjectManager::buildProjectModule(*project);
        }
      }
    }
    else if (mode == Mode::MODE_RUNNING)
    {
      mode = Mode::MODE_EDIT;
      gameModule.onStop();
      SceneManager::get().cleanupScene();
      unloadGameModule();
      debugLogInfo("Editor mode change: EDIT"); 
    }
  }

  bool Editor::onEvent(const Event& event)
  {
    if (event.type == Event::KEYBOARD)
    {
      Keycode keyStartStop = ConfigManager::get().editorConfig().keyStartStop;
      if (event.keyboardEvent.type == KeyboardEvent::KEY_DOWN && event.keyboardEvent.keyCode == keyStartStop)
      {
        toggleMode();
        return true;
      }
      return false;
    }
    //debugLogInfo("TextEvent %c", (char)event.textEvent.character);
    //return true;
    return false;
  }

  bool Editor::unloadGameModule()
  {
    return Platform::unloadModule(gameModule.module);
  }

  bool Editor::loadGameModule(const char* modulePath)
  {
    Module* module = Platform::loadModule(modulePath);
    if (module)
    {
      SMOL_GAME_CALLBACK_ONSTART onGameStartCallback = (SMOL_GAME_CALLBACK_ONSTART)
        Platform::getFunctionFromModule(module, SMOL_CALLBACK_NAME_ONSTART);

      SMOL_GAME_CALLBACK_ONSTOP onGameStopCallback = (SMOL_GAME_CALLBACK_ONSTOP)
        Platform::getFunctionFromModule(module, SMOL_CALLBACK_NAME_ONSTOP);

      SMOL_GAME_CALLBACK_ONUPDATE onGameUpdateCallback = (SMOL_GAME_CALLBACK_ONUPDATE)
        Platform::getFunctionFromModule(module, SMOL_CALLBACK_NAME_ONUPDATE);

      // This is an optional callback
      SMOL_GAME_CALLBACK_ONGUI onGameGUICallback = (SMOL_GAME_CALLBACK_ONGUI)
        Platform::getFunctionFromModule(module, SMOL_CALLBACK_NAME_ONGUI);

      bool success = true;
      if (!onGameStartCallback)
      {
        success = false;
        smol::Log::error("Game module doesn't have a '%s' callback", SMOL_CALLBACK_NAME_ONSTART);
      }

      if (!onGameUpdateCallback)
      {
        success = false;
        smol::Log::error("Game module doesn't have a '%s' callback", SMOL_CALLBACK_NAME_ONUPDATE);
      }

      if (!onGameStopCallback)
      {
        success = false;
        smol::Log::error("Game module doesn't have a '%s' callback", SMOL_CALLBACK_NAME_ONSTOP);
      }

      if (success)
      {
        gameModule.module    = module;
        gameModule.onStart   = onGameStartCallback;
        gameModule.onStop    = onGameStopCallback;
        gameModule.onUpdate  = onGameUpdateCallback;
        gameModule.onGUI     = onGameGUICallback ? onGameGUICallback : dummyOnGameGUICallback;
        return true;
      }
    }

    gameModule.module    = nullptr;
    gameModule.onStart   = dummyOnGameStartCallback;
    gameModule.onStop    = dummyOnGameStopCallback;
    gameModule.onUpdate  = dummyOnGameUpdateCallback;
    gameModule.onGUI     = dummyOnGameGUICallback;
    return false;
  }

  void Editor::updateGame(float delta)
  {
    if (mode == Mode::MODE_RUNNING)
    {
      gameModule.onUpdate(delta);
    }
    else
    {
      Renderer::setClearColor(Color::GRAY);
      Renderer::clearBuffers((Renderer::CLEAR_COLOR_BUFFER | Renderer::CLEAR_DEPTH_BUFFER));
    }
  }

  const char* Editor::shouldReopenWithProject()
  {
    if (reopenProjectFilePath[0])
      return reopenProjectFilePath;

    return nullptr;
  }

  bool Editor::getCloseFlag()
  {
    return closeFlag;
  }

  void Editor::terminate()
  {
  }
}
