#include <smol/smol_systems_root.h>

namespace smol
{
  SystemsRoot* SystemsRoot::instance = nullptr;

  SystemsRoot::SystemsRoot(
      Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, Scene& scene):
    config(config), renderer(renderer), keyboard(keyboard), mouse(mouse), loadedScene(scene) { }


    void SystemsRoot::initialize(Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, Scene& scene)
    {
      if (!SystemsRoot::instance)
      {
        SystemsRoot::instance = new SystemsRoot(config, renderer, keyboard, mouse, scene);
      }
    }

    inline SystemsRoot* SystemsRoot::get()
    {
      return SystemsRoot::instance;
    }
}
