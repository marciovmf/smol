#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_scene.h>

namespace smol
{
  GlobalRendererConfig::GlobalRendererConfig(const Config& config)  { update(config); }

  void GlobalRendererConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("renderer");
    if (!entry)
    {
      debugLogError("No 'Renderer' config found");
      return;
    }

    enableMSAA            = entry->getVariableNumber("enableMSAA") >= 1.0;
    enableGammaCorrection = entry->getVariableNumber("enableGammaCorrection") >= 1.0;
  }

  GlobalSystemConfig::GlobalSystemConfig(const Config& config)      { update(config); }

  void GlobalSystemConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("system");
    if (!entry)
    {
      debugLogError("No 'system' config found");
      return;
    }

    showCursor = entry->getVariableNumber("showCursor") >= 1.0; 
    captureCursor = entry->getVariableNumber("captureCursor") >= 1.0;
    const Vector2 defaultGlVersion = Vector2{3.0f, 0.0f};
    Vector2 glVersion = entry->getVariableVec2("glVersion", defaultGlVersion);
    glVersionMajor = (int) glVersion.x;
    glVersionMinor = (int) glVersion.y;
  }

  GlobalDisplayConfig::GlobalDisplayConfig(const Config& config)  { update(config); }

  void GlobalDisplayConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("display");
    if (!entry)
    {
      debugLogError("No 'display' config found");
      return;
    }

    caption = entry->getVariableString("caption");
    Vector2 size = entry->getVariableVec2("size");
    width = size.x;
    height = size.y;
  }



  SystemsRoot* SystemsRoot::instance = nullptr;

  SystemsRoot::SystemsRoot(Config& config):
    config(config),
    rendererConfig(config),
    renderer(rendererConfig),
    resourceManager(),
    loadedScene()
  { 
  }


  void SystemsRoot::initialize(Config& config)
  {
    if (!SystemsRoot::instance)
    {
      SystemsRoot::instance = new SystemsRoot(config);
      SystemsRoot::instance->renderer.setScene(SystemsRoot::instance->loadedScene);
      SystemsRoot::instance->resourceManager.initialize();
    }
  }

  inline SystemsRoot* SystemsRoot::get()
  {
    return SystemsRoot::instance;
  }
}
