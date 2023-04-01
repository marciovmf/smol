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
    struct WindowVariables
    {
      Vector2 size;
      const char* caption;
    }; 

    struct SystemVariables
    {
      bool showCursor;
      bool captureCursor;
      int glVersionMajor;
      int glVersionMinor;
    };

    int smolMain(int argc, char** argv)
    {
      Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // parse variables file
      SystemVariables systemVariables;
      WindowVariables windowVariables;
      Config config(SMOL_VARIABLES_FILE);
      ConfigEntry* entry;
      entry = config.findEntry("window");
      if (entry)
      {
        windowVariables.size = entry->getVariableVec2("size");
        windowVariables.caption = entry->getVariableString("caption");
      }

      entry = config.findEntry("system");
      if(entry)
      {
        // system variables
        systemVariables.showCursor = entry->getVariableNumber("showCursor") > 0.0f;
        systemVariables.captureCursor = entry->getVariableNumber("captureCursor") > 0.0f;
        const Vector2 defaultGlVersion = Vector2{3.0f, 0.0f};
        Vector2 glVersion = entry->getVariableVec2("glVersion", defaultGlVersion);
        systemVariables.glVersionMajor = (int) glVersion.x;
        systemVariables.glVersionMinor = (int) glVersion.y;
      }

      if (!Platform::initOpenGL(systemVariables.glVersionMajor, systemVariables.glVersionMinor))
        return 1;

      Module* game = Platform::loadModule(SMOL_GAME_MODULE_NAME);
      SMOL_GAME_CALLBACK_ONSTART onGameStartCallback = (SMOL_GAME_CALLBACK_ONSTART)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTART);

      SMOL_GAME_CALLBACK_ONSTOP onGameStopCallback = (SMOL_GAME_CALLBACK_ONSTOP)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTOP);

      SMOL_GAME_CALLBACK_ONUPDATE onGameUpdateCallback = (SMOL_GAME_CALLBACK_ONUPDATE)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONUPDATE);

      if (! (game && onGameStartCallback && onGameStopCallback && onGameUpdateCallback))
      {
        Log::error("Failed to load a valid game module.");
        return 1;
      }

      Window* window = Platform::createWindow((int) windowVariables.size.x,
          (int) windowVariables.size.y, windowVariables.caption);

      Platform::showCursor(systemVariables.showCursor);

      if (systemVariables.captureCursor)
        Platform::captureCursor(window);

      int lastWidth, lastHeight;
      Platform::getWindowSize(window, &lastWidth, &lastHeight);
      ResourceManager resourceManager;
      Scene scene(resourceManager);
      Renderer renderer(scene, lastWidth, lastHeight);

      Keyboard keyboardSystem;
      Mouse mouseSystem;

      // Initialize systems root

      SystemsRoot::initialize(config,
          renderer,
          keyboardSystem,
          mouseSystem, 
          resourceManager,
          scene);

      onGameStartCallback();

      uint64 startTime = 0;
      uint64 endTime = 0;

      while(! Platform::getWindowCloseFlag(window))
      {
        float deltaTime = Platform::getMillisecondsBetweenTicks(startTime, endTime);
        startTime = Platform::getTicks();

        mouseSystem.update();
        keyboardSystem.update();
        onGameUpdateCallback(deltaTime);
        Platform::updateWindowEvents(window);

        // check for resize.
        //TODO(marcio): Make an event system so we get notified when this
        //happens.
        int windowWidth, windowHeight;
        Platform::getWindowSize(window, &windowWidth, &windowHeight);
        if (windowWidth != lastWidth || windowHeight != lastHeight)
        {
          lastWidth = windowWidth;
          lastHeight = windowHeight;
          renderer.resize(windowWidth, windowHeight);
        }

        renderer.render();
        endTime = Platform::getTicks();
      }

      onGameStopCallback();
      Platform::unloadModule(game);
      Platform::destroyWindow(window);
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

