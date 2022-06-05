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
#include <smol/smol_vector2.h>
#include <smol/smol_hashmap.h>

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

    enum Colour
    {
      RED = 10,
      GREEN = 55,
      BLUE = 94,
      FOO = 17
    };

    //uint64 colourToHash(Colour c)
    //{
    //  return smol::intToHash((int) c);
    //}

    int smolMain(int argc, char** argv)
    {
      smol::Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // Test hashmap ---------------------------------------------------------
      {
#if 1
        {
          smol::Hashmap<const char*, Vector3> map;
          map.add("red", Vector3{1.0f, 0.0f, 0.0f});
          map.add("green", Vector3{0.0f, 1.0f, 0.0f});
          map.add("blue", Vector3{0.0f, 0.0f, 1.0f});

          Vector3& green = map.get("green");
          Log::info("GREEN is (%f, %f, %f)", green.x, green.y, green.z);

          Vector3& red = map.get("red");
          Log::info("RED is (%f, %f, %f)", red.x, red.y, red.z);

          Vector3& blue = map.get("blue");
          Log::info("BLUE is (%f, %f, %f)", blue.x, blue.y, blue.z);
        }
#endif
        {
          smol::Hashmap<Colour, Vector3, int, uint64 (*)(Colour)> map2(64, 0, smol::typeToHash<Colour>);
          map2.add(Colour::RED,   Vector3{1.0f, 0.0f, 0.0f});
          map2.add(Colour::GREEN, Vector3{0.0f, 1.0f, 0.0f});
          map2.add(Colour::BLUE,  Vector3{0.0f, 0.0f, 1.0f});

          if (map2.hasKey(Colour::GREEN))
          {
            Vector3& green = map2.get(Colour::GREEN);
            Log::info("GREEN is (%f, %f, %f)", green.x, green.y, green.z);
          }

          if (map2.hasKey(Colour::RED))
          {
            Vector3& red = map2.get(Colour::RED);
            Log::info("RED is (%f, %f, %f)", red.x, red.y, red.z);
          }

          if (map2.hasKey(Colour::BLUE))
          {
            Vector3& blue = map2.get(Colour::BLUE);
            Log::info("BLUE is (%f, %f, %f)", blue.x, blue.y, blue.z);
          }

          if (map2.hasKey(Colour::FOO))
          {
            Vector3& foo = map2.get(Colour::FOO);
            Log::info("FOO is (%f, %f, %f)", foo.x, foo.y, foo.z);
          }

        }

        return 0;
      }
      // end test hashmap ----------------------------------------------------


      // parse variables file
      SystemVariables systemVariables;
      WindowVariables windowVariables;
      Config config(SMOL_VARIABLES_FILE);
      ConfigEntry* entry = config.entries;

      for (int i = 0; i < config.entryCount; i++)
      {
        if (strncmp(entry->variables[0].name ,"window", 6) == 0)
        {
          // Window variables
          windowVariables.size = entry->getVariableVec2("size");
          windowVariables.caption = entry->getVariableString("caption");
        }

        if (strncmp(entry->variables[0].name ,"system", 6) == 0)
        {
          // system variables
          systemVariables.showCursor = entry->getVariableNumber("showCursor") > 0.0f;
          systemVariables.captureCursor = entry->getVariableNumber("captureCursor") > 0.0f;
          const Vector2 defaultGlVersion = Vector2{3.0f, 0.0f};
          Vector2 glVersion = entry->getVariableVec2("glVersion", defaultGlVersion);
          systemVariables.glVersionMajor = (int) glVersion.x;
          systemVariables.glVersionMinor = (int) glVersion.y;
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

      Platform::showCursor(systemVariables.showCursor);

      if (systemVariables.captureCursor)
        Platform::captureCursor(window);

      // Initialize systems root
      smol::SystemsRoot root;
      root.config = &config;

      root.keyboard = &smol::Keyboard();
      root.mouse = &smol::Mouse();
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
        root.mouse->update();
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

