#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_color.h>
#include <smol/smol_vector2.h>
#include <smol/smol_gui.h>
#include <string.h>

#if defined(SMOL_DEBUG)
#define SMOL_LOGFILE nullptr
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_ALL
#else
#define SMOL_LOGFILE "smol_engine_log.txt"
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_FATAL |  smol::Log::LOG_ERROR
#endif

#ifndef SMOL_GAME_MODULE_NAME
#ifdef SMOL_PLATFORM_WINDOWS
#define SMOL_GAME_MODULE_NAME "game.dll"
#else
#define SMOL_GAME_MODULE_NAME "game.so"
#endif
#endif

#define SMOL_VARIABLES_FILE ((const char*) "assets/variables.txt")

namespace smol
{
  namespace launcher
  {
    bool toggleValue = false;
    Point2 windowPos = Point2{550, 150};
    bool wireframeMode = false;
    float frameTime = 0.0f;
    float fps = 0;

    void onEditorGUI(GUI& gui)
    {
      const Point2 mousePos = SystemsRoot::get()->mouse.getCursorPosition();
      char text[128];

      const int buttonHeight = 30;
      const int vSpacing = 5;
      int yPos = 5;

      windowPos = gui.beginWindow(SMOL_CONTROL_ID, "Test window", windowPos.x, windowPos.y, 300, 350);
        if (gui.doButton(SMOL_CONTROL_ID, "Button 1", 5, yPos, 290, buttonHeight))
          debugLogInfo("Button 1 clicked!");
        yPos += vSpacing + buttonHeight;

        snprintf(text, 128, "Wireframe mdoe: '%s'", wireframeMode ? "On":"Off");
        wireframeMode = gui.doToggleButton(SMOL_CONTROL_ID, text, wireframeMode, 5, yPos, 290, 30);
        yPos += vSpacing + buttonHeight + 15;

        gui.label(SMOL_CONTROL_ID, "Statistics and other info", 145, yPos, GUI::Align::CENTER);
        yPos += vSpacing + buttonHeight + 15;

        gui.verticalSeparator(150, yPos, 130);
        yPos += vSpacing + buttonHeight + 5;
        
        gui.label(SMOL_CONTROL_ID, "Cursor", 145, yPos, GUI::Align::RIGHT);
        snprintf(text, sizeof(text), "%d,%d", mousePos.x, mousePos.y);
        gui.label(SMOL_CONTROL_ID, text, 155, yPos, GUI::Align::LEFT);
        yPos += vSpacing + buttonHeight;

        gui.label(SMOL_CONTROL_ID, "Delta Time", 145, yPos, GUI::Align::RIGHT);
        snprintf(text, sizeof(text), "%f ms", frameTime);
        gui.label(SMOL_CONTROL_ID, text, 155, yPos, GUI::Align::LEFT);
        yPos += vSpacing + buttonHeight;

        gui.label(SMOL_CONTROL_ID, "FPS", 145, yPos, GUI::Align::RIGHT);
        snprintf(text, sizeof(text), "%f", fps);
        gui.label(SMOL_CONTROL_ID, text, 155, yPos, GUI::Align::LEFT);
        yPos += vSpacing + buttonHeight;
      gui.endWindow();
    }

    int smolMain(int argc, char** argv)
    {
      Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // parse variables file
      Config config(SMOL_VARIABLES_FILE);
      GlobalSystemConfig systemConfig(config);
      GlobalDisplayConfig displayConfig(config);

      if (!Platform::initOpenGL(systemConfig.glVersionMajor, systemConfig.glVersionMinor))
        return 1;

      // Load game module
      Module* game = Platform::loadModule(SMOL_GAME_MODULE_NAME);
      SMOL_GAME_CALLBACK_ONSTART onGameStartCallback = (SMOL_GAME_CALLBACK_ONSTART)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTART);

      SMOL_GAME_CALLBACK_ONSTOP onGameStopCallback = (SMOL_GAME_CALLBACK_ONSTOP)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTOP);

      SMOL_GAME_CALLBACK_ONUPDATE onGameUpdateCallback = (SMOL_GAME_CALLBACK_ONUPDATE)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONUPDATE);

      SMOL_GAME_CALLBACK_ONGUI onGameGUICallback = (SMOL_GAME_CALLBACK_ONGUI)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONGUI);

      if (! (game && onGameStartCallback && onGameStopCallback && onGameUpdateCallback))
      {
        Log::error("Failed to load a valid game module.");
        return 1;
      }

      // initialie display and system stuff
      Window* window = Platform::createWindow(displayConfig.width, displayConfig.height, displayConfig.caption);

      if(displayConfig.fullScreen)
      {
        debugLogInfo("Going fullscreen");
        Platform::setFullScreen(window, true);
      }

      Platform::showCursor(systemConfig.showCursor);

      if (systemConfig.captureCursor)
        Platform::captureCursor(window);

      Platform::getWindowSize(window, &displayConfig.width, &displayConfig.height);
      // Initialize systems root
      SystemsRoot::initialize(config);
      Mouse& mouse        = SystemsRoot::get()->mouse;
      Keyboard& keyboard  = SystemsRoot::get()->keyboard;
      Renderer& renderer  = SystemsRoot::get()->renderer;

      renderer.resize(displayConfig.width, displayConfig.height);

      // UI stuff...
      // Loads the UI font and material
      ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
      Handle<Font> uiFont = resourceManager.loadFont("assets/font/segoeui.font");
      Handle<Material> uiMaterial = resourceManager.loadMaterial("assets/ui.material");
      uiMaterial->setSampler2D("mainTex", uiFont->getTexture());
      StreamBuffer buffer;
      Renderer::createStreamBuffer(&buffer);

      // Run game/engine
      uint64 startTime = 0;
      uint64 endTime = 0;
      onGameStartCallback();

      GUI gui(uiMaterial, uiFont);

      int numFrames = 0;
      float frameTimeAccumulated = 0.0f;

      while(! Platform::getWindowCloseFlag(window))
      {
        float deltaTime = Platform::getMillisecondsBetweenTicks(startTime, endTime);
        startTime = Platform::getTicks();

        mouse.update();
        keyboard.update();
        onGameUpdateCallback(deltaTime);
        Platform::updateWindowEvents(window);

        // check for resize.
        //TODO(marcio): Make an event system so we get notified when this happens.
        int windowWidth, windowHeight;
        Platform::getWindowSize(window, &windowWidth, &windowHeight);
        if (windowWidth != displayConfig.width || windowHeight != displayConfig.height)
        {
          displayConfig.width = windowWidth;
          displayConfig.height = windowHeight;
          renderer.resize(windowWidth, windowHeight);
        }

        // render scene
        Renderer::setRenderMode(wireframeMode ? Renderer::RenderMode::WIREFRAME : Renderer::RenderMode::SHADED);
        renderer.render(deltaTime);
        Renderer::setRenderMode(Renderer::RenderMode::SHADED);

        // GUI
        //gui.getMaterial()->setVec2("screenSize", Vector2((float)windowWidth, (float) windowHeight));
        Renderer::setMaterial(gui.getMaterial());

        if (onGameGUICallback)
        {
          gui.begin(windowWidth, windowHeight);
          onGameGUICallback(gui);
          gui.end();
        }
        endTime = Platform::getTicks();
        
        numFrames++;
        Renderer::setViewport(0, 0, windowWidth, windowHeight);
        gui.begin(windowWidth, windowHeight);
        frameTime = deltaTime;
        fps = numFrames / frameTimeAccumulated;
        onEditorGUI(gui);
        gui.end();

        frameTimeAccumulated += deltaTime;
        if (frameTimeAccumulated > 1.0f)
        {
          numFrames = 0;
          frameTimeAccumulated = 1.0f - frameTimeAccumulated;
        }
      }

      onGameStopCallback();
      Platform::unloadModule(game);
      Platform::destroyWindow(window);
      SystemsRoot::terminate();
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  //TODO(marcio): handle command line here when we support any
  return smol::launcher::smolMain(0, (char**) lpCmdLine);
}
#endif  // SMOL_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
  return smol::launcher::smolMain(argc, argv);
}

