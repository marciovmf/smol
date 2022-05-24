#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_color.h>

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

    int smolMain(int argc, char** argv)
    {
      smol::Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // parse variables file
      WindowVariables windowVariables;
      Config config(SMOL_VARIABLES_FILE);
      ConfigEntry* entry = config.entries;

      for (int i = 0; i < config.entryCount; i++)
      {
        if (strncmp(entry->variables[0].name ,"window", strlen("window")) == 0)
        {
          // Window variables
          windowVariables.size = entry->getVariableVec2("size");
          windowVariables.caption = entry->getVariableString("caption");
        }

        entry = entry->next;
      }


      if (!Platform::initOpenGL(3, 2))
        return 1;

      smol::Module* game = Platform::loadModule(SMOL_GAME_MODULE_NAME);
      SMOL_GAME_CALLBACK_ONSTART onGameStartCallback = (SMOL_GAME_CALLBACK_ONSTART)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTART);

      SMOL_GAME_CALLBACK_ONSTOP onGameStopCallback = (SMOL_GAME_CALLBACK_ONSTOP)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTOP);

      SMOL_GAME_CALLBACK_ONUPDATE onGameUpdateCallback = (SMOL_GAME_CALLBACK_ONUPDATE)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONUPDATE);

      if (! (game && onGameStartCallback && onGameStopCallback && onGameUpdateCallback))
      {
        smol::Log::error("Failed to load a valid game module.");
        return 1;
      }

      smol::Window* window = Platform::createWindow((int) windowVariables.size.x,
          (int) windowVariables.size.y, windowVariables.caption);

      // Initialize systems root
      smol::SystemsRoot root;
      root.config = &config;

      root.keyboard = &smol::Keyboard();
      smol::Scene scene;
      root.loadedScene = &scene;

      onGameStartCallback(&root);

      int lastWidth, lastHeight;
      Platform::getWindowSize(window, &lastWidth, &lastHeight);

      smol::Renderer renderer(*root.loadedScene, lastWidth, lastHeight);
      root.renderer = &renderer;

      uint64 startTime = 0;
      uint64 endTime = 0;

      while(! Platform::getWindowCloseFlag(window))
      {
        float deltaTime = Platform::getMillisecondsBetweenTicks(startTime, endTime);
        startTime = Platform::getTicks();

        bool update = false;
        root.keyboard->update();
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

