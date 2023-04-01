
#include <smol/smol_sprite_batcher.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_scene.h>
#include <smol/smol_log.h>
#include <utility>

namespace smol
{
  const size_t SpriteBatcher::positionsSize   = 4 * sizeof(Vector3);
  const size_t SpriteBatcher::colorsSize      = 4 * sizeof(Color);
  const size_t SpriteBatcher::uvsSize         = 4 * sizeof(Vector2);
  const size_t SpriteBatcher::indicesSize     = 6 * sizeof(unsigned int);
  const size_t SpriteBatcher::totalSpriteSize = positionsSize + colorsSize + uvsSize + indicesSize;

  SpriteBatcher::SpriteBatcher(Handle<Material> material, int capacity):
    arena(capacity * totalSpriteSize + 1),
    spriteCount(0),
    spriteCapacity(capacity),
    dirty(false)
  {
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
    Scene& scene = SystemsRoot::get()->loadedScene;
    // It doesn't matter the contents of memory.
    // Nothing is read from this pointer.
    // It's just necessary to create a valid MeshData in order to reserve GPU memory
    char* memory = arena.pushSize(capacity * SpriteBatcher::totalSpriteSize);
    MeshData meshData((Vector3*)memory, capacity, 
        (unsigned int*)memory, capacity * 6,
        (Color*) memory, nullptr,
        (Vector2*) memory, nullptr);

    renderable = scene.createRenderable(material, resourceManager.createMesh(true, meshData));
  }
}
