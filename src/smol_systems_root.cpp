#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_random.h>
#include <smol/smol_scene.h>
#include <time.h>

namespace smol
{
  SystemsRoot* SystemsRoot::instance = nullptr;

  GlobalRendererConfig::GlobalRendererConfig(const Config& config)  { update(config); }

  void GlobalRendererConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("renderer");
    if (!entry)
    {
      debugLogError("No 'Renderer' config found");
      return;
    }

    enableMSAA            = entry->getVariableNumber("enable_msaa", true) >= 1.0;
    enableGammaCorrection = entry->getVariableNumber("enable_gamma_correction", true) >= 1.0;
  }

  GlobalSystemConfig::GlobalSystemConfig(const Config& config)
  { 
    update(config);
  }

  void GlobalSystemConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("system");
    if (!entry)
    {
      debugLogError("No 'system' config found");
      return;
    }

    showCursor = entry->getVariableNumber("show_cursor") >= 1.0; 
    captureCursor = entry->getVariableNumber("capture_cursor") >= 1.0;
    const Vector2 defaultGlVersion = Vector2{3.0f, 0.0f};
    Vector2 glVersion = entry->getVariableVec2("gl_version", defaultGlVersion);
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

    Vector2 size;
    fullScreen = entry->getVariableNumber("fullscreen") >= 1.0;
    caption    = entry->getVariableString("caption");
    size       = entry->getVariableVec2("size");
    width      = (int) size.x;
    height     = (int) size.y;
  }


  SystemsRoot::SystemsRoot(Config& config):
    config(config),
    rendererConfig(config)
  { 
  }

  void SystemsRoot::initialize(Config& config)
  {
    if (!SystemsRoot::instance)
    {
      seed((int32)time(0));
      SystemsRoot::instance = new SystemsRoot(config);
      SystemsRoot::instance->renderer.initialize(SystemsRoot::instance->rendererConfig);
      SystemsRoot::instance->resourceManager.initialize();
    }
  }

  void SystemsRoot::terminate()
  {
    SystemsRoot::instance->~SystemsRoot();
  }

  SystemsRoot::~SystemsRoot() 
  {
    SystemsRoot::instance = nullptr;
  }

  inline SystemsRoot* SystemsRoot::get()
  {
    return SystemsRoot::instance;
  }
}
