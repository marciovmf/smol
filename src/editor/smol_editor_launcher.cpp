#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>
#include <smol/smol_project_manager.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_scene_manager.h>
#include <smol/smol_config_manager.h>
#include <smol/smol_input_manager.h>
#include <smol/smol_render_target.h>
#include <smol/smol_renderer.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_color.h>
#include <smol/smol_vector2.h>
#include "smol_editor.h"
#include "smol_editor_common.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef SMOL_PLATFORM_WINDOWS
#define popen _popen
#endif


#if defined(SMOL_DEBUG)
#define SMOL_LOGFILE nullptr
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_ALL
#else
#define SMOL_LOGFILE "smol_engine_log.txt"
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_FATAL |  smol::Log::LOG_ERROR
#endif

#include <smol/smol_event_manager.h>
#include <smol/smol_event.h>

namespace smol
{
  bool resized = false;

  namespace editor
  {
    bool onEvent(const Event& event, void* payload)
    {
      if (event.type == Event::DISPLAY)
      {
        GlobalDisplayConfig* cfg = (GlobalDisplayConfig*) payload;
        cfg->width = event.displayEvent.width;
        cfg->height = event.displayEvent.height;
        resized = true;
      }

      return false; // let other handlers know about this
    }

    /**
     * Calculates the maximum size of a centered quad within an area specified by maxWidth and maxHeight in pixels.
     * The function assumes the origin is at the top-left corner and returns a Rect object with values ranging from 0.0f to 1.0f,
     * representing the percentage of the containing area.
     */
    Rectf calculateAspectAwarePosition(int maxWidth, int maxHeight, float aspectRatio)
    {
      Rectf rect;
      int availableWidth = maxWidth;
      int availableHeight = maxHeight;

      if (maxWidth == 0.0f || maxHeight == 0.0f)
        return Rectf(0.0f, 0.0f, 0.0f, 0.0f);

      if (aspectRatio <= 0.0f)
        return Rectf(0.0f, 0.0f, 1.0f, 1.0f);


      if ((float)maxWidth / maxHeight > aspectRatio) {
        availableWidth = (int)(maxHeight * aspectRatio);
      } else {
        availableHeight = (int)(maxWidth / aspectRatio);
      }

      rect.w = (float)availableWidth / maxWidth;
      rect.h = (float)availableHeight / maxHeight;

      rect.x = ((float)maxWidth - availableWidth) / (2.0f * maxWidth);
      rect.y = ((float)maxHeight - availableHeight) / (2.0f * maxHeight);

      // Account for top left origin by adjusting the y-coordinate
      rect.y = 1.0f - rect.y - rect.h;

      return rect;
    }

    int smolMain(int argc, const char** argv)
    {
      Project project = {};
      GameModule game = {};

      if (argc == 2)
      {
        if (!ProjectManager::loadProject(argv[1], project))
          return 1;

        Platform::setWorkingDirectory(project.path);
      }
      else
      {
        // Change working directory to the binary location
        Platform::setWorkingDirectory(Platform::getBinaryPath());
      }

      Log::verbosity(SMOL_LOGLEVEL);
      if (SMOL_LOGFILE != nullptr)
        smol::Log::toFile(SMOL_LOGFILE);

      // parse variables file
      ConfigManager::get().initialize(SMOL_VARIABLES_FILE);
      const GlobalSystemConfig& systemConfig = ConfigManager::get().systemConfig();
      GlobalDisplayConfig& displayConfig = (GlobalDisplayConfig&) ConfigManager::get().displayConfig(); // temporary hack; Casting const away.

      if (!Platform::initOpenGL(systemConfig.glVersionMajor, systemConfig.glVersionMinor))
        return 1;

      // initialie display and system stuff
      const char* windowTitleFormat = "SMOL v%s %s - %s";
      const size_t maxFmtSize = 64;
      SMOL_ASSERT(strlen(windowTitleFormat) < maxFmtSize, "Window title format exceeds %d bytes", maxFmtSize);
      const size_t windowTitleBufferSize = (int32) Project::PROJECT_MAX_NAME_LEN + maxFmtSize;
      char windowTitle[windowTitleBufferSize];
#ifdef SMOL_RELEASE
      const char *smolBuildType = (const char*) "";
#else
      const char *smolBuildType = (const char*) "(Debug)";
#endif
      snprintf(windowTitle, windowTitleBufferSize, windowTitleFormat, SMOL_VERSION, smolBuildType, project.valid ? project.name : (const char*)"<NO PROJECT>");
      Window* window = Platform::createWindow(displayConfig.width, displayConfig.height, windowTitle);

      if(displayConfig.fullScreen)
      {
        Platform::setFullScreen(window, true);
      }

      Platform::showCursor(systemConfig.showCursor);

      if (systemConfig.captureCursor)
        Platform::captureCursor(window);

      Platform::getWindowSize(window, &displayConfig.width, &displayConfig.height);

      // Initialize systems
      ResourceManager::get().initialize();
      Renderer::initialize(ConfigManager::get().rendererConfig());
      ResourceManager& resourceManager = ResourceManager::get();
      EventManager::get().addHandler(onEvent, Event::DISPLAY, &displayConfig);

      Editor editor;
      editor.initialize(window, project);

      // Create the game backbuffer
      RenderTarget backBuffer;
      Renderer::createTextureRenderTarget(&backBuffer, displayConfig.width, displayConfig.height);
      Handle<Texture> backBufferTexure = resourceManager.getTextureFromRenderTarget(backBuffer);

      Handle<Material> backbufferMaterial = resourceManager.createMaterial(resourceManager.loadShader("assets/texture_color_topleft_origin.shader"),
          &backBufferTexure, 1);
      backbufferMaterial->setSampler2D("mainTex", backBufferTexure);
      StreamBuffer streamBuffer;
      Renderer::createStreamBuffer(&streamBuffer);

      // Editor / Game loop ...
      uint64 startTime = 0;
      uint64 endTime = 0;

      //game.onStart();
      Rectf gameQuadPosition = Rectf(0.0f, 0.0f, 1.0f, 1.0f);

      while(! (Platform::getWindowCloseFlag(window) || editor.getCloseFlag()))
      {
        float deltaTime = Platform::getMillisecondsBetweenTicks(startTime, endTime);

        startTime = Platform::getTicks();
        Platform::updateWindowEvents(window);
        InputManager::get().update();
        EventManager::get().dispatchEvents();


        // Resize the back buffer if window dimentions changed
        if (resized)
        {
          resized = false;
          Renderer::resizeTextureRenderTarget(backBuffer, displayConfig.width, displayConfig.height);
          gameQuadPosition = calculateAspectAwarePosition(displayConfig.width, displayConfig.height, displayConfig.aspectRatio);
        }

        // Render the game to the back buffer
        Renderer::useRenderTarget(backBuffer);
        editor.updateGame(deltaTime);
        SceneManager::get().renderScene(deltaTime);

        // Draw the game backbuffer and the editor the the screen
        Renderer::setViewport(0, 0, displayConfig.width, displayConfig.height);
        Renderer::useDefaultRenderTarget();
        Renderer::setClearColor(displayConfig.cropAreaColor);
        Renderer::clearBuffers((Renderer::CLEAR_COLOR_BUFFER | Renderer::CLEAR_DEPTH_BUFFER));
        Renderer::setMaterial(backbufferMaterial);
        Renderer::begin(streamBuffer);
        Renderer::pushSprite(streamBuffer, Vector3(gameQuadPosition.x, gameQuadPosition.y, 0.0f),
            Vector2(gameQuadPosition.w, gameQuadPosition.h), Rectf(0.0f, 0.0f, 1.0f, 1.0f), Color::WHITE);
        Renderer::end(streamBuffer);

        editor.update(displayConfig.width, displayConfig.height);

        // Draw to the window
        Platform::swapBuffers(window);
        endTime = Platform::getTicks();
      }

      //game.onStop();
      Platform::unloadModule(game.module);
      Platform::destroyWindow(window);

      // Are we supposed to reopen with another project ?
      const char* reopenWithProjectPath = editor.shouldReopenWithProject();
      if (reopenWithProjectPath)
      {
        char cmdLine[Platform::MAX_PATH_LEN];
        const char* editorName = argv[0] + strlen(argv[0]);
        while (editorName > argv[0] && *editorName != '\\' && *editorName != '/') 
          editorName--;
        if (*editorName == '\\' || *editorName == '/') 
          editorName++;

        Platform::setWorkingDirectory(Platform::getBinaryPath());
        snprintf(cmdLine, Platform::MAX_PATH_LEN, "%s %s", editorName, reopenWithProjectPath);
        debugLogInfo("running '%s'", cmdLine);
        popen(cmdLine, "r");
      }

      editor.terminate();
      //ResourceManager::get().~ResourceManager();
      //SceneManager::get().~SceneManager();
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  return smol::editor::smolMain(0, (const char**) lpCmdLine);
}
#endif  // SMOL_PLATFORM_WINDOWS

int main(int argc, const char** argv)
{
  return smol::editor::smolMain(argc, argv);
}

