#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>


namespace smol
{
  SystemsRoot* SystemsRoot::instance = nullptr;

  SystemsRoot::SystemsRoot(Config& config,
      Renderer& renderer,
      Keyboard& keyboard,
      Mouse& mouse,
      ResourceManager& resourceManager,
      Scene& scene):
    config(config),
    renderer(renderer),
    keyboard(keyboard),
    mouse(mouse),
    resourceManager(resourceManager),
    loadedScene(scene) { }


  void SystemsRoot::initialize(Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, ResourceManager& resourceManager, Scene& scene)
  {
    if (!SystemsRoot::instance)
    {
      SystemsRoot::instance = new SystemsRoot(config, renderer, keyboard, mouse, resourceManager, scene);
    }
  }

  inline SystemsRoot* SystemsRoot::get()
  {
    return SystemsRoot::instance;
  }
}
