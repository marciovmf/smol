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

namespace smol
{
  namespace launcher
  {
    int smolMain(int argc, char** argv)
    {
      smol::Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      if (!Platform::initOpenGL(3, 2))
        return 1;

#define TEST_CFG_PARSING 1
#if TEST_CFG_PARSING
      {
      Config config("assets/test.txt");
      debugLogInfo("Num Entries = %d", config.entryCount);

      ConfigEntry* entry = config.entries;
      for (int i = 0; i < config.entryCount; i++)
      {
        debugLogInfo("Entry #%d: %d variables.", i, entry->variableCount);

        for (int varIndex = 0; varIndex < entry->variableCount; varIndex++)
        {
          ConfigVariable* variable = &entry->variables[varIndex];

          if (variable->type == ConfigVariable::NUMBER)
            debugLogInfo("\t%s = %f", variable->name, variable->numberValue);
          else if (variable->type == ConfigVariable::STRING)
            debugLogInfo("\t%s = %s", variable->name, variable->stringValue);
          else if (variable->type == ConfigVariable::VECTOR2)
            debugLogInfo("\t%s = %f, %f", variable->name,
                variable->vec2Value[0],
                variable->vec2Value[1]);
          else if (variable->type == ConfigVariable::VECTOR3)
            debugLogInfo("\t%s = %f, %f, %f", variable->name,
                variable->vec3Value[0],
                variable->vec3Value[1],
                variable->vec3Value[2]);
          else if (variable->type == ConfigVariable::VECTOR4)
            debugLogInfo("\t%s = %f, %f, %f, %f", variable->name,
                variable->vec4Value[0],
                variable->vec4Value[1],
                variable->vec4Value[2],
                variable->vec4Value[3]);
        }


        entry = entry->next;
      }

      debugLogInfo("done");

      }
#endif

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

      const int WIDTH = 1080;
      const int HEIGHT = 768;
      smol::Window* window = Platform::createWindow(WIDTH, HEIGHT, (const char*)"Smol Engine");

      // Initialize systems root
      smol::SystemsRoot root;
      root.keyboard = &smol::Keyboard();
      smol::Scene scene;
      root.loadedScene = &scene;

      onGameStartCallback(&root);

      int lastWidth, lastHeight;
      Platform::getWindowSize(window, &lastWidth, &lastHeight);

      smol::Renderer renderer(*root.loadedScene, lastWidth, lastHeight);
      root.renderer = &renderer;

      while(! Platform::getWindowCloseFlag(window))
      {
        bool update = false;
        root.keyboard->update();
        onGameUpdateCallback(0.0f); //TODO(marcio): calculate delta time!
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

