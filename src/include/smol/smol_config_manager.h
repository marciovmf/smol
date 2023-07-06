#ifndef SMOL_CONFIG_MANAGER_H
#define SMOL_CONFIG_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_color.h>

namespace smol
{
  struct Config;

  struct GlobalConfiguration
  {
    virtual void update(const Config&) = 0;
  };

  struct SMOL_ENGINE_API GlobalRendererConfig : public GlobalConfiguration
  {
    bool enableMSAA;
    bool enableGammaCorrection;

    GlobalRendererConfig();
    GlobalRendererConfig(const Config& config);
    void update(const Config& config) override;
  };

  struct SMOL_ENGINE_API GlobalDisplayConfig : public GlobalConfiguration
  {
    const char* caption;
    int width;
    int height;
    float aspectRatio;
    Color cropAreaColor;
    bool fullScreen;

    GlobalDisplayConfig();
    GlobalDisplayConfig(const Config& config);
    void update(const Config& config) override;
  }; 

  struct SMOL_ENGINE_API GlobalSystemConfig : public GlobalConfiguration
  {
    bool showCursor;
    bool captureCursor;
    int glVersionMajor;
    int glVersionMinor;

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

    ConfigManager();

    public:
    ~ConfigManager();
    static ConfigManager& get();
    void initialize(const char* path);
    const GlobalSystemConfig& systemConfig() const;
    const GlobalDisplayConfig& displayConfig() const;
    const GlobalRendererConfig& rendererConfig() const;
    const Config& rawConfig() const;

    // Disallow coppies
    ConfigManager(const ConfigManager& other) = delete;
    ConfigManager(const ConfigManager&& other) = delete;
    void operator=(const ConfigManager& other) = delete;
    void operator=(const ConfigManager&& other) = delete;
  };

}
#endif //SMOL_CONFIG_MANAGER_H

