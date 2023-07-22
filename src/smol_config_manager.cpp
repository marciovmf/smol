#include <smol/smol_config_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_input_manager.h>
#include <smol/smol_log.h>

namespace smol
{


  GlobalEditorConfig::GlobalEditorConfig(const Config& config)  { update(config); }

  GlobalEditorConfig::GlobalEditorConfig() {}

  void GlobalEditorConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("editor");
    if (!entry)
    {
      debugLogWarning("No 'editor' config found");
      return;
    }

    alwaysRebuildBeforeRun  = entry->getVariableNumber("always_build_on_run", alwaysRebuildBeforeRun) >= 1.0;
    const char* keyName     = entry->getVariableString("key_play_stop", "F5");
    keyStartStop = InputManager::get().keyboard.getKeycodeByName(keyName);
  }

  GlobalRendererConfig::GlobalRendererConfig(const Config& config)  { update(config); }

  GlobalRendererConfig::GlobalRendererConfig() {}

  void GlobalRendererConfig::update(const Config& config)
  {
    const ConfigEntry* entry = config.findEntry("renderer");
    if (!entry)
    {
      debugLogWarning("No 'Renderer' config found");
      return;
    }

    enableMSAA            = entry->getVariableNumber("enable_msaa", enableMSAA) >= 1.0;
    enableGammaCorrection = entry->getVariableNumber("enable_gamma_correction", enableGammaCorrection) >= 1.0;
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
      debugLogWarning("No 'system' config found");
      return;
    }

    showCursor = entry->getVariableNumber("show_cursor", showCursor) >= 1.0; 
    captureCursor = entry->getVariableNumber("capture_cursor", captureCursor) >= 1.0;
    const Vector2 defaultGlVersion = Vector2{(float) glVersionMajor, (float) glVersionMinor};
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
      debugLogWarning("No 'display' config found");
      return;
    }

    Vector2 size;
    fullScreen    = entry->getVariableNumber("fullscreen", fullScreen) >= 1.0;
    caption       = entry->getVariableString("caption", caption);
    aspectRatio   = (float) entry->getVariableNumber("aspect_ratio", aspectRatio);
    Vector3 color = entry->getVariableVec3("crop_area_color", Vector3(cropAreaColor.r, cropAreaColor.g, cropAreaColor.b));
    cropAreaColor = Color(color.x, color.y, color.z);
    size          = entry->getVariableVec2("size", Vector2((float) width, (float) height));
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
    config = new Config(path);
    cfgDisplay.update(*config);
    cfgSystem.update(*config);
    cfgRenderer.update(*config);
    cfgEditor.update(*config);
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

  const GlobalEditorConfig& ConfigManager::editorConfig() const
  {
    SMOL_ASSERT(initialized == true, "Atempt to get Editor configuration from an uninitialized ConfigManager");
    return cfgEditor;
  }
}
