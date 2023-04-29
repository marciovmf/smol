#ifndef SMOL_SYSTEMS_ROOT
#define SMOL_SYSTEMS_ROOT

#include <smol/smol.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_mouse.h>
#include <smol/smol_renderer.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_scene_manager.h>

namespace smol
{
  class Renderer;
  struct Config;
  struct ResourceManager;

  struct GlobalConfiguration
  {
    virtual void update(const Config&) = 0;
  };

  struct SMOL_ENGINE_API GlobalRendererConfig : public GlobalConfiguration
  {
    bool enableMSAA;
    bool enableGammaCorrection;

    GlobalRendererConfig(const Config& config);
    void update(const Config& config) override;
  };

  struct SMOL_ENGINE_API GlobalDisplayConfig : public GlobalConfiguration
  {
    const char* caption;
    int width;
    int height;
    bool fullScreen;

    GlobalDisplayConfig(const Config& config);
    void update(const Config& config) override;
  }; 

  struct SMOL_ENGINE_API GlobalSystemConfig : public GlobalConfiguration
  {
    bool showCursor;
    bool captureCursor;
    int glVersionMajor;
    int glVersionMinor;

    GlobalSystemConfig(const Config& config);
    void update(const Config& config) override;
  };

  struct SMOL_ENGINE_API SystemsRoot
  {
    Config&               config;
    SceneManager          sceneManager;
    Renderer              renderer;
    GlobalRendererConfig  rendererConfig;
    ResourceManager       resourceManager;
    Keyboard              keyboard; 
    Mouse                 mouse; 

    static SystemsRoot* get();

#ifndef SMOL_MODULE_GAME
    static void initialize(Config& config);
    static void terminate();
#endif

    // Disallow coppies
    SystemsRoot(const SystemsRoot& other) = delete;
    SystemsRoot(const SystemsRoot&& other) = delete;
    void operator=(const SystemsRoot& other) = delete;
    void operator=(const SystemsRoot&& other) = delete;

    private:
    static SystemsRoot* instance;
    SystemsRoot(Config& config);
    ~SystemsRoot();
  };

}

#endif  // SMOL_SYSTEMS_ROOT
