//TODO(marcio): Is it a good idea to make batcher creation/destruction implicit for meshNodes ? 

#include <smol/smol_sprite_node.h>
#include <smol/smol_transform.h>
#include <smol/smol_scene.h>

namespace smol
{

  Handle<SceneNode> SpriteNode::create(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Vector3& position,
      float width,
      float height,
      const Color& color,
      int angle,
      Handle<SceneNode> parent)
  {
    Transform t(
        position,
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        parent);

    Scene& scene = SystemsRoot::get()->sceneManager.getLoadedScene();
    Handle<SceneNode> handle = scene.createNode(SceneNode::Type::SPRITE, t);

    handle->sprite.rect = rect;
    handle->sprite.batcher = batcher;
    handle->sprite.width = width;
    handle->sprite.height = height;
    handle->sprite.angle = angle;
    handle->sprite.color = color;

    batcher->nodeCount++;
    batcher->dirty = true;
    return handle;
  }

  void SpriteNode::destroy(Handle<SceneNode> handle)
  {
    SMOL_ASSERT(handle->typeIs(SceneNode::Type::SPRITE), "Handle passed to SpriteNode::destroy() is not of type SPRITE");
    SystemsRoot::get()->sceneManager.getLoadedScene().destroyNode(handle);
    handle->sprite.batcher->nodeCount--;
  }
}
