#include <smol/smol_config_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_log.h>

namespace smol
{
  GlobalRendererConfig::GlobalRendererConfig(const Config& config)  { update(config); }

  GlobalRendererConfig::GlobalRendererConfig() {}

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

  GlobalSystemConfig::GlobalSystemConfig() {}

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

  GlobalDisplayConfig::GlobalDisplayConfig() {}

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
    fullScreen    = entry->getVariableNumber("fullscreen") >= 1.0;
    caption       = entry->getVariableString("caption");
    aspectRatio   = (float) entry->getVariableNumber("aspect_ratio", 0.0);
    Vector3 color = entry->getVariableVec3("crop_area_color", Vector3(0.0f, 0.0f, 0.0f));
    cropAreaColor = Color(color.x, color.y, color.z);
    size          = entry->getVariableVec2("size");
    width         = (int) size.x;
    height        = (int) size.y;
  }

  ConfigManager::ConfigManager()
    : initialized(false) { }

  ConfigManager::~ConfigManager()
  {
    initialized = false;
    debugLogInfo("Deinitialized ConfigManager");
    delete config;
  }

  ConfigManager& ConfigManager::get()
  {
    static ConfigManager instance;
    return instance;
  }

  void ConfigManager::initialize(const char* path)
  {
    SMOL_ASSERT(initialized == false, "Atempt reinitialize ConfigManager");
    //TODO(marcio): Can we still use our custom allocator here ?
    config = new Config(path);
    cfgDisplay.update(*config);
    cfgSystem.update(*config);
    cfgRenderer.update(*config);
    initialized = true;
  }

  const Config& ConfigManager::rawConfig() const
  {
    return *config;
  }

  const GlobalSystemConfig& ConfigManager::systemConfig() const
  {
    SMOL_ASSERT(initialized == true, "Atempt to get System configuration from an uninitialized ConfigManager");
    return cfgSystem;
  }

  const GlobalDisplayConfig& ConfigManager::displayConfig() const
  {
    SMOL_ASSERT(initialized == true, "Atempt to get Display configuration from an uninitialized ConfigManager");
    return cfgDisplay;
  }

  const GlobalRendererConfig& ConfigManager::rendererConfig() const
  {
    SMOL_ASSERT(initialized == true, "Atempt to get Render configuration from an uninitialized ConfigManager");
    return cfgRenderer;
  }
}
