
#include <smol/smol_sprite_batcher.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_scene.h>
#include <smol/smol_log.h>
#include <utility>

namespace smol
{
  SpriteBatcher::SpriteBatcher(Handle<Material> material, Mode mode, int capacity):
    mode(mode),
    material(material),
    nodeCount(0),
    dirty(false)
  {
    Renderer::createStreamBuffer(&buffer, 64);
  }

  void SpriteBatcher::begin()
  {

    // cache the renderable texture dimention so adjust sprite's UVs
    if (material->diffuseTextureCount > 0 )
    {
      textureDimention = material->textureDiffuse[0]->getDimention();
    }
    else
    {
      textureDimention = SystemsRoot::get()->resourceManager.getDefaultTexture().getDimention();
    }
    Renderer::begin(buffer);
  }

  void SpriteBatcher::end()
  {
    Renderer::end(buffer);
  }

  void SpriteBatcher::pushSpriteNode(SceneNode* sceneNode)
  {
    //TODO(marcio): Make sure we don't push more sprites than expected;
    //TODO(marcio): Make sure we received real SpriteNodes
    SpriteNode& node = sceneNode->sprite;
    float textureWidth = textureDimention.x;
    float textureHeight = textureDimention.y;

    // convert UVs from pixels to 0~1 range
    // pixel coords origin is at top left corner
    Rectf uvRect;
    uvRect.x = node.rect.x / (float) textureWidth;
    uvRect.y = 1 - (node.rect.y /(float) textureHeight); 
    uvRect.w = node.rect.w / (float) textureWidth;
    uvRect.h = node.rect.h / (float) textureHeight;

    const Vector3& pos = sceneNode->transform.getPosition();
    Renderer::pushSprite(buffer, pos, {node.width, node.height}, uvRect, node.color);
  }
}
