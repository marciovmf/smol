
#include <smol/smol_sprite_batcher.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_scene.h>
#include <smol/smol_log.h>
#include <utility>

namespace smol
{
  SpriteBatcher::SpriteBatcher(Handle<Material> material, int capacity):
    material(material),
    spriteNodeCount(0),
    textNodeCount(0),
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
    SMOL_ASSERT(sceneNode->typeIs(SceneNode::SPRITE),
        "A node of type '%d' was passed to SpriteBatcher::pushSpriteNode(). It can only accept SPRITE (%d) nodes",
        sceneNode->getType(), SceneNode::SPRITE);
    
    SpriteNode& node = sceneNode->sprite;
    float textureWidth = textureDimention.x;
    float textureHeight = textureDimention.y;

    // convert UVs from pixels to 0~1 range
    Rectf uvRect;
    uvRect.x = node.rect.x / (float) textureWidth;
    uvRect.y = 1 - (node.rect.y /(float) textureHeight); 
    uvRect.w = node.rect.w / (float) textureWidth;
    uvRect.h = node.rect.h / (float) textureHeight;

    // we don't pass the transform position and scale along because it's passed to the shader via Transform.
    Renderer::pushSprite(buffer, {0.0f, 0.0f, 0.0f}, {node.width, node.height}, uvRect, node.color1, node.color2, node.color3, node.color4);
  }

  void SpriteBatcher::pushTextNode(SceneNode* sceneNode)
  {
    SMOL_ASSERT(sceneNode->typeIs(SceneNode::TEXT),
        "A node of type '%d' was passed to SpriteBatcher::pushSpriteNode(). It can only accept SPRITE (%d) nodes",
        sceneNode->getType(), SceneNode::TEXT);
    TextNode& node = sceneNode->text;

    for (int i = 0; i < node.textLen; i++)
    {
      GlyphDrawData& data = node.drawData[i];
      Renderer::pushSprite(buffer, data.position, data.size, data.uv, data.color);
    }
  }

}
