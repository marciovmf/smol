//TODO(marcio): Is it a good idea to make batcher creation/destruction implicit for meshNodes ? 

#include <smol/smol_sprite_node.h>
#include <smol/smol_transform.h>
#include <smol/smol_scene_manager.h>
#include <smol/smol_scene.h>

namespace smol
{

  Handle<SceneNode> SpriteNode::create(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Transform& transform,
      float width,
      float height,
      const Color& color)
  {
    Scene& scene = SceneManager::get().getCurrentScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::SPRITE, transform);

    handle->sprite.rect = rect;
    handle->sprite.batcher = batcher;
    handle->sprite.width = width;
    handle->sprite.height = height;
    handle->sprite.color1 = color;
    handle->sprite.color2 = color;
    handle->sprite.color3 = color;
    handle->sprite.color4 = color;
    batcher->spriteNodeCount++;
    batcher->dirty = true;
    return handle;
  }

  void SpriteNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::SPRITE), "Handle passed to SpriteNode::destroy() is not of type SPRITE");
    SceneManager::get().getCurrentScene().destroyNode(handle);
    handle->sprite.batcher->spriteNodeCount--;
  }
}
