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

    GlyphDrawData drawData[255];

    void drawEditorUI(StreamBuffer& buffer, Handle<Material> material, Handle<Font> uiFont, float screenW, float screenH)
    {
      Color color = Color::BLACK;
      color.a = 0.6f;

      Renderer::setMaterial(material);
      Renderer::begin(buffer);

      // fixed width;
      float marginX = 30 / screenW;
      float marginY = 30 / screenH;
      float w = 800 / screenW;
      float h = 600 / screenH;

        Renderer::pushSprite(buffer,
            Vector3(marginX, marginY, 0.0f), 
            Vector2(w, h),
            Rectf(), Color::BLACK, Color::BLACK, color, color);

#if 1
        // draw text
        float fontScale = 16;
        const float scaleX = fontScale / screenW;
        const float scaleY = fontScale / screenH;
        const char* text = "Hello, Sailor!\nThis is another line of text.\nAnd this is yet another line.\n.";
        const size_t textLen = strlen(text);

        uiFont->computeString(text, Color::WHITE, drawData, 1.0f);
        for (int i = 0; i < textLen; i++)
        {
          GlyphDrawData& data = drawData[i];
          Vector3 offset = Vector3(marginX + data.position.x * scaleX, marginY + data.position.y *  scaleY, 0.0f);
          Vector2 size = data.size;
          size.x *=  scaleX;
          size.y *=  scaleY;
          //offset.sum();

          Renderer::pushSprite(buffer, offset, size, data.uv, data.color);
        }
#endif

        Renderer::end(buffer);
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
      Camera uiCamera;
      uiCamera.setOrthographic(100.0f, -10.0f, 10.0f);

      // Run game/engine
      uint64 startTime = 0;
      uint64 endTime = 0;
      onGameStartCallback();

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

        renderer.render(deltaTime);
        endTime = Platform::getTicks();

        // draw editor ui
        drawEditorUI(buffer, uiMaterial, uiFont, (float) windowWidth, (float) windowHeight);
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

