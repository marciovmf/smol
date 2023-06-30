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
#include "smol_editor.h"

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
#include <smol/smol_event_manager.h>
#include <smol/smol_event.h>

namespace smol
{
  namespace launcher
  {
    bool onEvent(const Event& event, void* payload)
    {
      if (event.type == Event::DISPLAY)
      {
        GlobalDisplayConfig* cfg = (GlobalDisplayConfig*) payload;
        cfg->width = event.displayEvent.width;
        cfg->height = event.displayEvent.height;
        Renderer::setViewport(0, 0, cfg->width, cfg->height);
      }

      return false; // let other handlers know about this
    }

    int smolMain(int argc, char** argv)
    {
      Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // Change working directory to the binary location
      Platform::setWorkingDirectory(Platform::getBinaryPath());

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
      SceneManager& sceneManager = SystemsRoot::get()->sceneManager;
      EventManager& eventManager = EventManager::get();

      Renderer::setViewport(0, 0,  displayConfig.width, displayConfig.height);
      eventManager.addHandler(onEvent, Event::DISPLAY, &displayConfig);

      Editor editor;
      editor.initialize();

      uint64 startTime = 0;
      uint64 endTime = 0;
      onGameStartCallback();

      while(! Platform::getWindowCloseFlag(window))
      {
        float deltaTime = Platform::getMillisecondsBetweenTicks(startTime, endTime);
        startTime = Platform::getTicks();
        Platform::updateWindowEvents(window);
        mouse.update();
        keyboard.update();
        eventManager.dispatchEvents();
        onGameUpdateCallback(deltaTime);
        sceneManager.render(deltaTime);
        editor.render(displayConfig.width, displayConfig.height);
        Platform::swapBuffers(window);
        endTime = Platform::getTicks();
      }

      onGameStopCallback();
      editor.terminate();
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

