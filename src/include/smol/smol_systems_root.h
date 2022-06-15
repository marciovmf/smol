#ifndef SMOL_SYSTEMS_ROOT
#define SMOL_SYSTEMS_ROOT

#include <smol/smol.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_mouse.h>

namespace smol
{
  class Renderer;
  struct Scene;
  struct Config;
  struct ResourceManager;

  struct SMOL_ENGINE_API SystemsRoot
  {
    Config& config;
    Renderer& renderer;
    Keyboard& keyboard; 
    Mouse& mouse; 
    Scene& loadedScene;    //TODO: Remove this. I just need some place to get a reference to the scene from the game side. This will probably be a scene manager in the future.
    ResourceManager& resourceManager;

    static void initialize(Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, ResourceManager& resourceManager, Scene& scene);
    static SystemsRoot* get();

    private:
    static SystemsRoot* instance;
    SystemsRoot(Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, ResourceManager& resourceManager, Scene& scene);
  };

}

#endif  // SMOL_SYSTEMS_ROOT
