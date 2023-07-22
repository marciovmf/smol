#ifndef SMOL_CONFIG_MANAGER_H
#define SMOL_CONFIG_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_color.h>
#include <smol/smol_keyboard.h>

namespace smol
{
  struct Config;

  struct GlobalConfiguration
  {
    virtual void update(const Config&) = 0;
  };

  struct SMOL_ENGINE_API GlobalEditorConfig : public GlobalConfiguration
  {
    bool alwaysRebuildBeforeRun = false;
    Keycode keyStartStop = Keycode::KEYCODE_F5;

    GlobalEditorConfig();
    GlobalEditorConfig(const Config& config);
    void update(const Config& config) override;
  };

  struct SMOL_ENGINE_API GlobalRendererConfig : public GlobalConfiguration
  {
    bool enableMSAA = true;
    bool enableGammaCorrection = true;

    GlobalRendererConfig();
    GlobalRendererConfig(const Config& config);
    void update(const Config& config) override;
  };

  struct SMOL_ENGINE_API GlobalDisplayConfig : public GlobalConfiguration
  {
    const char* caption = (const char*) "Smol Engine";
    int width = 1024;
    int height = 7688;
    float aspectRatio = 0.0f;
    Color cropAreaColor = Color::BLACK;
    bool fullScreen = false;

    GlobalDisplayConfig();
    GlobalDisplayConfig(const Config& config);
    void update(const Config& config) override;
  }; 

  struct SMOL_ENGINE_API GlobalSystemConfig : public GlobalConfiguration
  {
    bool showCursor = true;
    bool captureCursor = false;
    int glVersionMajor = 3;
    int glVersionMinor = 0;

    GlobalSystemConfig();
    GlobalSystemConfig(const Config& config);
    void update(const Config& config) override;
  };

  class SMOL_ENGINE_API ConfigManager final
  {
    bool initialized;
    Config* config;
    GlobalSystemConfig    cfgSystem;
    GlobalDisplayConfig   cfgDisplay;
    GlobalRendererConfig  cfgRenderer;
#ifndef SMOL_MODULE_GAME
    GlobalEditorConfig    cfgEditor;
#endif

    ConfigManager();

    public:
    ~ConfigManager();
    static ConfigManager& get();
    void initialize(const char* path);
    const GlobalSystemConfig& systemConfig() const;
    const GlobalDisplayConfig& displayConfig() const;
    const GlobalRendererConfig& rendererConfig() const;
#ifndef SMOL_MODULE_GAME
    const GlobalEditorConfig& editorConfig() const;
#endif
    const Config& rawConfig() const;

    // Disallow coppies
    ConfigManager(const ConfigManager& other) = delete;
    ConfigManager(const ConfigManager&& other) = delete;
    void operator=(const ConfigManager& other) = delete;
    void operator=(const ConfigManager&& other) = delete;
  };

}
#endif //SMOL_CONFIG_MANAGER_H

