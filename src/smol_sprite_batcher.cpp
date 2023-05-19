
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

  SpriteBatcher::SpriteBatcher(Handle<Material> material, Mode mode, int capacity):
    mode(mode),
    arena(capacity * totalSpriteSize + 1),
    nodeCount(0),
    spriteCount(0),
    batchedSprites(0),
    spriteCapacity(capacity),
    dirty(false)
  {
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
    Scene& scene = SystemsRoot::get()->sceneManager.getLoadedScene();
    // It doesn't matter the contents of memory.
    // Nothing is read from this pointer.
    // It's just necessary to create a valid MeshData in order to reserve GPU memory
    //TODO(marcio): Do we really need to reserve memory to create a dummy MeshData ? Can't we just updateMesh() on the first real use ?
    char* memory = arena.pushSize(capacity * SpriteBatcher::totalSpriteSize);
    MeshData meshData((Vector3*)memory, capacity, 
        (unsigned int*)memory, capacity * 6,
        (Color*) memory, nullptr,
        (Vector2*) memory, nullptr);

    renderable = scene.createRenderable(material, resourceManager.createMesh(true, meshData));
  }

  size_t SpriteBatcher::getTotalSizeRequired() const
  {
    return spriteCount * SpriteBatcher::totalSpriteSize;
  }

  void SpriteBatcher::begin()
  {
    batchedSprites = 0;
    arena.reset();

    // Reserve space for all sprites
    const size_t totalSize  = spriteCount * SpriteBatcher::totalSpriteSize;
    memory                  = arena.pushSize(totalSize);
    positions               = (Vector3*) memory;
    colors                  = (Color*)(positions + spriteCount * 4);
    uvs                     = (Vector2*)(colors + spriteCount * 4);
    indices                 = (uint32*)(uvs + spriteCount * 4);

    // cache the renderable texture dimention so adjust sprite's UVs
    if (renderable->material->diffuseTextureCount > 0 )
    {
      textureDimention = renderable->material->textureDiffuse[0]->getDimention();
    }
    else
    {
      textureDimention = SystemsRoot::get()->resourceManager.getDefaultTexture().getDimention();
    }
  }

  void SpriteBatcher::end()
  {
    MeshData meshData(positions, 4 * batchedSprites, indices, 6 * batchedSprites, colors, nullptr, uvs);
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
    Mesh* mesh = resourceManager.getMesh(renderable->mesh);
    Renderer::updateMesh(mesh, &meshData);
  }

  void SpriteBatcher::pushSpriteNode(SceneNode* sceneNode)
  {
    //TODO(marcio): Make sure we don't push more sprites than expected;
    //TODO(marcio): Make sure we don't push more nodes than expected;
    //TODO(marcio): Make sure we received real SpriteNodes
    
    Sprite& node = sceneNode->spriteInfo.sprite;
    float textureWidth = textureDimention.x;
    float textureHeight = textureDimention.y;

    // convert UVs from pixels to 0~1 range
    // pixel coords origin is at top left corner
    Rectf uvRect;
    uvRect.x = node.rect.x / (float) textureWidth;
    uvRect.y = 1 - (node.rect.y /(float) textureHeight); 
    uvRect.w = node.rect.w / (float) textureWidth;
    uvRect.h = node.rect.h / (float) textureHeight;

    Vector3* pVertex  = (4 * batchedSprites)  + positions;
    Vector2* pUVs     = (4 * batchedSprites)  + uvs;
    uint32* pIndices  = (6 * batchedSprites)  + indices;
    Color* pColors    = (4 * batchedSprites)  + colors;
    int offset        = (4 * batchedSprites);

    const Vector3& pos = sceneNode->transform.getPosition();
    float halfW = node.width/2.0f;
    float halfH = node.height/2.0f;

    pVertex[0] = {pos.x - halfW, pos.y + halfH, pos.z};             // top left
    pVertex[1] = {pos.x + halfW, pos.y - halfH, pos.z};             // bottom right
    pVertex[2] = {pos.x + halfW, pos.y + halfH, pos.z};             // top right
    pVertex[3] = {pos.x - halfW, pos.y - halfH, pos.z};             // bottom left

    pColors[0] = node.color;                                        // top left
    pColors[1] = node.color;                                        // bottom right
    pColors[2] = node.color;                                        // top right
    pColors[3] = node.color;                                        // bottom left

    pUVs[0] = {uvRect.x, uvRect.y};                                 // top left 
    pUVs[1] = {uvRect.x + uvRect.w, uvRect.y - uvRect.h};           // bottom right
    pUVs[2] = {uvRect.x + uvRect.w, uvRect.y};                      // top right
    pUVs[3] = {uvRect.x, uvRect.y - uvRect.h};                      // bottom left

    pIndices[0] = offset + 0;
    pIndices[1] = offset + 1;
    pIndices[2] = offset + 2;
    pIndices[3] = offset + 0;
    pIndices[4] = offset + 3;
    pIndices[5] = offset + 1;

    batchedSprites++;
  }
}
