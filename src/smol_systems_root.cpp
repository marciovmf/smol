#include <smol/smol_systems_root.h>

namespace smol
{
  SystemsRoot::SystemsRoot(
      Config& config, Renderer& renderer, Keyboard& keyboard, Mouse& mouse, Scene& scene):
    config(config), renderer(renderer), keyboard(keyboard), mouse(mouse), loadedScene(scene) { }
}
