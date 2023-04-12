#ifndef SMOL_SYSTEMS_ROOT
#define SMOL_SYSTEMS_ROOT

#include <smol/smol.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_mouse.h>
#include <smol/smol_renderer.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_scene.h>

namespace smol
{
  class Renderer;
  struct Scene;
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
    Config&                config;
    Scene                  loadedScene; //TODO(marcio): We need some kind of scene manager for this
    Renderer               renderer;
    GlobalRendererConfig   rendererConfig;
    ResourceManager        resourceManager;
    Keyboard               keyboard; 
    Mouse                  mouse; 


    static void initialize(Config& config);
    static SystemsRoot* get();

    // Disallow coppies
    SystemsRoot(const SystemsRoot& other) = delete;
    SystemsRoot(const SystemsRoot&& other) = delete;
    void operator=(const SystemsRoot& other) = delete;
    void operator=(const SystemsRoot&& other) = delete;

    private:
    static SystemsRoot* instance;
    SystemsRoot(Config& config);
  };

}

#endif  // SMOL_SYSTEMS_ROOT
